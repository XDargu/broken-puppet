#ifndef INC_COMP_COLLIDER_MESH_H_
#define INC_COMP_COLLIDER_MESH_H_

#include "base_component.h"
#include "physics_manager.h"
#include "render\collision_mesh.h"
#include "collider.h"

struct TCompColliderMesh : public CCollider, TBaseComponent {

	char path[32];

	TCompColliderMesh() : CCollider() { }
	TCompColliderMesh(physx::PxShape* the_mesh_collider) { collider = the_mesh_collider; strcpy(path, "unknown"); }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		strcpy(path, ("colliders/" + atts.getString("path", "missing_mesh") + "_collider").c_str());
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

		//Asignación de mascara al Shape para el filtrado de colisiones
		setupFiltering(collider, FilterGroup::eLEVEL, FilterGroup::eLEVEL);

		//Una vez creado el shape, no necesitamos el triangleMesh
		triangle_mesh->release();

		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	void init() {
	}
};

#endif
