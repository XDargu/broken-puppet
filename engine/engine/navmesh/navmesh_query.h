#ifndef _NAVMESH_QUERY_INC
#define _NAVMESH_QUERY_INC

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourCommon.h"
#include "navmesh.h"

class CNavmeshQuery {
public:
  enum ETool {
    FIND_PATH = 0
    , WALL_DISTANCE
    , RAYCAST
    , NUM_TOOLS
  };

  struct TPos {
    DirectX::XMFLOAT3 p;
    bool   set;
	TPos() : p(0.f,0.f,0.f), set(false) { }
  };

  CNavmesh* data;
  TPos    p1;
  TPos    p2;
  ETool   tool;

public:
  CNavmeshQuery( CNavmesh* anavmesh ) : data( anavmesh ) { }
  void update(DirectX::XMVECTOR& curr_pos);
  void updatePos(XMVECTOR& init_pos, XMVECTOR& end_pos);
  void render( );
  void drawAgent( const float* pos, float r, float h, float c, const unsigned int col );
  void findStraightPath();
  // tools
  void setTool( ETool atool );
  void resetTools( );
  void updateTool();
  void findInside( TPos& pos );
  void findPath( TPos& start, TPos& end);
  void wallDistance( TPos& pos );
  void raycast( TPos& start, TPos& end );
  XMVECTOR getRandomPoint(XMVECTOR center, float radius, XMVECTOR current_pos);
  //float getRandomFloat();


private:
  // the dt data
  dtQueryFilter m_filter;
  dtStatus m_pathFindStatus;

  static const int MAX_POLYS = 256;
  static const int MAX_SMOOTH = 2048;

  dtPolyRef m_startRef;
  dtPolyRef m_endRef;
  dtPolyRef m_polys[ MAX_POLYS ];
  dtPolyRef m_parent[ MAX_POLYS ];
  int m_npolys;
  float m_straightPath[ MAX_POLYS * 3 ];
  unsigned char m_straightPathFlags[ MAX_POLYS ];
  dtPolyRef m_straightPathPolys[ MAX_POLYS ];
  int m_nstraightPath;
  float m_polyPickExt[ 3 ];
  float m_queryPoly[ 4 * 3 ];

  static const int MAX_RAND_POINTS = 64;
  float m_randPoints[ MAX_RAND_POINTS * 3 ];
  int m_nrandPoints;
  bool m_randPointsInCircle;

  //float m_spos[ 3 ];
  //float m_epos[ 3 ];
  float m_hitPos[ 3 ];
  float m_hitNormal[ 3 ];
  bool m_hitResult;
  float m_distanceToWall;
  float m_neighbourhoodRadius;
  float m_randomRadius;
  //bool m_sposSet;
  //bool m_eposSet;

  int m_pathIterNum;
  dtPolyRef m_pathIterPolys[ MAX_POLYS ];
  int m_pathIterPolyCount;
  float m_prevIterPos[ 3 ], m_iterPos[ 3 ], m_steerPos[ 3 ], m_targetPos[ 3 ];

  static const int MAX_STEER_POINTS = 10;
  float m_steerPoints[ MAX_STEER_POINTS * 3 ];
  int m_steerPointCount;

public:
	//Path variables
	float m_smoothPath[MAX_SMOOTH * 3];
	int m_nsmoothPath;
	//PATH ----------------------------------------------------------------------
	float* straightPath;
	int numPointsStraightPath;
	//---------------------------------------------------------------------------
};

#endif
