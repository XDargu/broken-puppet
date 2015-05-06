#include "mcv_platform.h"
#include "comp_sensor_distance_player.h"
#include "../item_manager.h"
#include "entity_manager.h"
#include "physics_manager.h"
#include "handle\handle.h"

void TCompSensorDistPlayer::loadFromAtts(const std::string& elem, MKeyValue &atts) {
}

void TCompSensorDistPlayer::init() {
}

float TCompSensorDistPlayer::playerDistance(){

	CEntity* player = CEntityManager::get().getByName("Player");
	PxVec3 p_transform = Physics.XMVECTORToPxVec3(((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position);

	CEntity* e = CHandle(this).getOwner();
	PxVec3 e_transform = Physics.XMVECTORToPxVec3(((TCompTransform*)e->get<TCompTransform>())->position);

	return (p_transform - e_transform).magnitude();
}