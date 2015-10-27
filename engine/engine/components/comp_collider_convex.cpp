#include "mcv_platform.h"
#include "comp_collider_convex.h"
#include "comp_recast_aabb.h"
#include "../render/collision_convex.h"

const CCollision_Convex* c_m;

void TCompColliderConvex::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	strcpy(path, ("colliders/" + atts.getString("path", "missing_mesh") + "_collider").c_str());
	c_m = convex_collision_manager.getByName(path);

	physx::PxConvexMesh* convex_mesh = c_m->collision_convex;

	physx::PxConvexMeshGeometry geom(convex_mesh);

	collider = Physics.gPhysicsSDK->createShape(geom,
		*Physics.gPhysicsSDK->createMaterial(
		atts.getFloat("staticFriction", 0.5)
		, atts.getFloat("dynamicFriction", 0.5)
		, atts.getFloat("restitution", 0.5))
		,
		true);

	//AABB recast_aabb = AABB(XMVectorSet(-22.f, 0.f, -33.f, 0.f), XMVectorSet(5.0f, 1.f, -8.f, 0));

	setCollisionGroups();

	//Una vez creado el shape, no necesitamos el triangleMesh
	convex_mesh->release();

	inside_recastAABB = false;
	//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
}

void TCompColliderConvex::init() {
	/*const CCollision_Convex* c_nv = convex_collision_manager.getByName(path);
	TCompAABB* m_aabb = getSibling<TCompAABB>(this);

	for (int i = 0; i < CNav_mesh_manager::get().recastAABBs.size(); i++){
		TCompRecastAABB* recast_AABB = ((TCompRecastAABB*)CNav_mesh_manager::get().recastAABBs[i]);
		TCompAABB* aabb_comp = (TCompAABB*)recast_AABB->m_aabb;
		AABB recast_aabb = AABB(aabb_comp->min, aabb_comp->max);
		if (recast_aabb.intersects(m_aabb)) {
			addInputNavMesh();
			CNav_mesh_manager::get().colConvex.push_back(this);
		}
	}*/
}

void TCompColliderConvex::addInputNavMesh(){
	if (this){
		TCompAABB* aabb_module = getSibling<TCompAABB>(this);
		TCompTransform* trans = getSibling<TCompTransform>(this);

		std::string name = ((CEntity*)CHandle(this).getOwner())->getName();

		if ((aabb_module) && (trans)){

			TTransform* t = trans;
			//t->transformPoint()
			XMFLOAT3 min;
			XMStoreFloat3(&min, aabb_module->min);
			XMFLOAT3 max;
			XMStoreFloat3(&max, aabb_module->max);

			const CCollision_Convex* c_m = convex_collision_manager.getByName(path);

			if (path){
				//const CCollision_Mesh* c_m = mesh_collision_manager.getByName(path);
				if (c_m){
					if (m_v)
						delete[] m_v;
					if (t_v)
						delete[] t_v;
					unsigned n_vertex = c_m->nvertexs;
					m_v = new float[n_vertex * 8];
					memcpy(m_v, c_m->vertex_floats, n_vertex * 8 * sizeof(float));

					unsigned n_triangles = c_m->nindices;
					t_v = new int[n_triangles];
					memcpy(t_v, c_m->index_int, n_triangles * sizeof(int));

					CNav_mesh_manager::get().nav_mesh_input.addInput(min, max, m_v, t_v, n_vertex, n_triangles, t, CNav_mesh_manager::get().nav_mesh_input.MODULE);
				}
			}
		}
		else{
			if (!aabb_module)
				XASSERT(aabb_module, "Error getting aabb from entity %s", name.c_str());
			if (!trans)
				XASSERT(trans, "Error getting transform from entity %s", name.c_str());
		}
	}
}

void TCompColliderConvex::setCollisionGroups(){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	PxU32 myMask = convertStrInCollisionFilter(e->collision_tag);
	PxU32 notCollide = 0;
	bool found = false;
	auto it = CPhysicsManager::get().m_collision->find(myMask);
	if (it != CPhysicsManager::get().m_collision->end()){
		std::vector<physx::PxU32>colFil = it->second;
		if (!colFil.empty()){
			//found = std::find(colFil.begin(), colFil.end(), filterData1.word0) != colFil.end();
			for (int i = 0; i < colFil.size(); i++){
				notCollide |= colFil[i];
			}
		}
	}
	setupFiltering(collider, myMask, notCollide);
}

void TCompColliderConvex::setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems){
	PxU32 not_collide;
	for (int i = 0; i < num_elems; i++){
		not_collide |= vector_masks[i];
	}
	setupFiltering(collider, own_mask, not_collide);
}

TCompColliderConvex::~TCompColliderConvex(){
	if (t_v)
		delete[] t_v;

	if (m_v)
		delete[] m_v;
}

void TCompColliderConvex::checkIfInsideRecastAABB(){
	TCompAABB* m_aabb = getSibling<TCompAABB>(this);

	for (int i = 0; i < CNav_mesh_manager::get().recastAABBs.size(); i++){
		TCompRecastAABB* recast_AABB = ((TCompRecastAABB*)CNav_mesh_manager::get().recastAABBs[i]);
		TCompAABB* aabb_comp = (TCompAABB*)recast_AABB->m_aabb;
		AABB recast_aabb = AABB(aabb_comp->min, aabb_comp->max);
		if (recast_aabb.intersects(m_aabb)) {
			if (!inside_recastAABB){
				addInputNavMesh();
				CNav_mesh_manager::get().colConvex.push_back(CHandle(this));
				inside_recastAABB = true;
			}
		}else{
			if (inside_recastAABB){
				CNav_mesh_manager::get().removeConvex(CHandle(this));
				inside_recastAABB = false;
			}
		}
	}
}
