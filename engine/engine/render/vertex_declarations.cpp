#include "mcv_platform.h"
#include "vertex_declarations.h"

unsigned bytesOfFormat(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_R32G32_FLOAT: return 8;
  case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
  case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
  }
  fatal("bytesOfFormat: Unknown fmt");
  return 0;
}

CVertexDecl::CVertexDecl(D3D11_INPUT_ELEMENT_DESC* the_elems, UINT the_nelems)
  : elems(the_elems)
  , nelems(the_nelems)
  , bytes_per_vertex(0)
{
  for (unsigned i = 0; i < nelems; ++i) {
    bytes_per_vertex += bytesOfFormat(elems[i].Format);
  }
}

#define DEF_VTX_DECL(x) CVertexDecl x(x##_layout, ARRAYSIZE(x##_layout)) 

// ------------------------------------------------------
D3D11_INPUT_ELEMENT_DESC vdcl_position_color_layout[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position_color);

// ------------------------------------------------------
D3D11_INPUT_ELEMENT_DESC vdcl_position_uv_normal_layout[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
DEF_VTX_DECL(vdcl_position_uv_normal);

