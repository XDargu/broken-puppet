#include "mcv_platform.h"
#include "collision_mesh.h"
#include "physics_manager.h"

CMeshCollisionManager mesh_collision_manager;

std::vector<u8> vtxs_copy;
std::vector<TIndex>idxs_copy;

// Just to check we are not rendering a mesh we have not previously activated
//const CCollision_Mesh* CCollision_Mesh::current_active_mesh = nullptr;

CCollision_Mesh::CCollision_Mesh()
	:nvertexs(0)
	, nindices(0)
	, topology(D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, vtxs_decl(nullptr)
{ }

CCollision_Mesh::~CCollision_Mesh() {
}

// -------------------------------------------
bool CCollision_Mesh::create(
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

	indices = new TIndex[anindices];
	memcpy(indices, the_indices, anindices * sizeof(TIndex));

	return true;
}

// --------------------------------------
bool CCollision_Mesh::load(CDataProvider& dp) {

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

	for (int i = 0; i < header.nvertexs; ++i){
		vtxs_copy.push_back(vtxs[i]);
	}

	for (int i = 0; i < header.nidxs; ++i){
		idxs_copy.push_back(idxs[i]);
	}

	// Use our create mesh function
	return create(header.nvertexs
		, &vtxs[0]
		, header.nidxs
		, &idxs[0]
		, (ePrimitiveType)header.primitive_type
		, vtx_decl);
}

PxTriangleMesh* CCollision_Mesh::load(const char* name){//, PxCooking* mCooking, PxPhysics* gPhysicsSDK) {
	char full_name[MAX_PATH];
	sprintf(full_name, "data/meshes/%s.mesh", name);
	CFileDataProvider fdp(full_name);
	if (load(fdp)){
		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = getNvertexs();
		meshDesc.points.stride = vtxs_decl->bytes_per_vertex; // del header sacar los bytes per vertex
		meshDesc.points.data = vertexs;

		assert(getNindices() % 3 == 0);
		meshDesc.triangles.count = getNindices() / 3;
		meshDesc.triangles.stride = sizeof(TIndex) * 3;
		meshDesc.triangles.data = indices;
		meshDesc.flags = physx::PxMeshFlags(physx::PxMeshFlag::e16_BIT_INDICES);
		bool valid = meshDesc.isValid();

		physx::PxDefaultMemoryOutputStream stream;

		bool sucess = CPhysicsManager().gCooking->cookTriangleMesh(meshDesc, stream);
		if (sucess){
			physx::PxDefaultMemoryInputData rb(stream.getData(), stream.getSize());
			return 	CPhysicsManager().gPhysicsSDK->createTriangleMesh(rb);
		}else
			return NULL;
	}else{
		return NULL;
	}
}

void CCollision_Mesh::destroy() {
	//SAFE_RELEASE(ib);
}

std::vector<unsigned char> CCollision_Mesh::getTheVertexs(){
	return vtxs_copy;
}

std::vector <unsigned short> CCollision_Mesh::getTheIndices(){
	return idxs_copy;
}

unsigned CCollision_Mesh::getNvertexs(){
	return nvertexs;
}

unsigned CCollision_Mesh::getNindices(){
	return nindices;
}

D3D_PRIMITIVE_TOPOLOGY CCollision_Mesh::getTopology(){
	return topology;
}

