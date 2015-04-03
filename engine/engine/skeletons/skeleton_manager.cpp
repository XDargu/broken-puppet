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

    CMesh *my_mesh = new CMesh;
    CFileDataProvider fdp(smesh.c_str());
    is_ok = my_mesh->load(fdp);
    assert(is_ok);
    mesh = my_mesh;

    //mesh = mesh_manager.register( name, mesh );

  }
  else if (elem == "anim") {
    std::string caf = atts["name"];
    caf = root_path + caf + ".caf";
    int id = loadCoreAnimation(caf);
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
    auto& all_tex_coords = sm->getVectorVectorTextureCoordinate();
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
  header.vertex_type = CMesh::POSITION_UV_NORMAL_SKIN;

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

