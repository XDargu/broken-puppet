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
	falling = false;

	run_speed = ((TCompUnityCharacterController*)comp_unity_controller)->moveSpeedMultiplier;;
	((TCompUnityCharacterController*)comp_unity_controller)->jumpPower = 7;
	walk_speed = run_speed / 2;

	life = ((CEntity*)entity)->get<TCompLife>();
	
}

void fsm_basic_player::Idle(){	
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_idle");	

	if (((TCompUnityCharacterController*)comp_unity_controller)->IsJumping()){
		ChangeState("fbp_Jump");		
	}
	if (EvaluateMovement(false)){
		ChangeState("fbp_Walk");
	}	
	if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
		ChangeState("fbp_ThrowString");
	}
	if (falling){
		ChangeState("fbp_Fall");
	}
}

void fsm_basic_player::Walk(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_walk");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	((TCompUnityCharacterController*)comp_unity_controller)->moveSpeedMultiplier = walk_speed;
	if (!CIOStatus::get().isPressed(CIOStatus::RUN)){
		if (((TCompUnityCharacterController*)comp_unity_controller)->IsJumping()){
			ChangeState("fbp_Jump");
			return;
		}
		if (!EvaluateMovement(true)){
			ChangeState("fbp_Idle");
			return;
		}
		if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
			ChangeState("fbp_ThrowString");
		}
		if (falling){
			ChangeState("fbp_Fall");
		}
	}
	else{		
		ChangeState("fbp_Run");
	}
}

void fsm_basic_player::Run(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_run");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	((TCompUnityCharacterController*)comp_unity_controller)->moveSpeedMultiplier = run_speed;
	if (CIOStatus::get().isPressed(CIOStatus::RUN)){
		if (((TCompUnityCharacterController*)comp_unity_controller)->IsJumping()){
			ChangeState("fbp_Jump");
			return;
		}
		if (!EvaluateMovement(true)){
			ChangeState("fbp_Idle");
			return;
		}
		if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
			ChangeState("fbp_ThrowString");
		}
		if (falling){
			ChangeState("fbp_Fall");
		}
	}
	else{
		ChangeState("fbp_Walk");
	}

}

void fsm_basic_player::Jump(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_jump");
	EvaluateMovement(true);

	if (((TCompUnityCharacterController*)comp_unity_controller)->OnGround()){
		ChangeState("fbp_Idle");
		return;
	}
	if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
		ChangeState("fbp_ThrowString");
	}
	if (falling){
		ChangeState("fbp_Fall");
	}
}



void fsm_basic_player::ThrowString(float elapsed){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_throw");
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0,0,0), false, false, dir);
	state_time += elapsed;
	if (state_time >= 0.1f && !(CIOStatus::get().isPressed(CIOStatus::THROW_STRING))){
		state_time = 0;
		ChangeState("fbp_Idle");
	}	
}

