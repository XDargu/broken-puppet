#include "mcv_platform.h"
#include "mesh.h"

CMeshManager mesh_manager;

// Just to check we are not rendering a mesh we have not previously activated
const CMesh* CMesh::current_active_mesh = nullptr;

CMesh::CMesh()
	: vb(nullptr)
	, ib(nullptr)
	, nvertexs(0)
	, nindices(0)
	, topology(D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, vtxs_decl(nullptr)
{ }

CMesh::~CMesh() {
}

// -------------------------------------------
bool CMesh::create(
	unsigned anvertexs
	, const void* the_vertexs       // Can't be null
	, unsigned anindices
	, const TIndex* the_indices     // Can be null
	, ePrimitiveType primitive_type
	, const CVertexDecl* avtxs_decl
	, bool can_be_updated
	) {

	// Confirm we are not already created
	assert(vb == nullptr);

	// Confirm the given data is valid
	assert(anvertexs > 0);
	assert(the_vertexs != nullptr);
	assert(avtxs_decl != nullptr);
	assert(avtxs_decl->bytes_per_vertex > 0);

	// Save number of indices and vertexs
	nindices = anindices;
	nvertexs = anvertexs;
	vtxs_decl = avtxs_decl;

	switch (primitive_type) {
	case POINTS: topology = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST; break;
	case LINE_LIST: topology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST; break;
	case TRIANGLE_LIST: topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
	case TRIANGLE_STRIP: topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
	default:
		fatal("Primitive_type %d is not valid", primitive_type);
	}

	// Create the VB
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = avtxs_decl->bytes_per_vertex * anvertexs;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	if (can_be_updated) {
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.Usage = D3D11_USAGE_DYNAMIC;
	}

	// The initial contents of the VB
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = the_vertexs;
	HRESULT hr = ::render.device->CreateBuffer(&bd, &InitData, &vb);
	if (FAILED(hr))
		return false;

	// Create the Index Buffer if the user gives us indices
	if (the_indices) {
		assert(nindices > 0);

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(TIndex) * nindices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = the_indices;
		hr = ::render.device->CreateBuffer(&bd, &InitData, &ib);
		if (FAILED(hr))
			return false;
	}
	else {
		assert(nindices == 0);
	}

	return true;
}

void CMesh::destroy() {
	SAFE_RELEASE(ib);
	SAFE_RELEASE(vb);
}

// --------------------------------------
void CMesh::activate() const {

	XASSERT(vb, "Error loading vertex buffer");

	// Activate the vertex buffer
	UINT stride = vtxs_decl->bytes_per_vertex;
	UINT offset = 0;
	::render.ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	// Set primitive topology (LINES,POINTS,..)
	::render.ctx->IASetPrimitiveTopology(topology);

	// Set index buffer
	if (ib)
		::render.ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);

	current_active_mesh = this;
}

// --------------------------------------
void CMesh::render() const {

	assert(current_active_mesh == this);  // Did you forget to activate this mesh?

	assert(vb);
	if (ib)
		::render.ctx->DrawIndexed(nindices, 0, 0);
	else
		::render.ctx->Draw(nvertexs, 0);
}


// --------------------------------------
void CMesh::renderGroup(int group_id) const {
	assert(group_id < groups.size());
	assert(current_active_mesh == this);  // Did you forget to activate this mesh?

	const TGroup&g = groups[group_id];

	assert(vb);
	if (ib)
		::render.ctx->DrawIndexed(g.num_indices, g.first_index, 0);
	else {
		fatal("not supported!!");
		//::render.ctx->Draw(nvertexs, 0);
	}
}



// --------------------------------------
bool CMesh::load(CDataProvider& dp) {

	assert(dp.isValid());

	THeader header;
	dp.read(header);
	if (!header.isValid())
		return false;

	typedef unsigned char u8;
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

		case groups_magic: {
			assert(chunk.nbytes % 8 == 0);
			int ngroups = chunk.nbytes / 8;
			groups.resize(ngroups);
			dp.read(&groups[0], chunk.nbytes);
			break; }

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
	case POSITION_UV_NORMAL_SKIN: vtx_decl = &vdcl_position_uv_normal_skin; break;
	case POSITION_UV_NORMAL_TANGENT: vtx_decl = &vdcl_position_uv_normal_tangent; break;
	case POSITION_UV_NORMAL_SKIN_TANGENT: vtx_decl = &vdcl_position_uv_normal_skin_tangent; break;
	}
	assert(vtx_decl);

	std::vector<float>vec_cast_float(vtxs.begin(), vtxs.end());
	v_vertex = vec_cast_float;

	std::vector<int>vec_cast_int(idxs.begin(), idxs.end());
	v_tris = vec_cast_int;

	// Use our create mesh function
	return create(header.nvertexs
		, &vtxs[0]
		, header.nidxs
		, &idxs[0]
		, (ePrimitiveType)header.primitive_type
		, vtx_decl
		, false
	);
}

bool CMesh::load(const char* name) {
	char full_name[MAX_PATH];
	sprintf(full_name, "data/meshes/%s.mesh", name);
	CFileDataProvider fdp(full_name);
	return load(fdp);
}

// --------------------------------------
void CMesh::updateFromCPU(const void *new_cpu_data, size_t num_bytes_to_update) {
	if (num_bytes_to_update == 0)
		num_bytes_to_update = nvertexs * vtxs_decl->bytes_per_vertex;
	assert(vb);

	D3D11_MAPPED_SUBRESOURCE mapped_resource;

	// Get CPU access to the GPU buffer
	HRESULT hr = ::render.ctx->Map(
		vb
		, 0
		, D3D11_MAP_WRITE_DISCARD
		, 0
		, &mapped_resource);
	assert(hr == D3D_OK);

	// Copy from CPU to GPU
	memcpy(mapped_resource.pData, new_cpu_data, num_bytes_to_update);

	// Close the map
	::render.ctx->Unmap(vb, 0);

}

void CMesh::renderInstanced(const CMesh& instanced_data, size_t ninstances) const {
	assert(isValid());
	assert(instanced_data.isValid());

	// Set the buffer strides.
	unsigned int strides[2];
	strides[0] = vtxs_decl->bytes_per_vertex;      // My stride
	strides[1] = instanced_data.vtxs_decl->bytes_per_vertex;  // stride of the instance

	// Set the buffer offsets.
	unsigned int offsets[2] = { 0, 0 };

	// Set the array of pointers to the vertex and instance buffers.
	ID3D11Buffer* bufferPointers[2];
	bufferPointers[0] = vb;
	bufferPointers[1] = instanced_data.vb;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	::render.ctx->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	::render.ctx->IASetPrimitiveTopology(topology);

	if (ib) {
		assert(ib);

		// Set index buffer
		DXGI_FORMAT fmt = (sizeof(TIndex) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		::render.ctx->IASetIndexBuffer(ib, fmt, 0);

		::render.ctx->DrawIndexedInstanced(nindices, (UINT)ninstances, 0, 0, 0);
	}
	else {
		::render.ctx->DrawInstanced(nvertexs, (UINT)ninstances, 0, 0);
	}
}

