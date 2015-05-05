#include "mcv_platform.h"
#include "comp_sensor_needles.h"
#include "comp_needle.h"

void TCompSensorNeedles::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	radius = atts.getFloat("radius", 2.f);
	transform = assertRequiredComponent<TCompTransform>(this);
}

void TCompSensorNeedles::init() {
	numNeedlesInRange = 0;
}

void TCompSensorNeedles::getNeedlesInRange(std::vector<TCompNeedle*>* needlesInRangeVector){
	TCompTransform* m_transform = transform;
	needleInRange(needlesInRangeVector, m_transform->position, radius);
}

void TCompSensorNeedles::needleInRange(std::vector<TCompNeedle*>* needlesInRangeVector, XMVECTOR pos, float radius){
	for (auto & element : Citem_manager::get().needles) {
		TCompTransform* e_transform = element->getTransform();
		if (V3DISTANCE(e_transform->position, pos) <= radius){
			needlesInRangeVector->push_back(element);
		}
	}
}
