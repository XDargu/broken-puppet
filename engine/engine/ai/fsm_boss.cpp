#include "mcv_platform.h"
#include "fsm_boss.h"
#include "../entity_manager.h"
#include "../components/all_components.h"

using namespace DirectX;

fsm_boss::fsm_boss()
{
}

fsm_boss::~fsm_boss()
{
}

void fsm_boss::Init()
{
	// insert all states in the map

	AddState("fbp_Hidden", (statehandler)&fsm_boss::Hidden);
	AddState("fbp_RiseUp", (statehandler)&fsm_boss::RiseUp);
	AddState("fbp_Idle1", (statehandler)&fsm_boss::Idle1);
	AddState("fbp_Stunned1", (statehandler)&fsm_boss::Stunned1);
	AddState("fbp_Rain1", (statehandler)&fsm_boss::Rain1);
	AddState("fbp_Ball1", (statehandler)&fsm_boss::Ball1);
	AddState("fbp_Shoot1", (statehandler)&fsm_boss::Shoot1);
	AddState("fbp_Damaged1l", (statehandler)&fsm_boss::Damaged1);
	AddState("fbp_Idle2", (statehandler)&fsm_boss::Idle2);
	AddState("fbp_Stunned2", (statehandler)&fsm_boss::Stunned2);
	AddState("fbp_Wave", (statehandler)&fsm_boss::Wave);
	AddState("fbp_Rain2", (statehandler)&fsm_boss::Rain2);
	AddState("fbp_Ball2", (statehandler)&fsm_boss::Ball2);
	AddState("fbp_Shoot2", (statehandler)&fsm_boss::Shoot2);
	AddState("fbp_Damaged2", (statehandler)&fsm_boss::Damaged2);
	AddState("fbp_FinalState", (statehandler)&fsm_boss::FinalState);
	AddState("fbp_Dead", (statehandler)&fsm_boss::Dead);

	// reset the state
	ChangeState("fbp_Hidden");

	state_time = 0.f;
}

void fsm_boss::Hidden(){
	int i = 0;
	ChangeState("fbp_RiseUp");
}

void fsm_boss::RiseUp(){
	int i = 0;
	ChangeState("fbp_Idle1");
}

void fsm_boss::Idle1(){
	int i = 0;
}

void fsm_boss::Stunned1(){
	int i = 0;
}

void fsm_boss::Rain1(){
	int i = 0;
}

void fsm_boss::Ball1(){
	int i = 0;
}

//Shoot1
void fsm_boss::Shoot1(){
	int i = 0;
}

//Damaged1
void fsm_boss::Damaged1(){
	int i = 0;
}

//Idle2
void fsm_boss::Idle2(){
	int i = 0;
}

//Stunned2
void fsm_boss::Stunned2(){
	int i = 0;
}

//Wave
void fsm_boss::Wave(){
	int i = 0;
}

//Rain2
void fsm_boss::Rain2(){
	int i = 0;
}

//Ball2
void fsm_boss::Ball2(){
	int i = 0;
}

//Shoot2
void fsm_boss::Shoot2(){
	int i = 0;
}

//Damaged2
void fsm_boss::Damaged2(){
	int i = 0;
}

//FinalState
void fsm_boss::FinalState(){
	int i = 0;
}

//Dead
void fsm_boss::Dead(){
	int i = 0;
}