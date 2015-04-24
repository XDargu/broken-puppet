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
	if (own == "Enemy"){
		own_tag = FilterGroup::eENEMY;
	}else if (own == "Actor"){
		own_tag = FilterGroup::eACTOR;
	}else if (own == "Level"){
		own_tag = FilterGroup::eLEVEL;
	}

	if (not_col == "Enemy"){
		not_collides = FilterGroup::eENEMY;
	}
	else if (not_col == "Actor"){
		not_collides = FilterGroup::eACTOR;
	}
	else if (not_col == "Level"){
		not_collides = FilterGroup::eLEVEL;
	}
	CPhysicsManager::get().addCollisionFilter(own_tag, not_collides);
}

