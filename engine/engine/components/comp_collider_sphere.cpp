#include "mcv_platform.h"
#include "comp_collider_sphere.h"
#include "nav_mesh_manager.h"

void TCompColliderSphere::setShape(float radius, float static_friction, float dynamic_friction, float restitution) {
		collider = Physics.gPhysicsSDK->createShape(
			physx::PxSphereGeometry(
			physx::PxReal(radius)
			),
			*Physics.gPhysicsSDK->createMaterial(
			static_friction
			, dynamic_friction
			, restitution
			)
			,
			true);
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

void TCompColliderSphere::loadFromAtts(const std::string& elem, MKeyValue &atts) {

		physx::PxMaterial* mat = Physics.gPhysicsSDK->createMaterial(
			atts.getFloat("staticFriction", 0.5)
			, atts.getFloat("dynamicFriction", 0.5)
			, atts.getFloat("restitution", 0.5)
			);
		std::string frictionCombineMode = atts.getString("frictionCombineMode", "None");
		std::string restitutionCombineMode = atts.getString("restitutionCombineMode", "None");

		if (frictionCombineMode == "Min") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eMIN);
		}
		if (frictionCombineMode == "Max") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eMAX);
		}
		if (frictionCombineMode == "Average") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eAVERAGE);
		}
		if (frictionCombineMode == "Multiply") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eMULTIPLY);
		}

		if (restitutionCombineMode == "Min") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eMIN);
		}
		if (restitutionCombineMode == "Max") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eMAX);
		}
		if (restitutionCombineMode == "Average") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eAVERAGE);
		}
		if (restitutionCombineMode == "Multiply") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eMULTIPLY);
		}

		collider = Physics.gPhysicsSDK->createShape(
			physx::PxSphereGeometry(
			physx::PxReal(atts.getFloat("radius", 0.5))
			),
			*mat
			,
			true);

		addInputNavMesh();
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

void TCompColliderSphere::init() {
	}

void TCompColliderSphere::addInputNavMesh(){
		//--- NAVMESH OBSTACLES --------------------------------------------------------------------------------------------------

		TCompAABB* aabb_module = getSibling<TCompAABB>(this);
		TCompTransform* trans = getSibling<TCompTransform>(this);

		TTransform* t = trans;
		//t->transformPoint()
		XMFLOAT3 min;
		XMStoreFloat3(&min, aabb_module->min);
		XMFLOAT3 max;
		XMStoreFloat3(&max, aabb_module->max);
		int n_vertex = 8;
		int n_triangles = 24;

		const float vertex[24] = {
			min.x, min.y, min.z
			, max.x, min.y, min.z
			, min.x, max.y, min.z
			, max.x, max.y, min.z
			, min.x, min.y, max.z
			, max.x, min.y, max.z
			, min.x, max.y, max.z
			, max.x, max.y, max.z
		};

		float* m_v = new float[n_vertex * 3];
		memcpy(m_v, vertex, n_vertex * 3 * sizeof(float));

		const int indices[] = {
			0, 1, 2, 3, 4, 5, 6, 7
			, 0, 2, 1, 3, 4, 6, 5, 7
			, 0, 4, 1, 5, 2, 6, 3, 7
		};
		int* t_v = new int[n_triangles];
		memcpy(t_v, indices, n_triangles * sizeof(int));

		CNav_mesh_manager::get().nav_mesh_input.addInput(min, max, m_v, t_v, n_vertex, n_triangles, t, CNav_mesh_manager::get().nav_mesh_input.OBSTACLE);
		//------------------------------------------------------------------------------------------------------------------------
	}