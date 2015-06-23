#include "mcv_platform.h"
#include "comp_zone_aabb.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "comp_name.h"
#include "aabb.h"

TCompZoneAABB::TCompZoneAABB() {
}

TCompZoneAABB::~TCompZoneAABB() {
}

void TCompZoneAABB::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);
	m_name = assertRequiredComponent<TCompName>(this);
}

CHandle TCompZoneAABB::getZoneName(){
	return m_name;
}

void TCompZoneAABB::init() {

}

