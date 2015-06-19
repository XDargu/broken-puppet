#include "mcv_platform.h"
#include "comp_recast_aabb.h"
#include "aabb.h"
#include "nav_mesh_manager.h"

TCompRecastAABB::TCompRecastAABB() {
}

TCompRecastAABB::~TCompRecastAABB() {
}

void TCompRecastAABB::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	TCompTransform* transform = (TCompTransform*)m_transform;
	TCompAABB* aabb = (TCompAABB*)m_aabb;

	CNav_mesh_manager::get().recastAABBs.push_back(this);
}

void TCompRecastAABB::init() {

}

