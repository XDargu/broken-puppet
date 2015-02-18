#include "mcv_platform.h"
#include "mesh.h"

CMeshManager mesh_manager;

// Just to check we are not rendering a mesh we have not previously activated
const CMesh* CMesh::current_active_mesh = nullptr;

CMesh::CMesh()
: vb( nullptr )
, ib( nullptr )
, nvertexs( 0 )
, nindices( 0 )
, topology(D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED)
, vtxs_decl( nullptr )
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

  assert(vb);

  // Activate the vertex buffer
  UINT stride = vtxs_decl->bytes_per_vertex;
  UINT offset = 0;
  ::render.ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

  // Set primitive topology (LINES,POINTS,..)
  ::render.ctx->IASetPrimitiveTopology(topology);

  // Set index buffer
  if ( ib )
    ::render.ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);

  current_active_mesh = this;
}

// --------------------------------------
void CMesh::render() const {

  assert(current_active_mesh == this);  // Did you forget to activate this mesh?

  assert(vb);
  if ( ib )
    ::render.ctx->DrawIndexed(nindices, 0, 0);
  else
    ::render.ctx->Draw(nvertexs, 0);
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

  // Use our create mesh function
  return create(header.nvertexs
    , &vtxs[0]
    , header.nidxs
    , &idxs[0]
    , (ePrimitiveType)header.primitive_type
    , vtx_decl);
}

bool CMesh::load(const char* name) {
  char full_name[MAX_PATH];
  sprintf(full_name, "data/meshes/%s.mesh", name);
  CFileDataProvider fdp(full_name);
  return load(fdp);
}

bool CMesh::loadPrimitiveShape(const ePrimitiveShape shape) {
	switch (shape)
	{
	case ePrimitiveShape::CUBE:
		//
		break;
	}
	return NULL;
}