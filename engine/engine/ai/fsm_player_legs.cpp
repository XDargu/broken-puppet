#include "mcv_platform.h"
#include "fsm_player_legs.h"
#include "../entity_manager.h"
#include "../components/all_components.h"
#include "components\comp_skeleton.h"
#include "components\comp_skeleton_ik.h"

FSMPlayerLegs::FSMPlayerLegs()
{
}

FSMPlayerLegs::~FSMPlayerLegs()
{
}

void FSMPlayerLegs::Init()
{
	// insert all states in the map
	AddState("fbp_Idle", (statehandler)&FSMPlayerLegs::Idle);
	AddState("fbp_Walk", (statehandler)&FSMPlayerLegs::Walk);
	AddState("fbp_Jump", (statehandler)&FSMPlayerLegs::Jump);
	AddState("fbp_Run", (statehandler)&FSMPlayerLegs::Run);
	AddState("fbp_ThrowString", (statehandler)&FSMPlayerLegs::ThrowString);
	AddState("fbp_PullString", (statehandler)&FSMPlayerLegs::PullString);
	AddState("fbp_Fall", (statehandler)&FSMPlayerLegs::Fall);
	AddState("fbp_Land", (statehandler)&FSMPlayerLegs::Land);
	AddState("fbp_WrongFall", (statehandler)&FSMPlayerLegs::WrongFall);
	AddState("fbp_WrongLand", (statehandler)&FSMPlayerLegs::WrongLand);
	AddState("fbp_ProcessHit", (statehandler)&FSMPlayerLegs::ProcessHit);
	AddState("fbp_Hurt", (statehandler)&FSMPlayerLegs::Hurt);
	AddState("fbp_Ragdoll", (statehandler)&FSMPlayerLegs::Ragdoll);
	AddState("fbp_Dead", (statehandler)&FSMPlayerLegs::Dead);
	AddState("fbp_ReevaluatePriorities", (statehandler)&FSMPlayerLegs::ReevaluatePriorities);
	AddState("fbp_WakeUp", (statehandler)&FSMPlayerLegs::WakeUp);

	// reset the state
	ChangeState("fbp_Idle");

	// Values
	comp_rigidbody = ((CEntity*)entity)->get<TCompRigidBody>();
	comp_collider = ((CEntity*)entity)->get<TCompColliderCapsule>();
	comp_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	comp_skeleton_ik = ((CEntity*)entity)->get<TCompSkeletonIK>();
	comp_character_controller = ((CEntity*)entity)->get<TCompCharacterController>();
	comp_player_pivot_transform = ((CEntity*)(CEntityManager::get().getByName("PlayerPivot")))->get<TCompTransform>();
	entity_camera = CEntityManager::get().getByName("PlayerCamera");

	last_hit = 0.f;
	state_time = 0.f;
	falling = false;

	//run_speed = ((TCompCharacterController*)comp_character_controller)->moveSpeedMultiplier;
	run_speed = 6;
	((TCompCharacterController*)comp_character_controller)->jumpPower = 7;
	walk_speed = 1.5f;

	life = ((CEntity*)entity)->get<TCompLife>();

}

void FSMPlayerLegs::Idle(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;
	TCompSkeletonIK* skeleton_ik = comp_skeleton_ik;

	skeleton_ik->active = false;

	if (on_enter) {
		skeleton->loopAnimation(0);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_idle");	

	if (((TCompCharacterController*)comp_character_controller)->IsJumping()){
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_Jump");
	}
	if (EvaluateMovement(false, elapsed)){
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_Walk");
	}
	if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_ThrowString");
	}
	if (falling){
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_Fall");
	}
}

void FSMPlayerLegs::Walk(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->loopAnimation(1);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_walk");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	((TCompCharacterController*)comp_character_controller)->moveSpeedMultiplier = walk_speed;
	if (!CIOStatus::get().isPressed(CIOStatus::RUN)){
		if (((TCompCharacterController*)comp_character_controller)->IsJumping()){
			skeleton->stopAnimation(1);
			ChangeState("fbp_Jump");
			return;
		}
		if (!EvaluateMovement(true, elapsed)){
			skeleton->stopAnimation(1);
			ChangeState("fbp_Idle");
			return;
		}
		if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
			skeleton->stopAnimation(1);
			ChangeState("fbp_ThrowString");
		}
		if (falling){
			skeleton->stopAnimation(1);
			ChangeState("fbp_Fall");
		}
	}
	else{
		skeleton->stopAnimation(1);
		ChangeState("fbp_Run");
	}
}

