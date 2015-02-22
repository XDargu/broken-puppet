#ifndef INC_COMP_MESH_COLLIDER_H_
#define INC_COMP_MESH_COLLIDER_H_

#include "base_component.h"
#include "../render/collision_mesh.h"

struct TCompMeshCollider : TBaseComponent {
	PxTriangleMesh* collision_mesh;
	char path[32];

	TCompMeshCollider() { collision_mesh = nullptr; }
	TCompMeshCollider(PxTriangleMesh* the_mesh_collider) { collision_mesh = the_mesh_collider; strcpy(path, "unknown"); }

	void loadFromAtts(MKeyValue &atts) {
		strcpy(path, atts.getString("path", "missing_mesh").c_str());
		const CCollision_Mesh* c_m = mesh_collision_manager.getByName(path);
		collision_mesh = c_m->collision_mesh;
	}

	std::string toString() {
		return "Mesh Collider: " + std::string(path);
	}
};

#endif