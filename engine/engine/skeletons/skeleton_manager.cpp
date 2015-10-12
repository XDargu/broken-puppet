#include "mcv_platform.h"
#include "skeleton_manager.h"
#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"

CSkeletonManager skeleton_manager;

CCoreModel::CCoreModel() : CalCoreModel("unnamed"), mesh( nullptr ) {}

void CCoreModel::onStartElement(const std::string &elem, MKeyValue &atts) {
  if (elem == "skeleton") {
    
    // Load csf
    std::string csf = root_path + name + ".csf";
    bool is_ok = loadCoreSkeleton(csf);
    assert(is_ok);
   
    auto bones = getCoreSkeleton()->getVectorCoreBone();
    int bone_id = 0;
    for (auto b : bones) {
      dbg("Bone %02d : %s\n", bone_id, b->getName().c_str());
      bone_id++;
    }

    //float scale_factor = atts.getFloat("scale", 1.0f);
    //scale(scale_factor);
  }
  else if (elem == "mesh") {
    std::string name = atts["name"];

    // preguntar al mesh manager si tiene esta malla?
    // si la tiene usarla
    // Si no intentar cargarla.
    // Si falla o no esta el fichero
    // Cargar el cmf y probar de nuevo

    std::string cmf = root_path + name + ".cmf";
    int mesh_id = loadCoreMesh(cmf);
    assert(mesh_id >= 0);

    std::string smesh = root_path + name + ".mesh";
    bool is_ok = convertCMF(mesh_id, smesh.c_str());
    assert(is_ok);

	CMesh* my_mesh = new CMesh;
	CFileDataProvider fdp(smesh.c_str());
	is_ok = my_mesh->load(fdp);
	assert(is_ok);
	mesh = my_mesh;
	mesh_manager.registerNew(name.c_str(), my_mesh);	

  }
  else if (elem == "anim") {
    std::string caf = atts["name"];
	float blend_time = atts.getFloat("blend", 0.3f);
	animation_blend_times.push_back(blend_time);
	float blend_out_time = atts.getFloat("blend_out", 0.3f);
	animation_blend_out_times.push_back(blend_out_time);
	float cancel_time = atts.getFloat("cancel", 100.f);
	animation_cancel_times.push_back(cancel_time);
	animation_names.push_back(caf);
    caf = root_path + caf + ".caf";
    int id = loadCoreAnimation(caf);	
	getCoreAnimation(id)->setName(split_string(caf, "/").back());
    assert(id >= 0);

  }
  else if (elem == "dump_bone") {
    int id = atts.getInt("id", -1);
    if ( id != -1 )
      bone_ids_to_debug.push_back(id);
  }
  else if (elem == "dump_anim") {
    int the_anim_id = atts.getInt("id", 0);
    int the_bone_id = atts.getInt("bone", 0);
    CalCoreAnimation* core_anim = getCoreAnimation(the_anim_id);
    // get the list of core tracks of above core animation
    std::list<CalCoreTrack *>& listCoreTrack = core_anim->getListCoreTrack();
    // loop through all core tracks of the core animation
    std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
    for (iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack)
    {
      int bone_id = (*iteratorCoreTrack)->getCoreBoneId();
      if (bone_id == the_bone_id) {
        float anim_duration = core_anim->getDuration();
        for (float t = 0.0f; t < anim_duration; t += 1.f / 30.f) {
          CalVector trans;
          CalQuaternion q;
          (*iteratorCoreTrack)->getState(t, trans, q);

          XMVECTOR dxq = Cal2DX(q);
          XMMATRIX m = DirectX::XMMatrixRotationQuaternion(dxq);
          XMVECTOR front = m.r[0];      // pQ VISUALmente los arttistas h
          float yaw = getYawFromVector(front);
          dbg("At time %f Trans = %f %f %f q=%f %f %f %f Yaw:%f\n", t, trans.x, trans.y, trans.z, q.x, q.y, q.z, q.w, rad2deg( yaw ));
        }
      }
    }

  }
  else if (elem == "bone_correction") {
    std::string bone_name = atts["bone"];
    TBoneCorrector bc;
    bc.bone_id = getCoreSkeleton()->getCoreBoneId(bone_name);
    bc.local_dir = DX2Cal( atts.getPoint("local_dir") );
    bc.local_amount = atts.getFloat("amount", 1.0f);
    bc.render = atts.getBool("render", false);
    if ( bc.bone_id != -1 ) 
      bone_corrections.push_back(bc);
  }

  else if (elem == "BoneRigidbody") {

  }

  else if (elem == "jointD6") {

  }

}


