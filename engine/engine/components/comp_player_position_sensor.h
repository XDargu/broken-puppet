#ifndef INC_COMP_PLAYER_POSITION_SENSOR_H_
#define INC_COMP_PLAYER_POSITION_SENSOR_H_

#include "base_component.h"
#include "../item_manager.h"
#include "comp_transform.h"
using namespace DirectX;

#define V3DISTANCE(x, y) XMVectorGetX(XMVector3Length(x - y))

struct TCompPlayerPosSensor : TBaseComponent{
private:
	TCompTransform*		e_transform;
	TCompTransform*     p_transform;
	float radius;
public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		radius = atts.getFloat("radius", 2.f);
	}

	void init() {
	}

	bool playerInRange(){
		CEntity* player = CEntityManager::get().getByName("Player");
		p_transform = ((CEntity*)player)->get<TCompTransform>();
		CEntity* e = CHandle(this).getOwner();
		e_transform = (TCompTransform*)e->get<TCompTransform>();
		float prueba = V3DISTANCE(p_transform->position, e_transform->position);
		if (V3DISTANCE(p_transform->position, e_transform->position) < radius){
			physx::PxRaycastBuffer buf;
			Physics.raycastAll(e_transform->position + XMVectorSet(0, 1.5f, 0, 0), XMVector3Normalize(p_transform->position - e_transform->position), radius, buf);

			bool player_visible = false;
			if (buf.nbTouches > 1){
				if (std::strcmp(buf.touches[1].actor->getName(), "Player") == 0) {
					return true;
				}
			}else{
				return false;
			}
		}else{
			return false;
		}

		return STAY;
	}
};
#endif
