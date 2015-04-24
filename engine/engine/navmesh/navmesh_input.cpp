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

void CNavmeshInput::addInput(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, const float* vertx_mod, const int* indx_mod, unsigned nvtx_mod, unsigned nindx_mod, TTransform* t, kind k) {
  nverts_total += nvtx_mod;
  ntris_total += nindx_mod;

  TInput input;
  input.pmin = p0;
  input.pmax = p1;
  input.nvtx_module = nvtx_mod;
  input.nindx_module = nindx_mod;

  input.vertex_vector = vertx_mod;
  input.triangles_vector = indx_mod;  
  input.t = t;
  input.type = k;

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

  verts = new float[nverts*3];
  tris = new int[ntris];

  memset(verts, 0, nverts * 3 * sizeof(float));
  memset(tris, 0, ntris * sizeof(int));

  int ind_indx = 0;
  int idx_first = 0;
  int idx_second = 0;
  int idx_third = 0;
  for (int j = 0; j < ntris; j++){
	  if (ind_indx == 0){
		  idx_first = (int)input.triangles_vector[j];
		  ind_indx++;
	  }
	  else if (ind_indx == 1){
		  idx_second = (int)input.triangles_vector[j];
		  ind_indx++;
	  }
	  else if (ind_indx == 2){
		  idx_third = (int)input.triangles_vector[j];
		  tris[j - 2] = idx_first;
		  tris[j - 1] = idx_third;
		  tris[j] = idx_second;
		  ind_indx = 0;
	  }
  }
  ntris = input.nindx_module/3;

  if (input.type == MODULE){
	  int ind = 0;
	  int i = 0;
	  while (ind < input.nvtx_module * 8){
		  XMVECTOR vertex = DirectX::XMVectorSet(input.vertex_vector[ind], input.vertex_vector[ind + 1], input.vertex_vector[ind + 2], 0);
		  ind = ind + 8;
		  XMVECTOR aux = input.t->transformPoint(vertex);
		  XMFLOAT3 vertex_coords;
		  XMStoreFloat3(&vertex_coords, aux);
		  verts[i] = vertex_coords.x;
		  verts[i + 1] = vertex_coords.y;
		  verts[i + 2] = vertex_coords.z;
		  i = i + 3;
	  }
  }else if (input.type == OBSTACLE){
	  int i = 0;
	  while (i < input.nvtx_module * 3){
		  verts[i] = input.vertex_vector[i];
		  i = i + 1;
	  }
  }
}

void CNavmeshInput::unprepareInput( ) {
  delete [] verts;
  delete [] tris;
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
