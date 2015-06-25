#ifndef INC_NAV_MESH_MANAGER_H_
#define INC_NAV_MESH_MANAGER_H_
#include "navmesh\navmesh.h"
#include "navmesh\navmesh_query.h"
#include "render\render_utils.h"
#include "components\comp_collider_mesh.h"
#include "components\comp_collider_convex.h"
#include "components\comp_collider_box.h"
#include "components\comp_collider_sphere.h"
#include "components\comp_collider_capsule.h"
#include "physics_manager.h"

#include <thread>
#include <mutex>

class CNav_mesh_manager
{
private:
	//CNavmesh nav_mesh;
	float* polys;
	int num;
	bool first;
	bool need_update;
	int recast_aabb_index;
	CHandle player;
public:
	CNav_mesh_manager();
	~CNav_mesh_manager();
	bool build_nav_mesh();
	void render_nav_mesh();
	void render_tile(const dtMeshTile* tile);
	void findPath(XMVECTOR pst_src, XMVECTOR pst_dst, std::vector<XMVECTOR> &straightPath);
	void pathRender();
	void updateNavmesh();
	void prepareInputNavMesh();
	bool checkIfUpdatedNavMesh();
	int getIndexMyRecastAABB(CHandle my_aabb);
	bool rayCastHit(XMVECTOR pos, XMVECTOR wanted_pos);
	void clearNavMesh();
	void checkUpdates();
	int getLastRecastAABBIndex();
	void registerRecastAABB(CHandle recastAABB);
	void unregisterRecastAABB(CHandle recastAABB);
	void checkDistaceToEnemies();
	XMVECTOR getRandomNavMeshPoint(XMVECTOR center, float radius, XMVECTOR current_pos);
	static CNav_mesh_manager& get();
	CNavmeshInput nav_mesh_input;
	std::vector<CHandle>     colMeshes;
	std::vector<CHandle>   colConvex;
	std::vector<CHandle>      colBoxes;
	std::vector<CHandle>   colSpheres;
	std::vector<CHandle> recastAABBs;
	bool keep_updating_navmesh;
	//std::vector<TCompColliderCapsule*>  colCapsules;
};
#endif

