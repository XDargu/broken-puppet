#include "mcv_platform.h"
#include "comp_zone_aabb.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "comp_name.h"
#include "entity_manager.h"
#include "aabb.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"

TCompZoneAABB::TCompZoneAABB() {
}

TCompZoneAABB::~TCompZoneAABB() {
	CLogicManager::get().unregisterZoneAABB(this);
}

void TCompZoneAABB::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);
	m_name = assertRequiredComponent<TCompName>(this);
	id = atts.getInt("id", 0);
	CLogicManager::get().registerZoneAABB(this);
}

CHandle TCompZoneAABB::getZoneName(){
	return m_name;
}

bool TCompZoneAABB::isPlayerInside(){
	TCompTransform* p_transform = (TCompTransform*)player_transform;
	if (p_transform) {
		if (((TCompAABB*)m_aabb)->containts(p_transform->position))
			return true;
		else
			return false;
	}
}

bool TCompZoneAABB::isPointInside(XMVECTOR point){
	if (((TCompAABB*)m_aabb)->containts(point))
		return true;
	else
		return false;
	
}

void TCompZoneAABB::init() {
	player = CEntityManager::get().getByName("Player");
	if (player.isValid()) {
		player_transform = ((CEntity*)player)->get<TCompTransform>();
	}
}

