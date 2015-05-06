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

	own_tag = convertStrInCollisionFilter(own);
	if (own_tag != FilterGroup::eUNDEFINED)
	{
		own_t = true;
	}
	else
	{
		int i = 0;
	}

	not_collides = convertStrInCollisionFilter(not_col);
	if (not_collides != FilterGroup::eUNDEFINED) col_t = true;

	if ((own_t) && (col_t))
		CPhysicsManager::get().addCollisionFilter(own_tag, not_collides);
}

