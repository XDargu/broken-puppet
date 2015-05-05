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

void TCompSensorNeedles::getNeedlesInRange(){
	TCompTransform* m_transform = transform;
	needleInRange(m_transform->position, radius);
}

void TCompSensorNeedles::needleInRange(XMVECTOR pos, float radius){
	needlesInRangeVector.clear();
	for (auto & element : Citem_manager::get().needles) {
		TCompTransform* e_transform = element.needleRef->getTransform();
		if (V3DISTANCE(e_transform->position, pos) <= radius){
			needlesInRangeVector.push_back(&element);
		}
	}
}

needle_rope* TCompSensorNeedles::getTargetNeedle(){
	int i = 0;
	bool find = false;
	do{
		if (((needlesInRangeVector)[i])->call_it == false){
			find = true;
			return (needlesInRangeVector)[i];
		}
		i++;
	} while ((i < needlesInRangeVector.size()) && (!find));
}

int TCompSensorNeedles::getNumNeedles(){
	return needlesInRangeVector.size();
}