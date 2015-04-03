#ifndef _NAVMESH_INPUT_INC
#define _NAVMESH_INPUT_INC

#include <vector>
#include "transform.h"

class CNavmeshInput {
public:
  struct TInput {
	DirectX::XMFLOAT3 pmin;
	DirectX::XMFLOAT3 pmax;
	const float* vertex_vector;
	const short* triangles_vector;
	int nvtx_module;
	int nindx_module;
	TInput() : nvtx_module(0), nindx_module(0){}
  };
  static const int MAX_INPUTS = 1024;
  typedef std::vector< TInput > VInputs;

public:
  VInputs               inputs;
  DirectX::XMFLOAT3           aabb_min;
  DirectX::XMFLOAT3           aabb_max;
  
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
  void addInput(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, const float* vertx_mod, const short* indx_mod, unsigned nvtx_mod, unsigned nindx_mod, TTransform* t);
  void prepareInput( const TInput& input );
  void unprepareInput( );
  void computeBoundaries(  );
};

#endif
