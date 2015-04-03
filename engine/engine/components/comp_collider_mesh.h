#ifndef INC_COMP_COLLIDER_MESH_H_
#define INC_COMP_COLLIDER_MESH_H_

#include "base_component.h"
#include "nav_mesh_manager.h"
#include "physics_manager.h"
#include "render\collision_mesh.h"
#include "comp_aabb.h"
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

		const PxVec3* vertex = geom.triangleMesh->getVertices();
		unsigned n_vertex = geom.triangleMesh->getNbVertices();
		const float* m_v = (float*)vertex;

		/*for (int i = 0; i < n_vertex * 3; ++i) {
			const float a = m_v[i];
		}*/

		const void* triangles = geom.triangleMesh->getTriangles();
		unsigned n_triangles = geom.triangleMesh->getNbTriangles();
		const short* t_v = (short*)triangles;

		/*for (int i = 0; i < n_triangles*3; ++i) {
			const short a = t_v[i];
		}*/

		//PRUEBA NAV MESH
		TCompAABB* aabb_module = getSibling<TCompAABB>(this);
		TCompTransform* trans = getSibling<TCompTransform>(this);

		TTransform* t = trans;
		//t->transformPoint()
		XMFLOAT3 min;
		XMStoreFloat3(&min, aabb_module->min);
		XMFLOAT3 max;
		XMStoreFloat3(&max, aabb_module->max);
		CNav_mesh_manager::get().nav_mesh_input.addInput(min, max, m_v, t_v, n_vertex, n_triangles, t);
		//----------------

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
