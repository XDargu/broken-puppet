#ifndef _FSM_BOSS_H_
#define _FSM_BOSS_H_

#include "aifsmcontroller.h"
#include "physics_manager.h"
#include "entity_manager.h"

class fsm_boss : public aifsmcontroller
{
private:
	PxVec3 point_offset;
	float distance_to_point;
	CEntityManager* m_entity_manager;
	float obj_distance;
	CHandle obj_selected;
	float force;

	CHandle m_player;
public:
	fsm_boss();
	~fsm_boss();

	void Init();

	void Hidden();
	void RiseUp();

	void Idle1();
	void Stunned1();

	void Rain1();
	void Ball1();
	void Shoot1ReleaseDef();
	void Shoot1DownDef();
	void Shoot1Shoot();

	void Damaged1();

	void Idle2();
	void Stunned2();

	void Wave();
	void Rain2();
	void Ball2();
	void Shoot2();

	void Damaged2();

	void FinalState();	

	void Dead();

	void EvaluateHit();

	CHandle Player;
};

#endif