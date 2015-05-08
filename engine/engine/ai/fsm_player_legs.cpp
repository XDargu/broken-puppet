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
	AddState("fbp_ThrowStringPartial", (statehandler)&FSMPlayerLegs::ThrowStringPartial);
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
	comp_ragdoll = ((CEntity*)entity)->get<TCompRagdoll>();
	comp_skeleton_ik = ((CEntity*)entity)->get<TCompSkeletonIK>();
	comp_character_controller = ((CEntity*)entity)->get<TCompCharacterController>();
	comp_player_pivot_transform = ((CEntity*)(CEntityManager::get().getByName("PlayerPivot")))->get<TCompTransform>();
	entity_camera = CEntityManager::get().getByName("PlayerCamera");

	last_hit = 0.f;
	state_time = 0.f;
	falling = false;
	canThrow = false;

	//run_speed = ((TCompCharacterController*)comp_character_controller)->moveSpeedMultiplier;
	run_speed = 6;
	((TCompCharacterController*)comp_character_controller)->jumpPower = 7;
	walk_speed = 1.5f;

	((TCompCharacterController*)comp_character_controller)->lerpRotation = 0.15;

	current_animation_id = -1;

	life = ((CEntity*)entity)->get<TCompLife>();

}

void FSMPlayerLegs::Idle(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;
	TCompSkeletonIK* skeleton_ik = comp_skeleton_ik;

	canThrow = true;

	skeleton_ik->active = false;// state_time > 0.3f;
	
	int animation = torso->up_animation ? 8 : 0;

	if (animation != current_animation_id) {
		skeleton->stopAnimation(current_animation_id);
		skeleton->loopAnimation(animation);
		current_animation_id = animation;
	}	

	if (on_enter) {
		skeleton->loopAnimation(animation);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_idle");	

	if (((TCompCharacterController*)comp_character_controller)->IsJumping()){
		skeleton->stopAnimation(8);
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_Jump");
	}
	if (EvaluateMovement(false, elapsed)){
		skeleton->stopAnimation(8);
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_Walk");
	}
	if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
		skeleton->stopAnimation(8);
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_ThrowString");
	}
	if (falling){
		skeleton->stopAnimation(8);
		skeleton->stopAnimation(0);
		skeleton_ik->active = false;
		ChangeState("fbp_Fall");
	}
}

void FSMPlayerLegs::Walk(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	canThrow = true;
	
	int animation = torso->up_animation ? 9 : 1;

	if (movement_dir.z == 0) {
		if (movement_dir.x < 0) {
			animation = torso->up_animation ? 25 : 13;
		}
		else if (movement_dir.x > 0) {
			animation = torso->up_animation ? 24 : 12;
		}
	}
	else if (movement_dir.z < 0) {
		animation = torso->up_animation ? 22 : 16;
	}

	if (animation != current_animation_id) {
		skeleton->stopAnimation(current_animation_id);
		skeleton->loopAnimation(animation);
		current_animation_id = animation;
	}

	if (on_enter) {
		skeleton->loopAnimation(animation);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_walk");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	((TCompCharacterController*)comp_character_controller)->moveSpeedMultiplier = walk_speed;
	if (!CIOStatus::get().isPressed(CIOStatus::RUN)){
		if (((TCompCharacterController*)comp_character_controller)->IsJumping()){
			skeleton->stopAnimation(9);
			skeleton->stopAnimation(1);
			ChangeState("fbp_Jump");
			return;
		}
		if (!EvaluateMovement(true, elapsed)){
			skeleton->stopAnimation(9);
			skeleton->stopAnimation(1);
			ChangeState("fbp_Idle");
			return;
		}
		if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
			skeleton->stopAnimation(9);
			skeleton->stopAnimation(1);
			ChangeState("fbp_ThrowStringPartial");
		}
		if (falling){
			skeleton->stopAnimation(9);
			skeleton->stopAnimation(1);
			ChangeState("fbp_Fall");
		}
	}
	else{
		skeleton->stopAnimation(9);
		skeleton->stopAnimation(1);
		ChangeState("fbp_Run");
	}
}

