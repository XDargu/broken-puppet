#include "mcv_platform.h"
#include "comp_character_controller.h"
#include "comp_transform.h"
#include "comp_rigid_body.h"
#include "comp_collider_capsule.h"
#include "comp_name.h"

TCompCharacterController::TCompCharacterController() {}

void TCompCharacterController::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	// Required components
	transform = assertRequiredComponent<TCompTransform>(this);
	rigidbody = assertRequiredComponent<TCompRigidBody>(this);
	collider = assertRequiredComponent<TCompColliderCapsule>(this);
	TCompName* name = assertRequiredComponent<TCompName>(this);

	TCompTransform* trans = (TCompTransform*)transform;
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;
	TCompColliderCapsule* coll = (TCompColliderCapsule*)collider;

	// Death velocity
	max_vel_y = -10.f;

	physx::PxTransform relativePose(physx::PxVec3(0, (coll->getRadius() + coll->getHalfHeight()), 0), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	coll->collider->setLocalPose(relativePose);

	// Set up the rigidbody
	rigid->rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	rigid->rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
	rigid->rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
	rigid->rigidBody->setName(name->name);

	//float threshold = 3500.f;
	physx::PxReal threshold = 1000.f;
	rigid->rigidBody->setContactReportThreshold(threshold);

	// Lock rotation
	rigid->setLockXRot(true);
	rigid->setLockYRot(true);
	rigid->setLockZRot(true);

	// Update the transform manually
	rigid->auto_translate_transform = false;
	rigid->auto_rotate_transform = false;

	// Material properties
	high_friction_material = XMVectorSet(1, 1, 1, 0);
	zero_friction_material = XMVectorSet(0, 0, 0, 0);

	// Initialize variables
	crouchHeightFactor = 0.6f;
	crouchChangeSpeed = 4;
	jumpRepeatDelayTime = 0.25f;
	max_vel_y = -10.0f;

	jumpPower = 8;
	airSpeed = 6;
	airControl = 10;
	gravityMultiplier = 4;
	moveSpeedMultiplier = 8;
	animSpeedMultiplier = 1;
	lerpRotation = 0.35f;

	originalHeight = (coll->getRadius() + coll->getHalfHeight()) * 2;
	onAir = false;

	// PRUEBAS
	last_platform_speed = PxVec3(0, 0, 0);

	// Give the look position a default in case the character is not under control
	currentLookPos = Physics.XMVECTORToPxVec3(trans->position);
}

void TCompCharacterController::init(){

}

void TCompCharacterController::update(float elapsed){
}

void TCompCharacterController::fixedUpdate(float elapsed) {
	TCompTransform* trans = (TCompTransform*)transform;
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

	// remember when we were last in air, for jump delay
	if (!onGround) lastAirTime = CApp::get().total_time;

	// Get the rotation
	physx::PxVec3 rot = physx::PxVec3(0, -0.1f, 0);
	rot = rigid->rigidBody->getGlobalPose().q.rotate(rot);

	// Move the transform position
	trans->position = Physics.PxVec3ToXMVECTOR(rigid->rigidBody->getGlobalPose().p + rot);
	//((TCompTransform*)transform)->rotation = Physics.PxQuatToXMVECTOR(enemy_rigidbody->getGlobalPose().q);

	if (!onGround) ground_velocity = PxVec3(0, 0, 0);
}

void TCompCharacterController::Move(physx::PxVec3 move, bool crouch, bool jump, physx::PxVec3 lookPos)
{
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

	if (move.magnitude() > 1) move.normalize();

	// transfer input parameters to member variables.
	moveInput = move;
	crouchInput = crouch;
	jumpInput = jump;
	currentLookPos = lookPos;

	// grab current velocity, we will be changing it.
	velocity = rigid->rigidBody->getLinearVelocity();
	physx::PxVec3 prev_velocity = rigid->rigidBody->getLinearVelocity();

	ConvertMoveInput(); // converts the relative move vector into local turn & fwd values

	TurnTowardsCameraForward(); // makes the character face the way the camera is looking

	GroundCheck(0); // detect and stick to ground

	SetFriction(); // use low or high friction values depending on the current states

	// control and velocity handling is different when grounded and airborne:
	if (onGround)
	{
		HandleGroundedVelocities();
	}
	else
	{
		HandleAirborneVelocities();
	}

	// reassign velocity, since it will have been modified by the above functions.		
	rigid->rigidBody->setLinearVelocity(velocity);
}



