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

//bool sortDeque(needle_rope* n1, needle_rope* n2){
	/*float min_distance = 100.f;
	std::deque<needle_rope*> dequeAux;
	for (int j = 0; j < needlesInRangeVector.size(); ++j){
		for (int i = 0; i < needlesInRangeVector.size(); ++i){
			//if (!needlesInRangeVector[i]->call_it){
				TCompTransform* e_transform = needlesInRangeVector[i]->needleRef->getTransform();
				if (V3DISTANCE(e_transform->position, pos) <= min_distance){
					min_distance = V3DISTANCE(e_transform->position, pos);
					if (needlesInRangeVector[i]->rope_asociated){
						dequeAux.push_front(needlesInRangeVector[i]);
					}
					else{
						dequeAux.push_back(needlesInRangeVector[i]);
					}
				}
			//}
		}
	}
	needlesInRangeVector = dequeAux;*/	

//}

needle_rope* TCompSensorNeedles::getTargetNeedle(){
	int min_distance = 100.f;
	needle_rope* priority = nullptr;
	needle_rope* nearest = nullptr;
	for (int i = 0; i < needlesInRangeVector.size(); i++) {
		if (((needlesInRangeVector)[i])->call_it == false){
			TCompTransform* e_transform = needlesInRangeVector[i]->needleRef->getTransform();
			TCompTransform* m_transform = transform;
			if (V3DISTANCE(e_transform->position, m_transform->position) < min_distance){
				if (needlesInRangeVector[i]->rope_asociated){
					priority = needlesInRangeVector[i];
				}
				nearest = needlesInRangeVector[i];
			}
		}
	}

	if (priority != nullptr){
		return priority;
	}else{
		return nearest;
	}
}

int TCompSensorNeedles::getNumNeedles(){
	return needlesInRangeVector.size();
}