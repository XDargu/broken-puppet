#ifndef _FSM_BOSS_H_
#define _FSM_BOSS_H_

#include "aifsmcontroller.h"

class fsm_boss : public aifsmcontroller
{

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
	void Shoot1();

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