#include "mcv_platform.h"
#include "comp_player_position_sensor.h"
#include "../item_manager.h"
#include "entity_manager.h"
#include "physics_manager.h"
#include "handle\handle.h"

void TCompPlayerPosSensor::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	radius = atts.getFloat("radius", 20.f);
}

void TCompPlayerPosSensor::init() {
}

bool TCompPlayerPosSensor::playerInRange(){

	float lowest_distance = radius*radius + 1.f;
	PxRigidActor* lowest_actor = nullptr;
	
	CEntity* player = CEntityManager::get().getByName("Player");
	PxVec3 p_transform =  Physics.XMVECTORToPxVec3(((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position);
	p_transform.y += 1.5f;

	CEntity* e = CHandle(this).getOwner();
	PxVec3 e_transform = Physics.XMVECTORToPxVec3(((TCompTransform*)e->get<TCompTransform>())->position);
	e_transform.y += 1.5f;

	if ((p_transform - e_transform).magnitudeSquared() < (radius * radius)){
		physx::PxRaycastBuffer buf;
		Physics.raycastAll(e_transform, (p_transform - e_transform).getNormalized(), radius, buf);

		bool player_visible = false;

		if (buf.nbTouches > 1){
			for (unsigned int i = 0; i < buf.nbTouches; ++i){
				const char* aux_name = buf.touches[1].actor->getName();
				// Ignore own entity
				PxRigidActor* m_actor = buf.touches[i].actor;
				const char* name = m_actor->getName();

				// Init the lowest distance
				if (i == 0) {
					lowest_actor = m_actor;
				}

				// Ignore own entity										
				CHandle m_entity = CHandle(m_actor->userData);					
				if ((m_entity == CHandle(this).getOwner()) == false){

					if ((m_actor->getGlobalPose().p - e_transform).magnitudeSquared() < lowest_distance)
					{
						lowest_distance = (m_actor->getGlobalPose().p - e_transform).magnitudeSquared();
						lowest_actor = m_actor;
					}

				}										
			}
			TCompTransform* p_transform = (((CEntity*)player)->get<TCompTransform>());
			XMVECTOR player_pos = p_transform->position;
			TCompTransform* e_transform = ((TCompTransform*)e->get<TCompTransform>());
			XMVECTOR own_pos = e_transform->position;
			float distance = V3DISTANCE(player_pos, own_pos);
			if ((std::strcmp(lowest_actor->getName(), "Player") == 0) || (distance< 7.f))
			{
				return true;
			}
		}
	}
	return false;
}