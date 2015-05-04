#include "mcv_platform.h"
#include "comp_player_position_sensor.h"
#include "../item_manager.h"
#include "entity_manager.h"
#include "physics_manager.h"

void TCompPlayerPosSensor::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	radius = atts.getFloat("radius", 2.f);
}

void TCompPlayerPosSensor::init() {
}

bool TCompPlayerPosSensor::playerInRange(){
	CEntity* player = CEntityManager::get().getByName("Player");
	p_transform = ((CEntity*)player)->get<TCompTransform>();
	CEntity* e = CHandle(this).getOwner();
	e_transform = (TCompTransform*)e->get<TCompTransform>();
	if (V3DISTANCE(p_transform->position, e_transform->position) < radius){
		physx::PxRaycastBuffer buf;
		Physics.raycastAll(e_transform->position + XMVectorSet(0, 1.5f, 0, 0), XMVector3Normalize(p_transform->position - e_transform->position), radius, buf);

		bool player_visible = false;
		if (buf.nbTouches > 1){
			if (std::strcmp(buf.touches[1].actor->getName(), "Player") == 0) {
				return true;
			}
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}

	return false;
}