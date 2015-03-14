#ifndef _NAVMESH_INPUT_INC
#define _NAVMESH_INPUT_INC

#include <vector>

class CNavmeshInput {
public:
  struct TInput {
    DirectX::XMVECTOR pmin;
	DirectX::XMVECTOR pmax;
	TInput() : pmin(DirectX::XMVectorZero()), pmax(DirectX::XMVectorZero()) { }
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

public:
  CNavmeshInput( );

  void clearInput( );
  void addInput(const DirectX::XMVECTOR& p0, const DirectX::XMVECTOR& p1);
  void prepareInput( const TInput& input );
  void unprepareInput( );
  void computeBoundaries(  );
};

#endif
