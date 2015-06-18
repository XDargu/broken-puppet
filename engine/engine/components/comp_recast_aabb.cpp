#include "mcv_platform.h"
#include "comp_recast_aabb.h"
#include "aabb.h"
#include "components\comp_transform.h"
#include "nav_mesh_manager.h"

TCompRecastAABB::TCompRecastAABB() {
	min = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	max = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

TCompRecastAABB::~TCompRecastAABB() {
}

void TCompRecastAABB::loadFromAtts(const std::string& elem, MKeyValue &atts) {
    m_transform = assertRequiredComponent<TCompTransform>(this);

	TCompTransform* transform = (TCompTransform*)m_transform;

	min = atts.getPoint("min");
	max = atts.getPoint("max");

	m_aabb = AABB(min, max);

	CNav_mesh_manager::get().recastAABBs.push_back(this);
}

void TCompRecastAABB::init() {

}

void TCompRecastAABB::update(float elapsed){
}
