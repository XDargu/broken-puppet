#include "mcv_platform.h"
#include "comp_sensor_needles.h"
#include "comp_needle.h"

void TCompSensorNeedles::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	radius = atts.getFloat("radius", 2.f);
	transform = assertRequiredComponent<TCompTransform>(this);
}

void TCompSensorNeedles::init() {
	numNeedlesInRange = 0;
	asociatedNeedle.grandma_asociated = CHandle();
	asociatedNeedle.needleRef = CHandle();
	asociatedNeedle.rope_asociated = CHandle();
}

//void TCompSensorNeedles::needleInRange(XMVECTOR pos, float radius){
	/*needlesInRangeVector.clear();
	for (auto & element : Citem_manager::get().needles) {
		CEntity* owner=element->needleRef.getOwner();
		TCompTransform* e_transform = ((CEntity*)owner)->get<TCompTransform>();
		if (V3DISTANCE(e_transform->position, pos) <= radius){
			needlesInRangeVector.push_back(element);
		}
	}*/
//}

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

/*bool TCompSensorNeedles::asociateGrandmaTargetNeedle(float max_dist_reach_needle){
	bool success = Citem_manager::get().getTargetNeedle(radius, max_dist_reach_needle);

	return success;
}*/

//bool TCompSensorNeedles::asociateGrandmaTargetNeedle(CHandle gradmaRef, float max_dist_reach_needle){

	//TCompTransform* m_transform = transform;
	//bool success = Citem_manager::get().asociateTargetNeedle(m_transform->position, radius, gradmaRef, max_dist_reach_needle);
	//bool success = Citem_manager::get().getTargetNeedle(gradmaRef, radius, max_dist_reach_needle);

	//return success;

	/*int min_distance = 100.f;
	needle_rope* priority = nullptr;
	needle_rope* nearest = nullptr;
	for (int i = 0; i < needlesInRangeVector.size(); i++) {
		if (((needlesInRangeVector)[i])->call_it == false){
			CEntity* owner = needlesInRangeVector[i]->needleRef.getOwner();
			TCompTransform* e_transform = ((CEntity*)owner)->get<TCompTransform>();
			TCompTransform* m_transform = transform;
			if (V3DISTANCE(e_transform->position, m_transform->position) < min_distance){
				if (needlesInRangeVector[i]->rope_asociated.isValid()){
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
	}*/
//}

int TCompSensorNeedles::getNumNeedles(CHandle grandmaRef, float max_dist_reach_needle){
	TCompTransform* m_transform = transform;
	int result = Citem_manager::get().getNumInRangle(grandmaRef, m_transform->position, radius, max_dist_reach_needle);
	return result;
}

CHandle TCompSensorNeedles::getNeedleAsociatedSensor(CHandle grandma){
	return Citem_manager::get().getNeedleAsociated(grandma);
}

CHandle TCompSensorNeedles::getRopeAsociatedSensor(CHandle grandma){
	return Citem_manager::get().getRopeAsociated(grandma);
}

void TCompSensorNeedles::removeNeedleRope(CHandle needleToTake){
	Citem_manager::get().removeNeedle(needleToTake);
}

void TCompSensorNeedles::setAsociatedNeedle(needle_rope needle_ref){
	asociatedNeedle = needle_ref;
}

needle_rope TCompSensorNeedles::getAsociatedNeedle(){
	return asociatedNeedle;
}