void TCompCharacterController::ConvertMoveInput()
{
	// convert the world relative moveInput vector into a local-relative
	// turn amount and forward amount required to head in the desired
	// direction.

	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

	// TODO Ojo, no está seguro que funcione bien
	physx::PxTransform px_trans = rigid->rigidBody->getGlobalPose();
	physx::PxVec3 localMove = px_trans.transformInv(moveInput);


}
void TCompCharacterController::TurnTowardsCameraForward()
{
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;
	TCompTransform* trans = (TCompTransform*)transform;

	PxTransform tt = Physics.transformToPxTransform(*trans);
	tt.q = PxQuat(0, 0, 0, 1);
	PxVec3 tLookAt = tt.q.rotate(currentLookPos) + tt.p;
	tLookAt.y = XMVectorGetY(trans->position);

	/*physx::PxVec3 lookAt = rigid->rigidBody->getGlobalPose().q.rotate(currentLookPos) + rigid->rigidBody->getGlobalPose().p;
	lookAt.y = XMVectorGetY(trans->position);//rigid->rigidBody->getGlobalPose().p.y - 0.1f;*/

	trans->aimAt(Physics.PxVec3ToXMVECTOR(tLookAt), XMVectorSet(0, 1, 0, 0), lerpRotation);

	//rigid->rigidBody->setGlobalPose(physx::PxTransform(rigid->rigidBody->getGlobalPose().p, Physics.XMVECTORToPxQuat(((TCompTransform*)transform)->rotation)));
}

void TCompCharacterController::GroundCheck(float elapsed)
{
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

	PxTransform &px_trans = rigid->rigidBody->getGlobalPose();

	PxRaycastBuffer buf;

	Physics.raycastAll(px_trans.p + physx::PxVec3(0, 1, 0) *.1f, -physx::PxVec3(0, 1, 0), 0.25f, buf);

	if (velocity.y < jumpPower*.5f)
	{
		onGround = false;

		// comprobamos si el el objeto es el más cercano, si lo es, obtenemos su movimiento

		for (int i = 0; i < (int)buf.nbTouches; i++)
		{
			if (buf.touches[i].actor != rigid->rigidBody) {
				// check whether we hit a non-trigger collider (and not the character itself)

				// this counts as being on ground.

				// stick to surface - helps character stick to ground - specially when running down slopes
				if (velocity.y <= 0)
				{

					if (elapsed == 0)
						int i = 0;

					// Colocamos en el ground a pelo
					PxTransform px_trans = rigid->rigidBody->getGlobalPose();
					px_trans.p = buf.touches[i].position + physx::PxVec3(0, 0.1f, 0);
					rigid->rigidBody->setGlobalPose(px_trans);

					PxActor* ground_actor = buf.touches[i].actor;
					// If is a rigidbody
					if (ground_actor->isRigidBody())
					{

						// Check if is moving
						ground_velocity = ((PxRigidBody*)ground_actor)->getLinearVelocity();
						ground_velocity = PxVec3(ground_velocity.x, 0, ground_velocity.z);

						// Pruebas							
						// restar última velocidad y añadir la nueva

						PxVec3 diff = last_platform_speed - ground_velocity;
						rigid->rigidBody->addForce(ground_velocity - diff, PxForceMode::eVELOCITY_CHANGE, true);
						last_platform_speed = ground_velocity;

						// Aply weight force
						/*if (!((PxRigidBody*)ground_actor)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
							PxVec3 down_force = Physics.gScene->getGravity() * rigid->rigidBody->getMass();
							((PxRigidBody*)ground_actor)->addForce(down_force, PxForceMode::eFORCE, true);
						}*/
					}
					else {
						ground_velocity = PxVec3(0, 0, 0);
					}



				}

				// Comprobar velocidad aculumada en caida
				if (velocity.y < max_vel_y){
					//TODO: usar el metodo para eliminar enemigo
					CEntity* e = CHandle(this).getOwner();
					e->sendMsg(TGroundHit(e, velocity.y));
				}

				onGround = true;
				//rigidbody.useGravity = false;
				break;
			}
		}

	}

	// remember when we were last in air, for jump delay
	if (!onGround) lastAirTime = CApp::get().total_time;
}

void TCompCharacterController::SetFriction()
{
	TCompColliderCapsule* coll = (TCompColliderCapsule*)collider;

	if (onGround)
	{

		// set friction to low or high, depending on if we're moving
		if (moveInput.magnitude() == 0)
		{
			// when not moving this helps prevent sliding on slopes:
			coll->setMaterialProperties(high_friction_material);
		}
		else
		{
			// but when moving, we want no friction:
			coll->setMaterialProperties(zero_friction_material);
		}
	}
	else
	{
		// while in air, we want no friction against surfaces (walls, ceilings, etc)
		coll->setMaterialProperties(zero_friction_material);
	}
}

