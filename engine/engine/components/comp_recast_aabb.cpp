#include "mcv_platform.h"
#include "comp_recast_aabb.h"
#include "aabb.h"
#include "nav_mesh_manager.h"

TCompRecastAABB::TCompRecastAABB() {
	index = -1;
	active = false;
}

TCompRecastAABB::~TCompRecastAABB() {
}

void TCompRecastAABB::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	CNav_mesh_manager::get().registerRecastAABB(this);
	index=CNav_mesh_manager::get().getLastRecastAABBIndex();

}

void TCompRecastAABB::setIndex(int ind){
	index = ind;
}

bool TCompRecastAABB::getActive(){
	return active;
}

void TCompRecastAABB::setActive(bool act){
	active = act;
}

int TCompRecastAABB::getIndex(){
	return index;
}

void TCompRecastAABB::init() {

}

