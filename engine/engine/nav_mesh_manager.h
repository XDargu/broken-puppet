#ifndef INC_NAV_MESH_MANAGER_H_
#define INC_NAV_MESH_MANAGER_H_
#include "navmesh\navmesh.h"

class CNav_mesh_manager
{
private:
	CNavmesh nav_mesh;
public:
	CNav_mesh_manager();
	~CNav_mesh_manager();
	bool build_nav_mesh();
	static CNav_mesh_manager& get();
	CNavmeshInput nav_mesh_input;
};
#endif

