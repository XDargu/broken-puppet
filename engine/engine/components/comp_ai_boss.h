#ifndef INC_COMP_AI_BOSS_H_
#define INC_COMP_AI_BOSS_H_

#include "base_component.h"
#include "physics_manager.h"
#include "../ai/fsm_boss.h"

struct TCompAiBoss : TBaseComponent {
private:
	fsm_boss m_fsm_boss;

	CHandle mPlayer;
	CHandle player_trans;
	CHandle mBoss;
	CHandle boss_trans;

	CHandle comp_skeleton;

	CHandle R_hitch;
	PxFixedJoint* R_hitch_joint;

	CHandle L_hitch;
	PxFixedJoint* L_hitch_joint;

	PxVec3 point_to_go;
	PxVec3 point_offset;
	float distance_to_point;
	float force;

	bool activate;
	bool shoot;
	bool debris_debris;
	bool attack2Active;
	float attack2Time;

	float debris_creation_delay;
	int debris_created;
	int move_things;

public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void fixedUpdate(float elapsed);
	void update(float elapsed);

	void breakHitch(CHandle m_hitch);

	bool can_break_hitch;
	
};

#endif