void FSMPlayerLegs::Run(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	canThrow = true;

	int animation = torso->up_animation ? 10 : 2;

	if (movement_dir.z == 0) {
		if (movement_dir.x < 0) {
			animation = torso->up_animation ? 25 : 15;
		}
		else if (movement_dir.x > 0) {
			animation = torso->up_animation ? 26 : 14;
		}
	}
	else if (movement_dir.z < 0) {
		animation = torso->up_animation ? 23 : 21;
	}

	if (animation != current_animation_id) {
		skeleton->stopAnimation(current_animation_id);
		skeleton->loopAnimation(animation);
		current_animation_id = animation;
	}

	if (on_enter) {
		skeleton->loopAnimation(animation);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_run");
	//((TCompUnityCharacterController*)comp_unity_controller)->Move(physx::PxVec3(0, 0, 1), false, false, physx::PxVec3(0, 0, 1));
	((TCompCharacterController*)comp_character_controller)->moveSpeedMultiplier = run_speed;
	if (CIOStatus::get().isPressed(CIOStatus::RUN)){
		if (((TCompCharacterController*)comp_character_controller)->IsJumping()){
			skeleton->stopAnimation(10);
			skeleton->stopAnimation(2);
			ChangeState("fbp_Jump");
			return;
		}
		if (!EvaluateMovement(true, elapsed)){
			skeleton->stopAnimation(10);
			skeleton->stopAnimation(2);
			ChangeState("fbp_Idle");
			return;
		}
		if (CIOStatus::get().isPressed(CIOStatus::THROW_STRING)){
			skeleton->stopAnimation(10);
			skeleton->stopAnimation(2);
			ChangeState("fbp_ThrowStringPartial");
		}
		if (falling){
			skeleton->stopAnimation(10);
			skeleton->stopAnimation(2);
			ChangeState("fbp_Fall");
		}
	}
	else{
		skeleton->stopAnimation(10);
		skeleton->stopAnimation(2);
		ChangeState("fbp_Walk");
	}

}

void FSMPlayerLegs::Jump(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	canThrow = true;

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
		ChangeState("fbp_ThrowStringPartial");
		skeleton->stopAnimation(6);
	}
	if (falling){
		ChangeState("fbp_Fall");
		skeleton->stopAnimation(6);
	}
}