void TCompCharacterController::HandleGroundedVelocities()
{
	TCompRigidBody* rigid = rigidbody;

	float water_level = CApp::get().water_level;
	float atten = 0.2f;
	float water_multiplier = 1;

	if (rigid->rigidBody->getGlobalPose().p.y < water_level - atten)  {	

		float proportion = min(1, (water_level - rigid->rigidBody->getGlobalPose().p.y) / atten);
		water_multiplier = 1 - (proportion * 0.5f);
	}
	

	onAir = false;
	physx::PxVec3 groundMove = physx::PxVec3(moveInput.x, velocity.y, moveInput.z) * moveSpeedMultiplier * water_multiplier;
	velocity = groundMove;

	velocity.y = 0;

	if (moveInput.magnitude() == 0)
	{
		// when not moving this prevents sliding on slopes:
		velocity.x = 0;
		velocity.z = 0;
	}
	// check whether conditions are right to allow a jump:
	// TODO aún no tenemos animaciones
	//bool animationGrounded = animator.GetCurrentAnimatorStateInfo(0).IsName("Grounded");
	bool animationGrounded = true;
	bool okToRepeatJump = CApp::get().total_time > lastAirTime + jumpRepeatDelayTime;

	if (jumpInput && !crouchInput && okToRepeatJump && animationGrounded)
	{
		// jump!
		onGround = false;
		velocity = moveInput*airSpeed;
		velocity.y = jumpPower;
		TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;
		rigid->rigidBody->addForce(ground_velocity, PxForceMode::eVELOCITY_CHANGE, true);
	}
}

void TCompCharacterController::HandleAirborneVelocities()
{
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

	onAir = true;

	// we allow some movement in air, but it's very different to when on ground
	// (typically allowing a small change in trajectory)
	physx::PxVec3 airMove = physx::PxVec3(moveInput.x*airSpeed, velocity.y, moveInput.z*airSpeed);
	velocity = Vector3Lerp(velocity, airMove, CApp::get().delta_time*airControl);

	// NOTES siempre utilizamos la gravedad
	//enemy_rigidbody->useGravity = true;

	// apply extra gravity from multiplier:		
	physx::PxVec3 extraGravityForce = (Physics.gScene->getGravity()*gravityMultiplier) - Physics.gScene->getGravity();
	rigid->rigidBody->addForce(extraGravityForce);

}

//TODO colocar en utils funcion lerp
physx::PxVec3 TCompCharacterController::Vector3Lerp(physx::PxVec3 start, physx::PxVec3 end, float percent)
{
	return (start + percent * (end - start));
}
// TODO NOTE provisional para rotar objetos

bool TCompCharacterController::OnGround(){
	return onGround;
}

bool TCompCharacterController::IsJumping(){
	return (onAir && (velocity.y > 0) && (!onGround));
}

physx::PxReal TCompCharacterController::getMass(){
	return ((TCompRigidBody*)rigidbody)->rigidBody->getMass();
}




void TCompCharacterController::Move(physx::PxVec3 move, bool crouch, bool jump, physx::PxVec3 lookPos, float elapsed)
{
	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

	if (move.magnitude() > 1) move.normalize();

	// transfer input parameters to member variables.
	moveInput = move;
	crouchInput = crouch;
	jumpInput = jump;
	currentLookPos = lookPos;

	// grab current velocity, we will be changing it.
	velocity = rigid->rigidBody->getLinearVelocity();
	physx::PxVec3 prev_velocity = rigid->rigidBody->getLinearVelocity();

	ConvertMoveInput(); // converts the relative move vector into local turn & fwd values

	TurnTowardsCameraForward(); // makes the character face the way the camera is looking

	GroundCheck(elapsed); // detect and stick to ground

	SetFriction(); // use low or high friction values depending on the current states

	// control and velocity handling is different when grounded and airborne:
	if (onGround)
	{
		HandleGroundedVelocities();
	}
	else
	{
		HandleAirborneVelocities();
	}

	// reassign velocity, since it will have been modified by the above functions.		
	rigid->rigidBody->setLinearVelocity(velocity);
}

CHandle TCompCharacterController::getRigidBody(){
	return 	(TCompRigidBody*)rigidbody;
}

