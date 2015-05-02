#ifndef INC_VERTEX_DECLARATIONS_H_
#define INC_VERTEX_DECLARATIONS_H_

class CVertexDecl {
public:
	D3D11_INPUT_ELEMENT_DESC* elems;
	UINT                      nelems;
	unsigned                  bytes_per_vertex;
	CVertexDecl(D3D11_INPUT_ELEMENT_DESC* the_elems, UINT the_nelems);
};

extern CVertexDecl vdcl_position;
extern CVertexDecl vdcl_position_color;
extern CVertexDecl vdcl_position_uv;
extern CVertexDecl vdcl_position_uv_normal;
extern CVertexDecl vdcl_position_uv_normal_skin;
extern CVertexDecl vdcl_position_uv_normal_tangent;

template<class TVertex>
CVertexDecl* getVertexDecl();

#endif
