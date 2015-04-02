#include "mcv_platform.h"
#include "nav_mesh_manager.h"

static CNav_mesh_manager the_nav_mesh_manager;

CNav_mesh_manager& CNav_mesh_manager::get() {
	return the_nav_mesh_manager;
}

bool CNav_mesh_manager::build_nav_mesh(){
	nav_mesh.m_input = nav_mesh_input;
	nav_mesh.m_input.computeBoundaries();
	nav_mesh.build();
	return true;
}

CNav_mesh_manager::CNav_mesh_manager()
{
}


CNav_mesh_manager::~CNav_mesh_manager()
{
}
