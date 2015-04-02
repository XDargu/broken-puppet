#ifndef INC_COMP_COLLIDER_MESH_H_
#define INC_COMP_COLLIDER_MESH_H_

#include "base_component.h"
#include "nav_mesh_manager.h"

struct TCompColliderMesh : TBaseComponent {

	char path[32];
	physx::PxShape* collider;

	TCompColliderMesh() { collider = nullptr; }
	TCompColliderMesh(physx::PxShape* the_mesh_collider) { collider = the_mesh_collider; strcpy(path, "unknown"); }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		strcpy(path, ("colliders/" + atts.getString("path", "missing_mesh") + "_collider").c_str());
		const CCollision_Mesh* c_m = mesh_collision_manager.getByName(path);

		//PRUEBA NAV MESH
		TCompAABB* aabb_module=getSibling<TCompAABB>(this);
		XMVECTOR min = aabb_module->min;
		XMVECTOR max = aabb_module->max;
		CNav_mesh_manager::get().nav_mesh_input.addInput(aabb_module->min, aabb_module->max, c_m->vertex_floats, c_m->index_int, c_m->nvertexs, c_m->nindices, c_m->nav_stride_vertex);
		//----------------

		physx::PxTriangleMesh* triangle_mesh = c_m->collision_mesh;

		physx::PxTriangleMeshGeometry geom(triangle_mesh);

		collider = Physics.gPhysicsSDK->createShape(geom,
			*Physics.gPhysicsSDK->createMaterial(
			atts.getFloat("staticFriction", 0.5)
			, atts.getFloat("dynamicFriction", 0.5)
			, atts.getFloat("restitution", 0.5))
			,
			true);

		const PxVec3* vertex = geom.triangleMesh->getVertices();
		unsigned n_vertex = geom.triangleMesh->getNbVertices();

		
		const float* m_v = (float*)vertex;

		for (int i = 0; i < n_vertex * 3; ++i) {
			const float a = m_v[i];
		}

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

	// Set the collider static friction, dynamic friction and restitution given by a vector
	void setMaterialProperties(XMVECTOR properties) {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		mat->setStaticFriction(XMVectorGetX(properties));
		mat->setDynamicFriction(XMVectorGetY(properties));
		mat->setRestitution(XMVectorGetZ(properties));
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
