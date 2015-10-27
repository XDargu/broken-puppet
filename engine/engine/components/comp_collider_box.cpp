#include "mcv_platform.h"
#include "comp_collider_box.h"
#include "comp_recast_aabb.h"
#include "nav_mesh_manager.h"

void TCompColliderBox::setShape(float boxX, float boxY, float boxZ, float static_friction, float dynamic_friction, float restitution) {
		collider = Physics.gPhysicsSDK->createShape(
			physx::PxBoxGeometry(
			physx::PxReal(boxX)
			, physx::PxReal(boxY)
			, physx::PxReal(boxZ)
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

void TCompColliderBox::loadFromAtts(const std::string& elem, MKeyValue &atts) {

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
			physx::PxBoxGeometry(
			physx::PxReal(atts.getFloat("boxX", 0.5))
			, physx::PxReal(atts.getFloat("boxY", 0.5))
			, physx::PxReal(atts.getFloat("boxZ", 0.5))
			),
			*mat
			,
			true);

		//AABB recast_aabb = AABB(XMVectorSet(-22.f, 0.f, -33.f, 0.f), XMVectorSet(5.0f, 1.f, -8.f, 0));
		setCollisionGroups();
		inside_recastAABB = false;
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

void TCompColliderBox::init() {
}

void TCompColliderBox::addInputNavMesh(){
	//--- NAVMESH OBSTACLES --------------------------------------------------------------------------------------------------
	if (this){
		TCompAABB* aabb_module = getSibling<TCompAABB>(this);
		TCompTransform* trans = getSibling<TCompTransform>(this);

		if ((aabb_module) && (trans)){

			TTransform* t = trans;
			t_previous = trans->position;
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
			if (m_v)
				delete[] m_v;
			if (t_v)
				delete[] t_v;
			m_v = new float[n_vertex * 3];
			memcpy(m_v, vertex, n_vertex * 3 * sizeof(float));

			const int indices[] = {
				0, 1, 2, 3, 4, 5, 6, 7
				, 0, 2, 1, 3, 4, 6, 5, 7
				, 0, 4, 1, 5, 2, 6, 3, 7
			};
			t_v = new int[n_triangles];
			memcpy(t_v, indices, n_triangles * sizeof(int));

			CNav_mesh_manager::get().nav_mesh_input.addInput(min, max, m_v, t_v, n_vertex, n_triangles, t, CNav_mesh_manager::get().nav_mesh_input.OBSTACLE);
			//------------------------------------------------------------------------------------------------------------------------
		}
		else{
			std::string name = ((CEntity*)CHandle(this).getOwner())->getName();
			if (!aabb_module)
				XASSERT(aabb_module, "Error getting aabb from entity %s", name.c_str());
			if (!trans)
				XASSERT(trans, "Error getting transform from entity %s", name.c_str());
		}
	}
}

void TCompColliderBox::setCollisionGroups(){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	PxU32 myMask = convertStrInCollisionFilter(e->collision_tag);
	PxU32 notCollide=0;
	bool found = false;
	auto it = CPhysicsManager::get().m_collision->find(myMask);
	if (it != CPhysicsManager::get().m_collision->end()){
		std::vector<physx::PxU32>colFil = it->second;
		if (!colFil.empty()){
			for (int i = 0; i < colFil.size(); i++){
				notCollide |= colFil[i];
			}
		}
	}
	setupFiltering(collider, myMask, notCollide);
}

void TCompColliderBox::setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems){
	PxU32 not_collide;
	for (int i = 0; i < num_elems; i++){
		not_collide |= vector_masks[i];
	}
	setupFiltering(collider, own_mask, not_collide);
}

bool TCompColliderBox::getIfUpdated(){
	TCompTransform* trans = getSibling<TCompTransform>(this);
	if (trans){
		t_current = trans->position;
		float current_x = XMVectorGetX(t_current);
		float current_y = XMVectorGetY(t_current);
		float current_z = XMVectorGetZ(t_current);

		float prev_x = XMVectorGetX(t_previous);
		float prev_y = XMVectorGetY(t_previous);
		float prev_z = XMVectorGetZ(t_previous);
		if ((current_x != prev_x) || (current_y != prev_y) || (current_z != prev_z))
			return true;
		else
			return false;
	}else{
		std::string name = ((CEntity*)CHandle(this).getOwner())->getName();
		if (!trans)
			XASSERT(trans, "Error getting transform from entity %s", name.c_str());
	}
	return false;
}

TCompColliderBox::~TCompColliderBox(){
	if (t_v)
		delete [] t_v;

	if (m_v)
		delete [] m_v;
}

void TCompColliderBox::checkIfInsideRecastAABB(){
	TCompAABB* m_aabb = getSibling<TCompAABB>(this);

	for (int i = 0; i < CNav_mesh_manager::get().recastAABBs.size(); i++){
		TCompRecastAABB* recast_AABB = ((TCompRecastAABB*)CNav_mesh_manager::get().recastAABBs[i]);
		TCompAABB* aabb_comp = (TCompAABB*)recast_AABB->m_aabb;
		AABB recast_aabb = AABB(aabb_comp->min, aabb_comp->max);
		if (recast_aabb.intersects(m_aabb)) {
			if (!inside_recastAABB){
				addInputNavMesh();
				CNav_mesh_manager::get().colBoxes.push_back(CHandle(this));
				inside_recastAABB = true;
			}
		}else{
			if (inside_recastAABB){
				CNav_mesh_manager::get().removeBox(CHandle(this));
				inside_recastAABB = false;
			}
		}
	}
}