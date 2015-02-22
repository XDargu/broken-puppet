#ifndef INC_COMP_MESH_COLLIDER_H_
#define INC_COMP_MESH_COLLIDER_H_

#include "base_component.h"
#include "../render/collision_mesh.h"

struct TCompMeshCollider : TBaseComponent {
	const CCollision_Mesh* mesh_collider;
	char path[32];

	TCompMeshCollider() { mesh_collider = nullptr; }
	TCompMeshCollider(const CCollision_Mesh* the_mesh_collider) { mesh_collider = the_mesh_collider; strcpy(path, "unknown"); }

	void loadFromAtts(MKeyValue &atts) {
		strcpy(path, atts.getString("path", "missing_mesh").c_str());
		mesh_collider = mesh_collision_manager.getByName(path);
	}

	std::string toString() {
		return "Mesh Collider: " + std::string(path);
	}
};

#endif