void FSMPlayerLegs::Run(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->loopAnimation(2);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_run");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	((TCompCharacterController*)comp_character_controller)->moveSpeedMultiplier = run_speed;
	if (CIOStatus::get().isPressed(CIOStatus::RUN)){
		if (((TCompCharacterController*)comp_character_controller)->IsJumping()){
			skeleton->stopAnimation(2);
			ChangeState("fbp_Jump");
			return;
		}
		if (!EvaluateMovement(true, elapsed)){
			skeleton->stopAnimation(2);
			ChangeState("fbp_Idle");
			return;
		}
		if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
			skeleton->stopAnimation(2);
			ChangeState("fbp_ThrowString");
		}
		if (falling){
			skeleton->stopAnimation(2);
			ChangeState("fbp_Fall");
		}
	}
	else{
		skeleton->stopAnimation(2);
		ChangeState("fbp_Walk");
	}

}

void FSMPlayerLegs::Jump(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->loopAnimation(6);
		skeleton->playAnimation(5);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_jump");
	EvaluateMovement(true, elapsed);

	if (((TCompCharacterController*)comp_character_controller)->OnGround()){
		ChangeState("fbp_Idle");
		skeleton->stopAnimation(6);
		return;
	}
	if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
		ChangeState("fbp_ThrowString");
		skeleton->stopAnimation(6);
	}
	if (falling){
		ChangeState("fbp_Fall");
		skeleton->stopAnimation(6);
	}
}



void FSMPlayerLegs::ThrowString(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->playAnimation(4);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_throw");
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);

	if (state_time >= 0.1f && !(CIOStatus::get().isPressed(CIOStatus::THROW_STRING))){
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::PullString(float elapsed){
	TCompThirdPersonCameraController* camera_controller = ((CEntity*)entity_camera)->get<TCompThirdPersonCameraController>();
	camera_controller->offset = physx::PxVec3(0.5f, -0.22f, 1.07f);
	
	if (state_time > 2) {
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::Fall(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->loopAnimation(6);
	}
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_falling");
	EvaluateMovement(true, elapsed);

	if (((TCompRigidBody*)comp_rigidbody)->rigidBody->getLinearVelocity().y > 0){
		ChangeState("fbp_Jump");
		skeleton->stopAnimation(6);
	}
	else if (state_time >= 0.7f){
		ChangeState("fbp_WrongFall");
		skeleton->stopAnimation(6);
	}
	else
		if (((TCompCharacterController*)comp_character_controller)->OnGround()){
			ChangeState("fbp_Land");
			skeleton->stopAnimation(6);
		}
}

void FSMPlayerLegs::Land(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->loopAnimation(7);
	}
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_landing");
	EvaluateMovement(true, elapsed);
	if (state_time >= 0.5f){
		ChangeState("fbp_Idle");
		skeleton->stopAnimation(7);
	}
}

void FSMPlayerLegs::WrongFall(float elapsed){
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	dir.normalize();
	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_wrong_falling");
	if (((TCompCharacterController*)comp_character_controller)->OnGround()){
		ChangeState("fbp_WrongLand");
	}
}

void FSMPlayerLegs::WrongLand(float elapsed){
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_wrong_landing");

	if (state_time >= 0.2){
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::ProcessHit(float elapsed){}

void FSMPlayerLegs::Hurt(float elapsed){
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_hurt");
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	dir.normalize();
	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);

	if (state_time >= .2f){
		ChangeState("fbp_ReevaluatePriorities");
	}
}

void FSMPlayerLegs::Ragdoll(float elapsed){
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_ragdoll");

	TCompCharacterController* character_controller = (TCompCharacterController*)comp_character_controller;
	TCompRigidBody* rigidbody = (TCompRigidBody*)comp_rigidbody;
	TCompColliderCapsule* collider = (TCompColliderCapsule*)comp_collider;
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	if (on_enter) {
		collider->setMaterialProperties(1, 0.7f, 0.7f);

		rigidbody->setLockXRot(false);
		rigidbody->setLockYRot(false);
		rigidbody->setLockZRot(false);

		rigidbody->auto_rotate_transform = true;
		rigidbody->auto_translate_transform = true;
	}
	if (((state_time >= 1 && rigidbody->rigidBody->getLinearVelocity().magnitude() < 0.1f))
		|| (state_time >= 5))
	{
		rigidbody->setLockXRot(true);
		rigidbody->setLockYRot(true);
		rigidbody->setLockZRot(true);

		rigidbody->auto_rotate_transform = false;
		rigidbody->auto_translate_transform = false;

		// Volver a colocar al PJ. TODO: Mejorarlo para que no se quede atascado
		rigidbody->rigidBody->setGlobalPose(
			physx::PxTransform(
				rigidbody->rigidBody->getGlobalPose().p + physx::PxVec3(0, 1, 0),
				rigidbody->rigidBody->getGlobalPose().q
			)
		);

		collider->setMaterialProperties(0, 0, 0);

		if (((TCompLife*)life)->life <= 0){
			m_transform->rotation = XMQuaternionIdentity();
			ChangeState("fbp_Dead");
		}
		else{
			ChangeState("fbp_WakeUp");
		}

	}
}

void FSMPlayerLegs::Dead(float elapsed){
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_dead");

	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront()),elapsed);

	if (state_time >= 6){
		((TCompLife*)life)->life = 100;
		ChangeState("fbp_Idle");
	}


}

