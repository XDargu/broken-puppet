#include "mcv_platform.h"
#include "ai_basic_wander.h"
#include "../components/all_components.h"

using namespace DirectX;

void ai_basic_wander::Init()
{
	// insert all states in the map
	AddState("idle", (statehandler)&ai_basic_wander::IdleState);
	AddState("wander", (statehandler)&ai_basic_wander::WanderState);


	// reset the state
	ChangeState("idle");

	trans = ((CEntity*)entity)->get<TCompTransform>();
	enemyController = ((CEntity*)entity)->get<TCompEnemyController>();
	count = 0;
	wait_time = 3;

}


void ai_basic_wander::IdleState(float deltaTime)
{
	count += deltaTime;
	if (count >= wait_time){
		ChangeState("wander");
		count = 0;
	}
}

void ai_basic_wander::WanderState(float deltaTime)
{
	((TCompEnemyController*)enemyController)->addDeltaPos(((TCompTransform*)trans)->getFront() * 5 * deltaTime);
	count += deltaTime;
	if (count >= wait_time){		
		ChangeState("idle");
		count = 0;
	}
}