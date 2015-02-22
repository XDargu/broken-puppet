#ifndef INC_COMP_MESH_COLLIDER_H_
#define INC_COMP_MESH_COLLIDER_H_

#include "base_component.h"
#include "../render/collision_mesh.h"

struct TCompMeshCollider : TBaseComponent {
	physx::PxShape* collider;
	char path[32];

	TCompMeshCollider() { collider = nullptr; }
	TCompMeshCollider(physx::PxShape* the_mesh_collider) { collider = the_mesh_collider; strcpy(path, "unknown"); }

	void loadFromAtts(MKeyValue &atts) {
		strcpy(path, atts.getString("path", "missing_mesh").c_str());
		const CCollision_Mesh* c_m = mesh_collision_manager.getByName(path);
		physx::PxTriangleMesh* triangle_mesh = c_m->collision_mesh;

		physx::PxTriangleMeshGeometry geom(triangle_mesh);
		
		collider = Physics.gPhysicsSDK->createShape(geom,
			*Physics.gPhysicsSDK->createMaterial(
			atts.getFloat("staticFriction", 0.5)
			, atts.getFloat("dynamicFriction", 0.5)
			, atts.getFloat("restitution", 0.5))
			,
			true);		
	}

	std::string toString() {
		return "Mesh Collider: " + std::string(path);
	}
};

#endif