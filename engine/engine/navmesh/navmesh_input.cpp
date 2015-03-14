#include "mcv_platform.h"
#include "navmesh_input.h"
#include <assert.h>

CNavmeshInput::CNavmeshInput( )
  : verts( nullptr )
  , tris( nullptr )
  , nverts( 0 )
  , ntris( 0 )
  , nverts_total( 0 )
  , ntris_total( 0 )
{ }

// ---------------------------------------------------
void CNavmeshInput::clearInput( ) {
  inputs.clear( );
  nverts_total = 0;
  ntris_total = 0;
}

void CNavmeshInput::addInput(const DirectX::XMVECTOR& p0, const DirectX::XMVECTOR& p1) {
  nverts_total += 8;
  ntris_total += 10;

  TInput input;
  input.pmin = p0;
  input.pmax = p1;
  inputs.push_back( input );
}

// ---------------------------------------------------
void CNavmeshInput::prepareInput( const TInput& input ) {
  unprepareInput( );

  nverts = 8;
  ntris = 10;

  verts = new float[ nverts * 3 ];
  tris = new int[ ntris * 3 ];

  memset( verts, 0, nverts * 3 * sizeof( float ) );
  memset( tris, 0, ntris * 3 * sizeof( int ) );

  XMVECTOR v[ 8 ] = {
      DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetY(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetY(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetY(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetY(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetY(input.pmax), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetY(input.pmax), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetY(input.pmax), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetY(input.pmax), 0)
  };

  static const int idxs[ 6 ][ 4 ] = {
      { 4, 6, 7, 5 }
      , { 5, 7, 3, 1 }
      , { 1, 3, 2, 0 }
      , { 0, 2, 6, 4 }
      , { 3, 7, 6, 2 }
      , { 5, 1, 0, 4 }
  };

  for( int i = 0; i<8; ++i ) {
    XMVECTOR p = v[ i ];
    int idx = i * 3;
	verts[idx] = DirectX::XMVectorGetX(p);
	verts[idx + 1] = DirectX::XMVectorGetY(p);
	verts[idx + 2] = DirectX::XMVectorGetZ(p);
  }

  int idx = 0;
  for( int i = 0; i<5; ++i ) {
    tris[ idx++ ] = idxs[ i ][ 0 ];
    tris[ idx++ ] = idxs[ i ][ 2 ];
    tris[ idx++ ] = idxs[ i ][ 1 ];

    tris[ idx++ ] = idxs[ i ][ 0 ];
    tris[ idx++ ] = idxs[ i ][ 3 ];
    tris[ idx++ ] = idxs[ i ][ 2 ];
  }

  assert( idx == ntris * 3 );
}

void CNavmeshInput::unprepareInput( ) {
  delete [] verts;
  delete [] tris;
  verts = 0;
  tris = 0;
}

void CNavmeshInput::computeBoundaries( ) {
  aabb_min = DirectX::XMVectorZero();
  aabb_max = DirectX::XMVectorZero();

  for( auto& i : inputs ) {
	  if (DirectX::XMVectorGetX(i.pmin) < DirectX::XMVectorGetX(aabb_min))   DirectX::XMVectorSetX(aabb_min, DirectX::XMVectorGetX(i.pmin));
	  if (DirectX::XMVectorGetY(i.pmin) < DirectX::XMVectorGetY(aabb_min))   DirectX::XMVectorSetY(aabb_min, DirectX::XMVectorGetY(i.pmin));
	  if (DirectX::XMVectorGetZ(i.pmin) < DirectX::XMVectorGetZ(aabb_min))   DirectX::XMVectorSetZ(aabb_min, DirectX::XMVectorGetZ(i.pmin));
	  if (DirectX::XMVectorGetX(i.pmax) > DirectX::XMVectorGetX(aabb_max))   DirectX::XMVectorSetX(aabb_max, DirectX::XMVectorGetX(i.pmax));
	  if (DirectX::XMVectorGetY(i.pmax) > DirectX::XMVectorGetY(aabb_max))   DirectX::XMVectorSetY(aabb_max, DirectX::XMVectorGetY(i.pmax));
	  if (DirectX::XMVectorGetZ(i.pmax) > DirectX::XMVectorGetZ(aabb_max))   DirectX::XMVectorSetZ(aabb_max, DirectX::XMVectorGetZ(i.pmax));
  }
}
