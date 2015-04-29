#include "mcv_platform.h"
#include "collision_parser.h"
#include "physics_manager.h"

void CCollisionParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "collision_table") {
		own_tag = atts.getString("own_tag", "untagged");
	}
	if (elem == "collision_filter"){
		not_collides = atts.getString("not_collides_tag", "nothing");
		convertInCollisionFilter(own_tag, not_collides);
		loaded = true;
	}
}

void CCollisionParser::convertInCollisionFilter(std::string own, std::string not_col){
	physx::PxU32 own_tag;
	physx::PxU32 not_collides;
	bool own_t = false;
	bool col_t = false;
	if (own == "Enemy"){
		own_tag = FilterGroup::eENEMY;
		own_t = true;
	}else if (own == "Actor"){
		own_tag = FilterGroup::eACTOR;
		own_t = true;
	}else if (own == "Level"){
		own_tag = FilterGroup::eLEVEL;
		own_t = true;
	}

	if (not_col == "Enemy"){
		not_collides = FilterGroup::eENEMY;
		col_t = true;
	}
	else if (not_col == "Actor"){
		not_collides = FilterGroup::eACTOR;
		col_t = true;
	}
	else if (not_col == "Level"){
		not_collides = FilterGroup::eLEVEL;
		col_t = true;
	}

	if ((own_t) && (col_t))
		CPhysicsManager::get().addCollisionFilter(own_tag, not_collides);
}

