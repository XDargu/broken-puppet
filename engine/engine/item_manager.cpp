#include "mcv_platform.h"
#include "item_manager.h"
#include "components\all_components.h"

static Citem_manager the_item_manager;

Citem_manager& Citem_manager::get() {
	return the_item_manager;
}

Citem_manager::Citem_manager()
{
}


Citem_manager::~Citem_manager()
{
}

void Citem_manager::addNeedle(CHandle needle, CHandle rope){
	needle_rope needle_rope_struct;
	needle_rope_struct.needleRef = needle;
	needle_rope_struct.rope_asociated = rope;
	needle_rope_struct.grandma_asociated = CHandle();
	needles.push_back(needle_rope_struct);
}

void Citem_manager::removeNeedle(CHandle n){
	std::vector<needle_rope>::iterator iter = needles.begin();
	int i = 0;
	while (iter != needles.end())
	{
		if (needles[i].needleRef==n)
		{
			iter = needles.erase(iter);
			return;
		}
		else
		{
			++iter;
			++i;
		}
	}
}

bool Citem_manager::asociateTargetNeedle(XMVECTOR pos, float radius, CHandle grandma){
	bool success = false;
	int min_distance_rope = 100.f;
	int min_distance_no_rope = 100.f;
	int ind_priority=-1;
	int ind_nearest = -1;
	for (int i = 0; i < needles.size(); i++) {
		//if ((!needles[i].grandma_asociated.isValid())){
			CEntity* owner = needles[i].needleRef.getOwner();
			TCompTransform* e_transform = ((CEntity*)owner)->get<TCompTransform>();
			if (V3DISTANCE(e_transform->position, pos) <= radius){
				if (needles[i].rope_asociated.isValid()){
					if (V3DISTANCE(e_transform->position, pos) < min_distance_rope){
						min_distance_rope = V3DISTANCE(e_transform->position, pos);
						ind_priority = i;
					}
				}else{
					if (V3DISTANCE(e_transform->position, pos) < min_distance_no_rope){
						min_distance_no_rope = (V3DISTANCE(e_transform->position, pos));
						ind_nearest = i;
						success = true;
					}
				}
			}
		//}
	}
	if (ind_priority > -1){
		//SI ESTA ABUELA TIENE ASOCIADA OTRA NEEDLE_ROPE, TENDRE QUE DESASOCIARLA Y ASOCIAR LA MAS PRIORITARIA
		DesAsociatePriorityNeedleRope(grandma);
		needles[ind_priority].grandma_asociated = grandma;
		success = true;
	}else if (ind_nearest > -1){
		//SI ESTA ABUELA TIENE ASOCIADA OTRA NEEDLE_ROPE, TENDRE QUE DESASOCIARLA Y ASOCIAR LA MAS PRIORITARIA
		bool can_asociate_another=DesAsociateNoPriorityNeedleRope(grandma);
		if (can_asociate_another){
			needles[ind_nearest].grandma_asociated = grandma;
			success = true;
		}
	}
	return success;
}

int Citem_manager::getNumInRangle(XMVECTOR pos, float radius){
	int result = 0;
	for (auto & element : needles) {

		XASSERT(element.needleRef.isValid(), "Invalid needle");

		CEntity* owner = element.needleRef.getOwner();
		TCompTransform* e_transform = ((CEntity*)owner)->get<TCompTransform>();
		if (V3DISTANCE(e_transform->position, pos) <= radius){
			result++;
		}
	}
	return result;
}

void Citem_manager::DesAsociatePriorityNeedleRope(CHandle grandma){
	for (int i = 0; i < needles.size();i++) {

		if (needles[i].grandma_asociated == grandma){
			needles[i].grandma_asociated = CHandle();
			return;
		}
	}
}

bool Citem_manager::DesAsociateNoPriorityNeedleRope(CHandle grandma){
	bool previous_asociated = false;
	for (int i = 0; i < needles.size(); i++) {

		if (needles[i].grandma_asociated == grandma){
			previous_asociated = true;
			if (!needles[i].rope_asociated.isValid()){
				needles[i].grandma_asociated = CHandle();
				//The needle previously asociated don't have any rope so can asociate another
				return true;
			}else{
				return false;
			}
		}
	}
	return true;
}

CHandle Citem_manager::getNeedleAsociated(CHandle grandma){
	for (auto & element : needles) {

		XASSERT(element.needleRef.isValid(), "Invalid needle");
		if (element.grandma_asociated == grandma){
			return element.needleRef;
		}
	}
}

CHandle Citem_manager::getRopeAsociated(CHandle grandma){
	for (auto & element : needles) {

		if (element.grandma_asociated == grandma){
			return element.rope_asociated;
		}
	}
}

/*std::vector<TCompNeedle*> Citem_manager::needleInRange(XMVECTOR pos, float radius){
	std::vector<TCompNeedle*> needlesNear;
	for (auto & element : needles) {
		TCompTransform* e_transform = element->getTransform();
		//float aux_distance = V3DISTANCE(e_transform->position, pos);
		if (V3DISTANCE(e_transform->position, pos) <= radius){
			needlesNear.push_back(element);
		}
	}
	return needlesNear;
}*/
