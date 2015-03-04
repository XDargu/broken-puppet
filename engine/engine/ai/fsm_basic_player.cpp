#include "mcv_platform.h"
#include "fsm_basic_player.h"
#include "../entity_manager.h"
#include "../components/all_components.h"

using namespace DirectX;

#define V3DISTANCE(x, y) XMVectorGetX(XMVector3Length(x - y))

fsm_basic_player::fsm_basic_player()
{
}

fsm_basic_player::~fsm_basic_player()
{
}

void fsm_basic_player::Init()
{
	// insert all states in the map
	AddState("fbp_Idle", (statehandler)&fsm_basic_player::Idle);
	AddState("fbp_Walk", (statehandler)&fsm_basic_player::Walk);
	AddState("fbp_Jump", (statehandler)&fsm_basic_player::Jump);
	AddState("fbp_Run", (statehandler)&fsm_basic_player::Run);
	AddState("fbp_ThrowString", (statehandler)&fsm_basic_player::ThrowString);
	AddState("fbp_Fall", (statehandler)&fsm_basic_player::Fall);
	AddState("fbp_Land", (statehandler)&fsm_basic_player::Land);
	AddState("fbp_WrongFall", (statehandler)&fsm_basic_player::WrongFall);
	AddState("fbp_WrongLand", (statehandler)&fsm_basic_player::WrongLand);
	AddState("fbp_ProcessHit", (statehandler)&fsm_basic_player::ProcessHit);
	AddState("fbp_Hurt", (statehandler)&fsm_basic_player::Hurt);
	AddState("fbp_Ragdoll", (statehandler)&fsm_basic_player::Ragdoll);
	AddState("fbp_Dead", (statehandler)&fsm_basic_player::Dead);
	AddState("fbp_ReevaluatePriorities", (statehandler)&fsm_basic_player::ReevaluatePriorities);
	AddState("fbp_WakeUp", (statehandler)&fsm_basic_player::WakeUp);

	// reset the state
	ChangeState("fbp_Idle");

	// Values
	comp_mesh = ((CEntity*)entity)->get<TCompMesh>();
	comp_unity_controller = ((CEntity*)entity)->get<TCompUnityCharacterController>();
	comp_player_pivot_transform = ((CEntity*)(CEntityManager::get().getByName("PlayerPivot")))->get<TCompTransform>();
	entity_camera = CEntityManager::get().getByName("PlayerCamera");

	last_hit = 0.f;
	state_time = 0.f;
	
}

void fsm_basic_player::Idle(){	
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Idle");
	if (((TCompUnityCharacterController*)comp_unity_controller)->IsJumping()){
		ChangeState("fbp_Jump");		
	}
	else if (EvaluateMovement()){
		ChangeState("fbp_Walk");
	}	
}

void fsm_basic_player::Walk(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Run");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	if (((TCompUnityCharacterController*)comp_unity_controller)->IsJumping()){
		ChangeState("fbp_Jump");
		return;
	}
	if (!EvaluateMovement()){
		ChangeState("fbp_Idle");
		return;
	}

}

void fsm_basic_player::Jump(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_T");
	EvaluateMovement();
	if (((TCompUnityCharacterController*)comp_unity_controller)->OnGround()){
		ChangeState("fbp_Idle");
		return;
	}
}

void fsm_basic_player::Run(){}

void fsm_basic_player::ThrowString(){}
void fsm_basic_player::Fall(){}
void fsm_basic_player::Land(){}
void fsm_basic_player::WrongFall(){}
void fsm_basic_player::WrongLand(){}
void fsm_basic_player::ProcessHit(){}

void fsm_basic_player::Hurt(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_IdleWar");
	state_time += CApp::get().delta_time;
	if (state_time >= .01f){
		state_time = 0;
		ChangeState("fbp_ReevaluatePriorities");
	}
}

void fsm_basic_player::Ragdoll(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_T");
	state_time += CApp::get().delta_time;
	if (state_time >= 0.09){// TODO AND is in ground
		state_time = 0;
		ChangeState("fbp_WakeUp");
	}
}

void fsm_basic_player::Dead(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_T");
	state_time += CApp::get().delta_time;
}

void fsm_basic_player::ReevaluatePriorities(){
	// TODO Check if is on air 
	if (false){ // if not on ground
		ChangeState("fbp_WrongFall");
		return;
	}
	
	ChangeState("fbp_Idle");
}

void fsm_basic_player::WakeUp(){
	state_time += CApp::get().delta_time;
	if (state_time >= 0.01f){
		state_time = 0;
		ChangeState("fbp_Idle");
	}
}

// NO FSM FUNCTIONS

bool fsm_basic_player::EvaluateMovement(){

	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 movement_vector = physx::PxVec3(0, 0, 0);
	physx::PxQuat rotation = Physics.XMVECTORToPxQuat(m_transform->rotation);
	bool is_moving = false;
	bool jump = false;

	if (isKeyPressed('W')){
		movement_vector += physx::PxVec3(0, 0, 1);
		is_moving = true;
	}
	if (isKeyPressed('S')){
		movement_vector += physx::PxVec3(0, 0, -1);
		is_moving = true;
	}
	if (isKeyPressed('A')){
		movement_vector += physx::PxVec3(1, 0, 0);
		is_moving = true;
	}
	if (isKeyPressed('D')){
		movement_vector += physx::PxVec3(-1, 0, 0);
		is_moving = true;
	}
	if (isKeyPressed(' ')){
		jump = true;
	}

	movement_vector = rotation.rotate(movement_vector);
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	//dir.y = 0;
	dir.normalize();	
	((TCompUnityCharacterController*)comp_unity_controller)->Move(movement_vector, false, jump, dir);

	return is_moving;
}

bool fsm_basic_player::EvaluateFall(){
	return false;
}

void fsm_basic_player::EvaluateHit(){
	if (last_hit == 0)
		return;
	if (last_hit >= 10){ //ragdoll force
		ChangeState("fbp_Ragdoll");
	}
	else if(GetState() != "fbp_Ragdoll" ){
		ChangeState("fbp_Hurt");
	}
	// Evaluate live to lose
	EvaluateLiveToLose();

	last_hit = 0.f;
}

void fsm_basic_player::EvaluateLiveToLose(){
	// Call to component live and 
	if (false)// live under or equal 0
	{
		ChangeState("fbp_Dead");
	}
}

void fsm_basic_player::localCameraFront(){
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3((((TCompTransform*)((CEntity*)entity_camera)->get<TCompTransform>())->getFront()));
	dir.y = 0;
}


void fsm_basic_player::Rerotate(){
	TCompTransform* player_pivot_trans = (TCompTransform*)comp_player_pivot_transform;

	TCompTransform* transform = (TCompTransform*)((CEntity*)entity)->get<TCompTransform>();
	float player_pivot_yaw = getYawFromVector(player_pivot_trans->getFront());
	float m_yaw = getYawFromVector(transform->getFront());
	XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(player_pivot_trans->getUp(), player_pivot_yaw - m_yaw);
	transform->rotation = XMQuaternionSlerp(transform->rotation, XMQuaternionMultiply(transform->rotation, player_pivot_rot), 0.05f);
}

