#ifndef INC_NAV_MESH_MANAGER_H_
#define INC_NAV_MESH_MANAGER_H_
#include "navmesh\navmesh.h"
#include "navmesh\navmesh_query.h"
#include "render\render_utils.h"

#include <thread>
#include <mutex>

class CNav_mesh_manager
{
private:
	//CNavmesh nav_mesh;
	//dtNavMeshQuery navMeshQuery;
	float* prue;
	int num;
	bool first;

public:
	CNav_mesh_manager();
	~CNav_mesh_manager();
	bool build_nav_mesh();
	void render_nav_mesh();
	void render_tile(const dtMeshTile* tile);
	void findPath(XMVECTOR pst_src, XMVECTOR pst_dst, XMVECTOR* straightPath, int &numPoints);
	void pathRender();
	void updateNavmesh();
	void prepareInputNavMesh();
	static CNav_mesh_manager& get();
	CNavmeshInput nav_mesh_input;
};
#endif

