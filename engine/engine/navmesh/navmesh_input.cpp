#include "mcv_platform.h"
#include "navmesh_input.h"
#include "transform.h"
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

void CNavmeshInput::addInput(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, const float* vertx_mod, const short* indx_mod, unsigned nvtx_mod, unsigned nindx_mod, TTransform* t) {
  nverts_total += nvtx_mod*3;
  ntris_total += nindx_mod;

  TInput input;
  input.pmin = p0;
  input.pmax = p1;
  input.nvtx_module = nvtx_mod;
  input.nindx_module = nindx_mod;

  input.vertex_vector = vertx_mod;
  input.triangles_vector = indx_mod;  

  /*nverts_total += 8;
  ntris_total += 10;

  TInput input;
  input.pmin = p0;
  input.pmax = p1;*/



  inputs.push_back( input );
}

// ---------------------------------------------------
void CNavmeshInput::prepareInput( const TInput& input ) {
	// usar este metodo para usando los punteros a la primera posicion de los vectores de vertex y de indices
	// para hacer hacer el memset con sus longitudes correspondientes
  unprepareInput( );
 
  nverts = input.nvtx_module;
  ntris = input.nindx_module;

  //verts = input.vertx_module;

  verts = new float[nverts * 3];
  tris = new int[ntris];

  memset(verts, 0, nverts * 3 * sizeof(float));
  memset(tris, 0, ntris * sizeof(int));

  // RECORRER EL ARRAY DE VERTICES Y ALMACENAR SUS TRANSFORMARLAS A GLOBALES
  //

  /*verts = input.vertex_vector[0];
  tris = input.triangles_vector[0];

  float prueba1 = *input.vertex_vector[1];
  float prueba2 = *input.vertex_vector[2];*/

  ntris = input.nindx_module / 3;

  /*nverts = 8;
  ntris = 10;

  verts = new float[ nverts * 3 ];
  tris = new int[ ntris * 3 ];

  memset( verts, 0, nverts * 3 * sizeof( float ) );
  memset( tris, 0, ntris * 3 * sizeof( int ) );

  XMVECTOR v[ 8 ] = {
      DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetZ(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetZ(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetZ(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetZ(input.pmin), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetZ(input.pmax), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmin), DirectX::XMVectorGetZ(input.pmax), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmin), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetZ(input.pmax), 0)
	, DirectX::XMVectorSet(DirectX::XMVectorGetX(input.pmax), DirectX::XMVectorGetY(input.pmax), DirectX::XMVectorGetZ(input.pmax), 0)
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

  assert( idx == ntris * 3 );*/
}

void CNavmeshInput::unprepareInput( ) {
  //delete [] verts;
  //delete [] tris;
  verts = 0;
  tris = 0;
}

void CNavmeshInput::computeBoundaries( ) {
  aabb_min.x = 0.f;
  aabb_min.y = 0.f;
  aabb_min.z = 0.f;
  aabb_max.x = 0.f;
  aabb_max.y = 0.f;
  aabb_max.z = 0.f;

  for( auto& i : inputs ) {
	  if (i.pmin.x < aabb_min.x)   aabb_min.x=i.pmin.x;
	  if (i.pmin.y < aabb_min.y)   aabb_min.y=i.pmin.y;
	  if (i.pmin.z < aabb_min.z)   aabb_min.z=i.pmin.z;
	  if (i.pmax.x > aabb_max.x)   aabb_max.x=i.pmax.x;
	  if (i.pmax.y > aabb_max.y)   aabb_max.y=i.pmax.y;
	  if (i.pmax.z > aabb_max.z)   aabb_max.z=i.pmax.z;
  }
}
