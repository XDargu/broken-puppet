#ifndef INC_COMP_SENSOR_NEEDLES_H_
#define INC_COMP_SENSOR_NEEDLES_H_

#include "base_component.h"
#include "../item_manager.h"
#include "comp_transform.h"
#include "comp_needle.h"

struct TCompSensorNeedles : TBaseComponent{
private:
	TCompTransform*		transform;
	unsigned int numNeedlesInRange;
	float radius;
public:
	std::vector<TCompNeedle*> needlesInRange;

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		radius = atts.getFloat("radius", 2.f);
	}

	void init() {
		numNeedlesInRange = 0;
	}

	std::vector<TCompNeedle*> getNeedlesInRange(){
		CEntity* e = CHandle(this).getOwner();
		transform = (TCompTransform*)e->get<TCompTransform>();
		needleInRange(transform->position, radius);
		return needlesInRange;
	}

	void needleInRange(XMVECTOR pos, float radius){
		for (auto & element : Citem_manager::get().needles) {
			TCompTransform* e_transform = element->getTransform();
			//float aux_distance = V3DISTANCE(e_transform->position, pos);
			if (V3DISTANCE(e_transform->position, pos) <= radius){
				needlesInRange.push_back(element);
			}
		}
	}
};
#endif

