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

bool Citem_manager::asociateTargetNeedle(XMVECTOR pos, float radius, CHandle grandma, float max_dist_reach_needle, float distance_change_way_point){
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
						std::vector<XMVECTOR> path;
						CNav_mesh_manager::get().findPath(pos, e_transform->position, path);
						if (path.size() > 0){
							XMVECTOR last_point = path[path.size() - 1];
							if (V3DISTANCE(last_point, e_transform->position) <= max_dist_reach_needle - distance_change_way_point){
								if ((!needles[i].grandma_asociated.isValid()) || (needles[i].grandma_asociated == grandma)){
									min_distance_rope = V3DISTANCE(e_transform->position, pos);
									ind_priority = i;
								}
							}
						}
					}
				}else{
					if (V3DISTANCE(e_transform->position, pos) < min_distance_no_rope){
						std::vector<XMVECTOR> path;
						CNav_mesh_manager::get().findPath(pos, e_transform->position, path);
						if (path.size() > 0){
							if (V3DISTANCE(path[path.size() - 1], e_transform->position) <= max_dist_reach_needle - distance_change_way_point){
								if ((!needles[i].grandma_asociated.isValid()) || (needles[i].grandma_asociated == grandma)){
									min_distance_no_rope = (V3DISTANCE(e_transform->position, pos));
									ind_nearest = i;
								}
							}
						}
					}
				}
			}
		//}
	}
	if (ind_priority > -1){
		//SI ESTA ABUELA TIENE ASOCIADA OTRA NEEDLE_ROPE, TENDRE QUE DESASOCIARLA Y ASOCIAR LA MAS PRIORITARIA
		DesAsociatePriorityNeedleRope(grandma);
		if ((!needles[ind_priority].grandma_asociated.isValid()) || (needles[ind_priority].grandma_asociated==grandma)){
			needles[ind_priority].grandma_asociated = grandma;
			success = true;
		}
	}else if (ind_nearest > -1){
		//SI ESTA ABUELA TIENE ASOCIADA OTRA NEEDLE_ROPE, TENDRE QUE DESASOCIARLA Y ASOCIAR LA MAS PRIORITARIA
		//bool can_asociate_another=DesAsociateNoPriorityNeedleRope(grandma);
		DesAsociatePriorityNeedleRope(grandma);
		//if (can_asociate_another){
			if ((!needles[ind_nearest].grandma_asociated.isValid()) || (needles[ind_nearest].grandma_asociated == grandma)){
				needles[ind_nearest].grandma_asociated = grandma;
				success = true;
			}
		//}
	}
	return success;
}

int Citem_manager::getNumInRangle(CHandle grandmaRef, XMVECTOR pos, float radius, float max_dist_reach_needle, float distance_change_way_point){
	int result = 0;
	for (auto & element : needles) {

		XASSERT(element.needleRef.isValid(), "Invalid needle");

		CEntity* owner = element.needleRef.getOwner();
		TCompTransform* e_transform = ((CEntity*)owner)->get<TCompTransform>();
		if (V3DISTANCE(e_transform->position, pos) <= radius){
			std::vector<XMVECTOR> path;
			CNav_mesh_manager::get().findPath(pos, e_transform->position, path);
			if (path.size() > 0){
				int distance_prueba = V3DISTANCE(path[path.size() - 1], e_transform->position);
				if (V3DISTANCE(path[path.size() - 1], e_transform->position) <= max_dist_reach_needle - distance_change_way_point){
					if ((!element.grandma_asociated.isValid()) || (element.grandma_asociated==grandmaRef))
						result++;
				}
			}
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
	if (previous_asociated)
		return true;
	else
		return false;
}

CHandle Citem_manager::getNeedleAsociated(CHandle grandma){
	for (auto & element : needles) {

		XASSERT(element.needleRef.isValid(), "Invalid needle");
		if (element.grandma_asociated == grandma){
			return element.needleRef;
		}
	}

	return CHandle();
}

CHandle Citem_manager::getRopeAsociated(CHandle grandma){
	for (auto & element : needles) {

		if (element.grandma_asociated == grandma){
			return element.rope_asociated;
		}
	}

	return CHandle();
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
