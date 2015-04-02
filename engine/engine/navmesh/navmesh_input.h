#ifndef _NAVMESH_INPUT_INC
#define _NAVMESH_INPUT_INC

#include <vector>

class CNavmeshInput {
public:
  struct TInput {
    DirectX::XMVECTOR pmin;
	DirectX::XMVECTOR pmax;
	float* vertx_module;
	std::vector<float*>vertex_vector;
	int* triangles_module;
	std::vector<int*>triangles_vector;
	int nvtx_module;
	int nindx_module;
	TInput() : pmin(DirectX::XMVectorZero()), pmax(DirectX::XMVectorZero()), vertx_module(nullptr), triangles_module(nullptr), nvtx_module(0), nindx_module(0){}
  };
  static const int MAX_INPUTS = 1024;
  typedef std::vector< TInput > VInputs;

public:
  VInputs               inputs;
  DirectX::XMVECTOR           aabb_min;
  DirectX::XMVECTOR           aabb_max;
  
  float*                verts;
  int*                  tris;
  int                   nverts;
  int                   ntris;
  int                   nverts_total;
  int                   ntris_total;
  //std::vector<float>* vertex_vector;
  //std::vector<int>* indices_vector;

public:
  CNavmeshInput( );

  void clearInput( );
  void addInput(const DirectX::XMVECTOR& p0, const DirectX::XMVECTOR& p1, float* vertx_mod, int* indx_mod, int nvtx_mod, int nindx_mod, unsigned vertex_stride);
  void prepareInput( const TInput& input );
  void unprepareInput( );
  void computeBoundaries(  );
};

#endif