bool CCoreModel::load(const char* name) {
  setName(name);
  root_path = "data/skeletons/" + std::string(name) + "/";

  CalLoader::setLoadingMode(LOADER_ROTATE_X_AXIS);

  char full_name[MAX_PATH];
  sprintf(full_name, "%s%s.xml", root_path.c_str(), name);
  return xmlParseFile(full_name);
}

template< class TPOD >
static void saveChunk( CMemoryDataSaver& mds
                , unsigned chunk_type
                , const std::vector < TPOD > &vdata) {
  struct TChunkHeader {
    unsigned magic;
    unsigned nbytes;
  } chunk;
  chunk.magic = chunk_type;
  chunk.nbytes = static_cast< unsigned > ( vdata.size() * sizeof(TPOD) );
  mds.writePOD(chunk);
  mds.write(&vdata[0], chunk.nbytes);
}


bool CCoreModel::convertCMF(int mesh_id, const char* outfile) {

  // -------------------------------------------
  // A skin vertex
  struct CSkinVertex {
    XMFLOAT3 pos;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
	XMFLOAT4 tangent;
    unsigned char bone_ids[4];    // Bones influencing this vertex
    unsigned char weights[4];     // How much the influence is
  };
  typedef std::vector< CSkinVertex > VSkinVertexs;
  VSkinVertexs skin_vtxs;

  typedef std::vector< CMesh::TIndex > VIndices;
  VIndices     skin_idxs;

  CMesh::VGroups skin_groups;


  // The core mesh from cal3d
  CalCoreMesh *core_mesh = getCoreMesh(mesh_id);
  assert(core_mesh);

  int nsubmeshes = core_mesh->getCoreSubmeshCount();
  for (int sm_id = 0; sm_id < nsubmeshes; ++sm_id) {
    CalCoreSubmesh *sm = core_mesh->getCoreSubmesh(sm_id);
	sm->enableTangents(0, true);

    // -----------------------------------------------------
    // INDEX DATA
    // -----------------------------------------------------
    UINT vtx0 = static_cast<UINT>(skin_vtxs.size());
    CMesh::TIndex index0 = static_cast<CMesh::TIndex>(skin_idxs.size());
    int nfaces = sm->getFaceCount();
    auto& vfaces = sm->getVectorFace();
    for (auto& fit : vfaces) {
      // If the first submesh, has 120 indices.
      // The second submesh of cal3d, the index 0 
      // is our index 120, not the zero
      skin_idxs.push_back(vtx0 + fit.vertexId[0]);
      skin_idxs.push_back(vtx0 + fit.vertexId[2]);    // Swap face culling!
      skin_idxs.push_back(vtx0 + fit.vertexId[1]);
    }
    
    // -----------------------------------------------------
    // GROUPS INFO
    // -----------------------------------------------------
    CMesh::TGroup group;
    group.first_index = index0;
	group.num_indices = (int)(vfaces.size() * 3);
    skin_groups.push_back(group);

    // -----------------------------------------------------
    // VERTEX DATA
    // -----------------------------------------------------

    // All texture coords sets
    typedef std::vector<CalCoreSubmesh::TextureCoordinate> VCalTexCoords;
	typedef std::vector<CalCoreSubmesh::TangentSpace> VCalTangent;
    auto& all_tex_coords = sm->getVectorVectorTextureCoordinate();
	auto& all_tangent = sm->getVectorVectorTangentSpace();
    //assert(!all_tex_coords.empty());
	
    // Vertices
    int nvertexs = sm->getVertexCount();

    // Just use the first one
    VCalTexCoords* tex_coords0 = nullptr;
    if (!all_tex_coords.empty()) {
      tex_coords0 = &all_tex_coords[0];
      // Sizes must match
      assert(tex_coords0->size() == nvertexs);
    }	
	
	VCalTangent* tangent = nullptr;
	if (!all_tangent.empty()) {
		tangent = &all_tangent[0];
		// Sizes must match
		assert(tangent->size() == nvertexs);
	}

	// CALCULATE TANGENT
	std::vector<XMFLOAT3> tan1;
	std::vector<XMFLOAT3> tan2;
	tan1.resize(nvertexs * 2, XMFLOAT3(0, 0, 0));
	tan2.resize(nvertexs * 2, XMFLOAT3(0, 0, 0));

	if (tex_coords0) {
		auto a = 0;
		while (a < skin_idxs.size()) {
			// Get the indices of each triangle
			auto i1 = skin_idxs[a];
			auto i2 = skin_idxs[a + 1];
			auto i3 = skin_idxs[a + 2];

			//vtx - i is the original array of floats #(pos, uv, N ...)		
			auto vtx1 = sm->getVectorVertex()[i1];
			auto vtx2 = sm->getVectorVertex()[i2];
			auto vtx3 = sm->getVectorVertex()[i3];

			// The positions are the 3 first floats in each array
			XMFLOAT3 v1 = XMFLOAT3(vtx1.position.x, vtx1.position.y, vtx1.position.z);
			XMFLOAT3 v2 = XMFLOAT3(vtx2.position.x, vtx2.position.y, vtx2.position.z);
			XMFLOAT3 v3 = XMFLOAT3(vtx3.position.x, vtx3.position.y, vtx3.position.z);

			// Then the tex coords
			XMFLOAT2 w1 = XMFLOAT2((*tex_coords0)[i1].u, (*tex_coords0)[i1].v);
			XMFLOAT2 w2 = XMFLOAT2((*tex_coords0)[i2].u, (*tex_coords0)[i2].v);
			XMFLOAT2 w3 = XMFLOAT2((*tex_coords0)[i3].u, (*tex_coords0)[i3].v);

			float x1 = v2.x - v1.x;
			float x2 = v3.x - v1.x;
			float y1 = v2.y - v1.y;
			float y2 = v3.y - v1.y;
			float z1 = v2.z - v1.z;
			float z2 = v3.z - v1.z;

			float s1 = w2.x - w1.x;
			float s2 = w3.x - w1.x;
			float t1 = w2.y - w1.y;
			float t2 = w3.y - w1.y;

			float r = 1.0F / (s1 * t2 - s2 * t1);
			XMFLOAT3 sdir = XMFLOAT3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
			XMFLOAT3 tdir = XMFLOAT3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

			tan1[i1] = XMFLOAT3(tan1[i1].x + sdir.x, tan1[i1].y + sdir.y, tan1[i1].z + sdir.z);
			tan1[i2] = XMFLOAT3(tan1[i2].x + sdir.x, tan1[i2].y + sdir.y, tan1[i2].z + sdir.z);
			tan1[i3] = XMFLOAT3(tan1[i3].x + sdir.x, tan1[i3].y + sdir.y, tan1[i3].z + sdir.z);

			tan2[i1] = XMFLOAT3(tan2[i1].x + tdir.x, tan2[i1].y + tdir.y, tan2[i1].z + tdir.z);
			tan2[i2] = XMFLOAT3(tan2[i2].x + tdir.x, tan2[i2].y + tdir.y, tan2[i2].z + tdir.z);
			tan2[i3] = XMFLOAT3(tan2[i3].x + tdir.x, tan2[i3].y + tdir.y, tan2[i3].z + tdir.z);

			// Next triangle
			a = a + 3;
		}
	}

    // For each cal3d vertex
    int cal_vtx_idx = 0;
    for (auto& vit : sm->getVectorVertex()) {

      // Build one of our skinned vertexs
      CSkinVertex sv;
      memset(&sv, 0x00, sizeof(sv));
	  
      sv.pos = XMFLOAT3(vit.position.x, vit.position.y, vit.position.z);
      sv.normal = XMFLOAT3(vit.normal.x, vit.normal.y, vit.normal.z);
	  
      if (tex_coords0)
        sv.uv = XMFLOAT2((*tex_coords0)[cal_vtx_idx].u, (*tex_coords0)[cal_vtx_idx].v);
	  
	  /*if (tangent)
		  sv.tangent = XMFLOAT3((*tangent)[cal_vtx_idx].tangent.x, (*tangent)[cal_vtx_idx].tangent.y, (*tangent)[cal_vtx_idx].tangent.z);*/

	  // Calculated tangent
	  // Gram-Schmidt orthogonalize
	  XMVECTOR t = XMLoadFloat3(&tan1[cal_vtx_idx]);
	  XMVECTOR t2 = XMLoadFloat3(&tan2[cal_vtx_idx]);
	  XMVECTOR n = XMLoadFloat3(&sv.normal);
	  XMVECTOR v_tangent = XMVector3Normalize(t - n * XMVector3Dot(t, n));

	  // Calculate handedness
	  
	  float head = XMVectorGetX(XMVector3Dot(XMVector3Cross(n, t), t2));
	  v_tangent = XMVectorSetW(v_tangent, head < 0 ? -1.f : 1.f);
	  XMStoreFloat4(&sv.tangent, v_tangent);
	  
	  // Calculate handedness
	  //tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;

      assert(vit.vectorInfluence.size() <= 4);
      int idx = 0;
      int total_weight = 0;
      for (auto fit : vit.vectorInfluence) {
        assert(fit.boneId < 256);
        assert(fit.weight >= 0.f && fit.weight <= 1.0f);
        sv.bone_ids[idx] = static_cast<unsigned char>(fit.boneId);
        sv.weights[idx] = static_cast<unsigned char>(fit.weight * 255.f);
        total_weight += sv.weights[idx];
        ++idx;
      }

      // Confirm the total weight of the vertex == 255, or give the error
      // to the first vertex
      if (total_weight != 255) {
        int error = 255 - total_weight;
        assert(error > 0);
        sv.weights[0] += error;
      }

      // Save the vertex
      skin_vtxs.push_back(sv);

      ++cal_vtx_idx;
    }
  }

  // 
  CMesh::THeader header;
  header.bytes_per_index = sizeof(CMesh::TIndex);
  header.bytes_per_vertex = sizeof(CSkinVertex);
  header.magic = CMesh::THeader::valid_magic;
  header.magic_tail = header.magic;
  header.nidxs = (int)skin_idxs.size();
  header.nvertexs = (int)skin_vtxs.size();
  header.primitive_type = CMesh::TRIANGLE_LIST;
  header.version = CMesh::THeader::current_version;
  header.vertex_type = CMesh::POSITION_UV_NORMAL_SKIN_TANGENT;

  // Virtual file
  CMemoryDataSaver mds;
  mds.writePOD(header);
  
  saveChunk(mds, CMesh::vtxs_magic, skin_vtxs);
  saveChunk(mds, CMesh::idxs_magic, skin_idxs);
  saveChunk(mds, CMesh::groups_magic, skin_groups);

  CMesh::TChunkHeader chunk;
  chunk.magic = CMesh::end_magic;
  chunk.nbytes = 0;
  mds.writePOD(chunk);

  return mds.saveToFile(outfile);
}
