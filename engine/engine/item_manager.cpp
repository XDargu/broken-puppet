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
	int prueba = 1;
}

void Citem_manager::removeNeedle(needle_rope n){
	std::vector<needle_rope>::iterator iter = needles.begin();
	int i = 0;
	while (iter != needles.end())
	{
		if (needles[i].needleRef==n.needleRef)
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

void Citem_manager::asociateTargetNeedle(XMVECTOR pos, float radius, CHandle grandma){
	int min_distance = 100.f;
	needle_rope priority;
	needle_rope nearest;
	int ind_priority=-1;
	int ind_nearest = -1;
	for (int i = 0; i < needles.size(); i++) {
		if ((!needles[i].grandma_asociated.isValid())){
			CEntity* owner = needles[i].needleRef.getOwner();
			TCompTransform* e_transform = ((CEntity*)owner)->get<TCompTransform>();
			if (V3DISTANCE(e_transform->position, pos) <= radius){
				if (V3DISTANCE(e_transform->position, pos) < min_distance){
					if (needles[i].rope_asociated.isValid()){
						ind_priority = i;
					}
					ind_nearest=i;
				}
			}
		}
	}
	if (ind_priority > -1){
		needles[ind_priority].grandma_asociated = grandma;
	}else{
		needles[ind_nearest].grandma_asociated = grandma;
	}
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
