#include "mcv_platform.h"
#include "collision_convex.h"
#include "physics_manager.h"

CConvexCollisionManager convex_collision_manager;

// Just to check we are not rendering a mesh we have not previously activated
const CCollision_Convex* CCollision_Convex::convex_collision = nullptr;

CCollision_Convex::CCollision_Convex()
	:nvertexs(0)
	, nindices(0)
	, topology(D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, vtxs_decl(nullptr)
{ }

CCollision_Convex::~CCollision_Convex() {
}

// -------------------------------------------
bool CCollision_Convex::create(
	unsigned anvertexs
	, const void* the_vertexs       // Can't be null
	, unsigned anindices
	, const TIndex* the_indices     // Can be null
	, ePrimitiveType primitive_type
	, const CVertexDecl* avtxs_decl
	) {

	// Confirm the given data is valid
	assert(anvertexs > 0);
	assert(the_vertexs != nullptr);
	assert(avtxs_decl != nullptr);
	assert(avtxs_decl->bytes_per_vertex > 0);

	// Save number of indices and vertexs
	nindices = anindices;
	nvertexs = anvertexs;
	vtxs_decl = avtxs_decl;

	//Check if primitive is triangle list.
	switch (primitive_type) {
	case TRIANGLE_LIST: topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
	default:
		fatal("Primitive_type %d is not valid", primitive_type);
	}

	size_t total_bytes_for_vertex = avtxs_decl->bytes_per_vertex * anvertexs;

	vertexs = new u8[total_bytes_for_vertex];
	memcpy(vertexs, the_vertexs, total_bytes_for_vertex);

	//Copiamos el vector de vertices a un vector de floats
	vertex_floats = new float[total_bytes_for_vertex];
	memcpy(vertex_floats, the_vertexs, total_bytes_for_vertex);

	indices = new TIndex[anindices];
	memcpy(indices, the_indices, anindices * sizeof(TIndex));

	//Copiamos el vector de indices a un vector de ints
	index_int = new int[anindices];
	memcpy(index_int, &v_tris[0], anindices * sizeof(int));

	nindices = anindices;
	// Change indices

	return true;
}

// --------------------------------------
bool CCollision_Convex::load(CDataProvider& dp) {

	assert(dp.isValid());

	THeader header;
	dp.read(header);
	if (!header.isValid())
		return false;

	std::vector< u8 > vtxs;
	std::vector< TIndex > idxs;

	bool finished = false;
	while (!finished) {
		TChunkHeader chunk;
		dp.read(chunk);
		switch (chunk.magic) {

		case vtxs_magic:
			assert(chunk.nbytes == header.nvertexs * header.bytes_per_vertex);
			vtxs.resize(chunk.nbytes);
			dp.read(&vtxs[0], chunk.nbytes);
			break;

		case idxs_magic:
			assert(chunk.nbytes == header.nidxs * header.bytes_per_index);
			idxs.resize(header.nidxs);
			dp.read(&idxs[0], chunk.nbytes);
			break;

		case end_magic:
			finished = true;
			break;

		default:
			fatal("Invalid chunk type %08x while reading %s\n", chunk.magic, dp.getName());
			break;
		}
	}

	// Convert the header.vertex_type to our vertex_declaration
	const CVertexDecl* vtx_decl = nullptr;
	switch (header.vertex_type) {
	case POSITIONS: vtx_decl = nullptr; break;
	case POSITION_UV: vtx_decl = nullptr; break;
	case POSITION_COLOR: vtx_decl = &vdcl_position_color; break;
	case POSITION_UV_NORMAL: vtx_decl = &vdcl_position_uv_normal; break;
	}
	assert(vtx_decl);

	//Informacion casteada para nav meshes
	std::vector<float>vec_cast_float(vtxs.begin(), vtxs.end());
	v_vertex = vec_cast_float;
	std::vector<int>vec_cast_int(idxs.begin(), idxs.end());
	v_tris = vec_cast_int;
	//-----------------------------------------------------

	// Use our create mesh function
	return create(header.nvertexs
		, &vtxs[0]
		, header.nidxs
		, &idxs[0]
		, (ePrimitiveType)header.primitive_type
		, vtx_decl);
}

bool CCollision_Convex::load(const char* name){//, PxCooking* mCooking, PxPhysics* gPhysicsSDK) {
	char full_name[MAX_PATH];
	sprintf(full_name, "data/meshes/%s.mesh", name);
	CFileDataProvider fdp(full_name);
	if (load(fdp)){
		physx::PxConvexMeshDesc  convexDesc;
		convexDesc.points.count = getNvertexs();
		convexDesc.points.stride = vtxs_decl->bytes_per_vertex; // del header sacar los bytes per vertex
		convexDesc.points.data = vertexs;
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		convexDesc.vertexLimit = 256;

		/*assert(getNindices() % 3 == 0);
		meshDesc.triangles.count = getNindices() / 3;
		meshDesc.triangles.stride = sizeof(TIndex) * 3;
		meshDesc.triangles.data = indices;
		meshDesc.flags = physx::PxMeshFlags(physx::PxMeshFlag::e16_BIT_INDICES) | (physx::PxMeshFlag::eFLIPNORMALS);*/

		bool valid = convexDesc.isValid();
		physx::PxDefaultMemoryOutputStream stream;

		bool sucess = CPhysicsManager::get().gCooking->cookConvexMesh(convexDesc, stream);
		if (sucess){
			physx::PxDefaultMemoryInputData rb(stream.getData(), stream.getSize());
			collision_convex = CPhysicsManager::get().gPhysicsSDK->createConvexMesh(rb);
			return true;
		}
		else{
			collision_convex = nullptr;
			return false;
		}
	}
	else{
		collision_convex = nullptr;
		return NULL;
	}
}

void CCollision_Convex::destroy() {
	collision_convex->release();
}

unsigned CCollision_Convex::getNvertexs(){
	return nvertexs;
}

unsigned CCollision_Convex::getNindices(){
	return nindices;
}

D3D_PRIMITIVE_TOPOLOGY CCollision_Convex::getTopology(){
	return topology;
}