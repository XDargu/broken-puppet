#ifndef INC_COMP_COLLIDER_MESH_H_
#define INC_COMP_COLLIDER_MESH_H_

#include "base_component.h"

struct TCompColliderMesh : TBaseComponent {

	char path[32];
	physx::PxShape* collider;

	TCompColliderMesh() { collider = nullptr; }
	TCompColliderMesh(physx::PxShape* the_mesh_collider) { collider = the_mesh_collider; strcpy(path, "unknown"); }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

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

		//Asignación de mascara al Shape para el filtrado de colisiones
		setupFiltering(collider, FilterGroup::eLEVEL, FilterGroup::eLEVEL);

		//Una vez creado el shape, no necesitamos el triangleMesh
		triangle_mesh->release();

		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	void init() {
	}

	physx::PxMaterial* getMaterial() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return mat;
	}

	// Returns the material properties as a vector
	XMVECTOR getMaterialProperties() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return XMVectorSet(
			mat->getStaticFriction(),
			mat->getDynamicFriction(),
			mat->getRestitution(),
			0
			);
	}

	std::string toString() {
		return "Mesh Collider: " + std::string(path);
	}
};

#endif