void FSMPlayerLegs::ReevaluatePriorities(){

	if (!((TCompCharacterController*)comp_character_controller)->OnGround()){
		ChangeState("fbp_WrongFall");
	}
	else{
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::WakeUp(float elapsed){

	if (state_time >= 0.01f){
		ChangeState("fbp_Idle");
	}
}

// NO FSM FUNCTIONS

bool FSMPlayerLegs::EvaluateMovement(bool lookAtCamera, float elapsed){

	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompRigidBody* rigidbody = (TCompRigidBody*)comp_rigidbody;

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

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR projected_front = XMVector3Cross(up, XMVector3Cross(camera_transform->getFront(), up));
	movement_vector = rotation.rotate(movement_vector);
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(projected_front);
	if (!lookAtCamera)
		dir = Physics.XMVECTORToPxVec3(m_transform->getFront());

	dir.normalize();
	bool ragdoll = (getCurrentNode() == "fbp_Ragdoll");
	((TCompCharacterController*)comp_character_controller)->Move(movement_vector, false, jump, dir, elapsed);

	// Evaluate falling
	physx::PxVec3 velocity = rigidbody->rigidBody->getLinearVelocity();
	falling = velocity.y <= 0 && !((TCompCharacterController*)comp_character_controller)->OnGround();

	return is_moving;
}

bool FSMPlayerLegs::EvaluateFall(float elapsed){
	EvaluateMovement(true, elapsed);
	TCompRigidBody* rigidbody = (TCompRigidBody*)comp_rigidbody;

	physx::PxVec3 velocity = rigidbody->rigidBody->getLinearVelocity();
	return velocity.y <= 0 && !((TCompCharacterController*)comp_character_controller)->OnGround();
}

void FSMPlayerLegs::EvaluateHit(float damage){

	float real_damage = 0.f;

	if (getCurrentNode() != "fbp_Dead") {		
		if (damage > 10000.f){ // Damage needed for ragdoll state
			real_damage = 20;
			ChangeState("fbp_Ragdoll");
		}
		else if (getCurrentNode() != "fbp_Ragdoll"){
			real_damage = 10;
			ChangeState("fbp_Hurt");
		}
	}

	EvaluateLiveToLose(real_damage);
	
}

void FSMPlayerLegs::EvaluateLiveToLose(float damage){
	// Call to component live and 
	if (((TCompLife*)life)->Hurt(damage))// live under or equal 0
	{
		// Change state to ragdoll instead of death, for visual purposes
		ChangeState("fbp_Ragdoll");
	}
}

void FSMPlayerLegs::localCameraFront(){
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3((((TCompTransform*)((CEntity*)entity_camera)->get<TCompTransform>())->getFront()));
	dir.y = 0;
}
