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

void Citem_manager::addNeedle(TCompNeedle* needle, TCompRope* rope){
	needle_rope needle_rope_struct;
	needle_rope_struct.needleRef = needle;
	needle_rope_struct.rope_asociated = rope;
	needle_rope_struct.call_it = false;
	needles.push_back(needle_rope_struct);
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
