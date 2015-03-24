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

void Citem_manager::addNeedle(TCompNeedle* n){
	needles.push_back(n);
}

void Citem_manager::removeNeedle(TCompNeedle* n){
	std::vector<TCompNeedle*>::iterator iter = needles.begin();
	while (iter != needles.end())
	{
		if (*iter == n)
		{
			iter = needles.erase(iter);
			return;
		}
		else
		{
			++iter;
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