void FSMPlayerLegs::ThrowString(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	canThrow = false;
	
	int animation = torso->up_animation ? 19 : 4;

	if (on_enter) {
		skeleton->loopAnimation(0);
		skeleton->playAnimation(animation);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_throw");
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);

	if (state_time >= skeleton->getAnimationDuration(4) && !(CIOStatus::get().isPressed(CIOStatus::THROW_STRING))){
		skeleton->stopAnimation(0);
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::ThrowStringPartial(float elapsed){

	canThrow = false;

	TCompSkeleton* skeleton = comp_skeleton;

	int animation = torso->up_animation ? 20 : 11;

	if (on_enter) {
		skeleton->playAnimation(animation);
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

	TCompSkeleton* skeleton = comp_skeleton;

	canThrow = false;

	if (on_enter) {
		skeleton->loopAnimation(15);
	}

	TCompThirdPersonCameraController* camera_controller = ((CEntity*)entity_camera)->get<TCompThirdPersonCameraController>();
	camera_controller->offset = physx::PxVec3(0.5f, -0.22f, 1.07f);
	
	if (state_time > 2) {
		skeleton->stopAnimation(15);
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::Fall(float elapsed){
	TCompSkeleton* skeleton = comp_skeleton;

	canThrow = false;

	if (on_enter) {
		skeleton->loopAnimation(6);
	}
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_falling");
	EvaluateMovement(true, elapsed);

	if (((TCompRigidBody*)comp_rigidbody)->rigidBody->getLinearVelocity().y > 0){
		ChangeState("fbp_Jump");
		skeleton->stopAnimation(6);
	}
	else if (state_time >= 1.1f){
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

	canThrow = false;

	TCompSkeleton* skeleton = comp_skeleton;
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();

	if (on_enter) {
		skeleton->playAnimation(7);
	}

	if (state_time > 0.2f) {
		physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
		dir.normalize();
		((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);
	}
	else {
		EvaluateMovement(true, elapsed);
	}

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_landing");
	
	if (state_time >= 0.5f){
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::WrongFall(float elapsed){

	canThrow = false;

	TCompSkeleton* skeleton = comp_skeleton;

	if (on_enter) {
		skeleton->loopAnimation(6);
	}

	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
	dir.normalize();

	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);
	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_wrong_falling");
	if (((TCompCharacterController*)comp_character_controller)->OnGround()){
		skeleton->stopAnimation(6);
		//ChangeState("fbp_WrongLand");
		ChangeState("fbp_Ragdoll");
	}
}

void FSMPlayerLegs::WrongLand(float elapsed){

	canThrow = false;

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_wrong_landing");
	TCompSkeleton* skeleton = comp_skeleton;
	TCompTransform* camera_transform = ((CEntity*)entity_camera)->get<TCompTransform>();

	if (on_enter) {
		skeleton->loopAnimation(7);
	}

	if (state_time > 0.2f) {
		physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_transform->getFront());
		dir.normalize();
		((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);
	}
	else {
		EvaluateMovement(true, elapsed);
	}

	if (state_time >= 0.5){
		skeleton->stopAnimation(7);
		ChangeState("fbp_Idle");
	}
}

void FSMPlayerLegs::ProcessHit(float elapsed){}

void FSMPlayerLegs::Hurt(float elapsed){

	canThrow = false;

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

	canThrow = false;

	//((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("prota_ragdoll");

	TCompCharacterController* character_controller = (TCompCharacterController*)comp_character_controller;
	TCompRigidBody* rigidbody = (TCompRigidBody*)comp_rigidbody;
	TCompColliderCapsule* collider = (TCompColliderCapsule*)comp_collider;
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompRagdoll* m_ragdoll = comp_ragdoll;
	TCompSkeleton* m_skeleton = comp_skeleton;

	if (on_enter) {
		if (m_ragdoll) { m_ragdoll->setActive(true); }

		collider->setMaterialProperties(1, 0.7f, 0.7f);

		/*rigidbody->setLockXRot(false);
		rigidbody->setLockYRot(false);
		rigidbody->setLockZRot(false);

		rigidbody->auto_rotate_transform = true;
		rigidbody->auto_translate_transform = true;*/
	}
	if (((state_time >= 4 && rigidbody->rigidBody->getLinearVelocity().magnitude() < 0.1f))
		|| (state_time >= 5))
	{
		if (m_ragdoll) { m_ragdoll->setActive(false); }

		/*rigidbody->setLockXRot(true);
		rigidbody->setLockYRot(true);
		rigidbody->setLockZRot(true);

		rigidbody->auto_rotate_transform = false;
		rigidbody->auto_translate_transform = false;*/

		// Volver a colocar al PJ. TODO: Mejorarlo para que no se quede atascado
		/*rigidbody->rigidBody->setGlobalPose(
			physx::PxTransform(
				rigidbody->rigidBody->getGlobalPose().p + physx::PxVec3(0, 1, 0),
				rigidbody->rigidBody->getGlobalPose().q
			)
		);*/

		collider->setMaterialProperties(0, 0, 0);

		if (((TCompLife*)life)->life <= 0){
			if (m_ragdoll) { m_ragdoll->setActive(false); }
			m_transform->rotation = XMQuaternionIdentity();
			ChangeState("fbp_Dead");
		}
		else{
			if (m_ragdoll) { m_ragdoll->setActive(false); }
			TCompSkeleton* m_skeleton = comp_skeleton;
			m_skeleton->playAnimation(18);
			ChangeState("fbp_WakeUp");
		}
	}

	else
	{
		// Tiempo normal de ragdoll
		// Recolocar la cápsula donde el ragdoll, para que la cámara lo siga
		if (m_ragdoll) {
			// Bone 003: Spine
			XMVECTOR spine_pos = m_skeleton->getPositionOfBone(3);

			XMVECTOR pos_orig = Physics.PxVec3ToXMVECTOR(rigidbody->rigidBody->getGlobalPose().p);
			XMVECTOR pos_final = XMVectorLerp(pos_orig, spine_pos, 0.1f);
			
			/*((TCompCharacterController*)comp_character_controller)->Move(
				Physics.XMVECTORToPxVec3(pos_final - pos_orig)
				, false, false, Physics.XMVECTORToPxVec3(((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront()), elapsed);*/
			rigidbody->rigidBody->setGlobalPose(
				physx::PxTransform(
					Physics.XMVECTORToPxVec3(pos_final),
					rigidbody->rigidBody->getGlobalPose().q
					)
				);
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
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompSkeleton* m_skeleton = comp_skeleton;

	if (on_enter) {
		XMVECTOR head_pos = m_skeleton->getPositionOfBone(41);
		XMVectorSetY(head_pos, XMVectorGetY(m_transform->position));
		XMVECTOR head_dir = XMVector3Normalize(head_pos - m_transform->position);
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);

		XMVECTOR new_front = XMVector3Cross(up, head_dir);
		m_transform->lookAt(m_transform->position - new_front, up);
	}
	

	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(m_transform->getFront());
	dir.normalize();
	((TCompCharacterController*)comp_character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, dir, elapsed);

	if (state_time >= 3.3f){
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
	else if (isKeyPressed('S')){
		movement_vector += physx::PxVec3(0, 0, -1);
		is_moving = true;
	}
	if (isKeyPressed('A')){
		movement_vector += physx::PxVec3(1, 0, 0);
		is_moving = true;
	}
	else if (isKeyPressed('D')){
		movement_vector += physx::PxVec3(-1, 0, 0);
		is_moving = true;
	}
	if (isKeyPressed(' ')){
		jump = true;
	}

	// Movement correction
	movement_dir = movement_vector;
	movement_vector = rotation.rotate(movement_vector);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);

	XMVECTOR projected_front = XMVector3Cross(up, XMVector3Cross(camera_transform->getFront(), up));
	XMVECTOR projected_left = XMVector3Cross(up, XMVector3Cross(camera_transform->getLeft(), up));

	XMVECTOR camera_dir = camera_transform->getFront();

	// Going front
	if (movement_dir.z > 0) {
		camera_dir =
			camera_transform->getFront() * movement_dir.z +
			camera_transform->getLeft() * movement_dir.x;
				
		movement_vector = PxQuat(deg2rad(-45 * movement_dir.x), PxVec3(0, 1, 0)).rotate(movement_vector);
	}

	// Going back
	if (movement_dir.z < 0) {
		camera_dir =
			-camera_transform->getFront() * movement_dir.z +
			-camera_transform->getLeft() * movement_dir.x;

		movement_vector = PxQuat(deg2rad(45 * movement_dir.x), PxVec3(0, 1, 0)).rotate(movement_vector);
	}

	// Look at the front of the camera
	//physx::PxVec3 dir = Physics.XMVECTORToPxVec3(projected_front);
	physx::PxVec3 dir = Physics.XMVECTORToPxVec3(camera_dir);

	// Look at the front of the transform
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
