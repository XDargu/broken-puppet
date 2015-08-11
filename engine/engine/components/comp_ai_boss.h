#ifndef INC_COMP_AI_BOSS_H_
#define INC_COMP_AI_BOSS_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompAiBoss : TBaseComponent {
private:
	CHandle mPlayer;
	CHandle mBoss;
	PxVec3 point_to_go;
	PxVec3 point_offset;
	float distance_to_point;
	float force;

	bool activate;
	bool shoot;
	bool debris_debris;

	float debris_creation_delay;
	int debris_created;
	int move_things;

public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts){};
	void init();
	void fixedUpdate(float elapsed);
	void update(float elapsed);
	
};

#endif