void fsm_basic_player::Fall(float elapsed){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_falling");
	EvaluateMovement(true);
	state_time += elapsed;

	if (((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->getLinearVelocity().y > 0){
		ChangeState("fbp_Jump");
		state_time = 0;
	}
	else if (state_time >= 0.7f){
		ChangeState("fbp_WrongFall");
		state_time = 0;
	}else
	if (((TCompUnityCharacterController*)comp_unity_controller)->OnGround()){
		ChangeState("fbp_Land");
		state_time = 0;
	}

}

void fsm_basic_player::Land(float elapsed){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_landing");
	state_time += elapsed;
	if (state_time >= 0.2){
		state_time = 0;
		ChangeState("fbp_Idle");		
	}
}

void fsm_basic_player::WrongFall(){
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	dir.normalize();
	((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir);
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_wrong_falling");
	if (((TCompUnityCharacterController*)comp_unity_controller)->OnGround()){
		ChangeState("fbp_WrongLand");
	}
}

void fsm_basic_player::WrongLand(float elapsed){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_wrong_landing");
	state_time += elapsed;
	if (state_time >= 0.2){
		state_time = 0;
		ChangeState("fbp_Idle");
	}
}

void fsm_basic_player::ProcessHit(){}

void fsm_basic_player::Hurt(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_hurt");
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	dir.normalize();
	((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0,0,0), false, false, dir);
	state_time += CApp::get().delta_time;
	if (state_time >= .2f){
		state_time = 0;
		ChangeState("fbp_ReevaluatePriorities");
	}
}

void fsm_basic_player::Ragdoll(float elapsed){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_ragdoll");
	state_time += elapsed;

	physx::PxMaterial* mat;
	((TCompUnityCharacterController*)comp_unity_controller)->enemy_collider->getMaterials(&mat, 1);
	mat->setRestitution(1);

	((TCompUnityCharacterController*)comp_unity_controller)->mJoint->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eFREE);
	((TCompUnityCharacterController*)comp_unity_controller)->mJoint->setMotion(physx::PxD6Axis::eSWING2, physx::PxD6Motion::eFREE);
	((TCompUnityCharacterController*)comp_unity_controller)->mJoint->setMotion(physx::PxD6Axis::eTWIST, physx::PxD6Motion::eFREE);
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	m_transform->rotation =
		Physics.PxQuatToXMVECTOR(
			((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->getGlobalPose().q
		);	

	if (((state_time >= 1 && ((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->getLinearVelocity().magnitude() < 0.1f))
		|| (state_time >= 5))
	{
		state_time = 0;
		((TCompUnityCharacterController*)comp_unity_controller)->mJoint->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eLOCKED);
		((TCompUnityCharacterController*)comp_unity_controller)->mJoint->setMotion(physx::PxD6Axis::eSWING2, physx::PxD6Motion::eLOCKED);
		((TCompUnityCharacterController*)comp_unity_controller)->mJoint->setMotion(physx::PxD6Axis::eTWIST, physx::PxD6Motion::eLOCKED);
		((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->setGlobalPose(
			physx::PxTransform(
				((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->getGlobalPose().p + physx::PxVec3(0, 1, 0),
				((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->getGlobalPose().q
			)
			);
		mat->setRestitution(0);
		

		if (((TCompLife*)life)->life <= 0){
			m_transform->rotation = XMQuaternionIdentity();
			ChangeState("fbp_Dead");
		}
		else{
			ChangeState("fbp_WakeUp");
		}
		
	}
}

void fsm_basic_player::Dead(float elapsed){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_dead");
	
	((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront()));

	state_time += elapsed;
	if(state_time >= 6){
		((TCompLife*)life)->life = 100;
		ChangeState("fbp_Idle");
	}
	

}

void fsm_basic_player::ReevaluatePriorities(){
	
	if (!((TCompUnityCharacterController*)comp_unity_controller)->OnGround()){ 
		ChangeState("fbp_WrongFall");
	}
	else{
		ChangeState("fbp_Idle");
	}
}

void fsm_basic_player::WakeUp(){
	state_time += CApp::get().delta_time;
	if (state_time >= 0.01f){
		state_time = 0;
		ChangeState("fbp_Idle");
	}
}

// NO FSM FUNCTIONS

bool fsm_basic_player::EvaluateMovement(bool lookAtCamera){

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
	if (!lookAtCamera)
		dir = Physics.XMVECTORToPxVec3(m_transform->getFront());
	
	dir.normalize();	
	bool ragdoll = (GetState() == "fbp_Ragdoll");
	((TCompUnityCharacterController*)comp_unity_controller)->Move(movement_vector, false, jump, dir);

	// Evaluate falling
	physx::PxVec3 velocity = ((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody->getLinearVelocity();
	falling = velocity.y <= 0 && !((TCompUnityCharacterController*)comp_unity_controller)->OnGround();

	return is_moving;
}

bool fsm_basic_player::EvaluateFall(){
	EvaluateMovement(true);
	physx::PxVec3 velocity = ((TCompRigidBody*)((TCompUnityCharacterController*)comp_unity_controller)->enemy_rigidbody)->rigidBody->getLinearVelocity();
	return velocity.y <= 0 && !((TCompUnityCharacterController*)comp_unity_controller)->OnGround();
}

void fsm_basic_player::EvaluateHit(){

	if (GetState() != "fbp_Dead") {
		if (last_hit == 0)
			return;
		float damage = 0;
		if (last_hit >= 10){ //ragdoll force
			damage = 20;
			ChangeState("fbp_Ragdoll");
		}
		else if (GetState() != "fbp_Ragdoll"){
			damage = 10;
			ChangeState("fbp_Hurt");
		}
		// Evaluate live to lose
		EvaluateLiveToLose(damage);
		last_hit = 0.f;
	}
}

void fsm_basic_player::EvaluateLiveToLose(float damage){
	// Call to component live and 
	if (((TCompLife*)life)->Hurt(damage))// live under or equal 0
	{
		ChangeState("fbp_Ragdoll");
	}
}

void fsm_basic_player::localCameraFront(){
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3((((TCompTransform*)((CEntity*)entity_camera)->get<TCompTransform>())->getFront()));
	dir.y = 0;
}
