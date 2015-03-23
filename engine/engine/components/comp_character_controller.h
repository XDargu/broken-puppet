#ifndef INC_COMP_CHARACTER_CONTROLLER_H_
#define INC_COMP_CHARACTER_CONTROLLER_H_

#include "base_component.h"

struct TCompCharacterController : TBaseComponent {

private:

	CHandle transform;						// Reference to the transform
	CHandle rigidbody;						// Reference to the rigidbody
	CHandle collider;						// Reference to the collider

	float airSpeed;							// determines the max speed of the character while airborne
	float airControl;						// determines the response speed of controlling the character while airborne;
	float gravityMultiplier;				// gravity modifier - often higher than natural gravity feels right for game characters

	float animSpeedMultiplier;				// how much the animation of the character will be multiplied by

	bool onAir;
	bool onGround;							// Is the character on the ground
	physx::PxVec3 currentLookPos;			// The current position where the character is looking
	float originalHeight;					// Used for tracking the original height of the characters capsule collider
	float lastAirTime;						// USed for checking when the character was last in the air for controlling jumps
	CHandle capsule_collider;				// The collider for the character
	physx::PxVec3 moveInput;
	bool crouchInput;
	bool jumpInput;
	float max_vel_y;						// constante de velocidad maxima en y para muerte por caida
	physx::PxVec3 velocity;
	CHandle m_entity;

public:
	float crouchHeightFactor;					// collider height is multiplied by this when crouching
	float crouchChangeSpeed;					// speed at which capsule changes height when crouching/standing
	float jumpRepeatDelayTime;					// amount of time that must elapse between landing and being able to jump again

	float moveSpeedMultiplier;				// how much the move speed of the character will be multiplied by
	float lerpRotation;
	float jumpPower;						// determines the jump force applied when jumping (and therefore the jump height)

	XMVECTOR high_friction_material;		// High friction material properties
	XMVECTOR zero_friction_material;		// Zero friction material properties

	TCompCharacterController() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

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
		
		// Set up the collider
		setupFiltering(coll->collider, FilterGroup::eENEMY, FilterGroup::eENEMY);

		physx::PxTransform relativePose(physx::PxVec3(0, (coll->getRadius() + coll->getHalfHeight()), 0), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
		coll->collider->setLocalPose(relativePose);

		// Set up the rigidbody
		rigid->rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
		rigid->rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		rigid->rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
		rigid->rigidBody->setName(name->name);		

		setupFiltering(rigid->rigidBody, FilterGroup::eENEMY, FilterGroup::eENEMY);
		float threshold = 3500.f;
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

		// Give the look position a default in case the character is not under control
		currentLookPos = Physics.XMVECTORToPxVec3(trans->position);
	}

	void init(){
		
	}

	void update(float elapsed){
	}

	void fixedUpdate(float elapsed) {
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
	}

	void Move(physx::PxVec3 move, bool crouch, bool jump, physx::PxVec3 lookPos)
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

		GroundCheck(); // detect and stick to ground

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

	void ConvertMoveInput()
	{
		// convert the world relative moveInput vector into a local-relative
		// turn amount and forward amount required to head in the desired
		// direction.

		TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

		// TODO Ojo, no está seguro que funcione bien
		physx::PxTransform px_trans = rigid->rigidBody->getGlobalPose();
		physx::PxVec3 localMove = px_trans.transformInv(moveInput);


	}
	void TurnTowardsCameraForward()
	{
		TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

		physx::PxVec3 lookAt = rigid->rigidBody->getGlobalPose().q.rotate(currentLookPos) + rigid->rigidBody->getGlobalPose().p;
		lookAt.y = rigid->rigidBody->getGlobalPose().p.y - 0.1f;

		((TCompTransform*)transform)->aimAt(Physics.PxVec3ToXMVECTOR(lookAt), XMVectorSet(0, 1, 0, 0), lerpRotation);

		//rigid->rigidBody->setGlobalPose(physx::PxTransform(rigid->rigidBody->getGlobalPose().p, Physics.XMVECTORToPxQuat(((TCompTransform*)transform)->rotation)));
	}

	void GroundCheck()
	{
		TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;

		physx::PxTransform &px_trans = rigid->rigidBody->getGlobalPose();

		physx::PxRaycastBuffer buf;

		Physics.raycastAll(px_trans.p + physx::PxVec3(0, 1, 0) *.1f, -physx::PxVec3(0, 1, 0), 0.25f, buf);

		if (velocity.y < jumpPower*.5f)
		{
			onGround = false;
			// Dejo el rigidbody usando la gravity todo el tiempo
			//rigidbody.useGravity = true;				


			for (int i = 0; i < buf.nbTouches; i++)
			{
				if (buf.touches[i].actor != rigid->rigidBody) {
					// check whether we hit a non-trigger collider (and not the character itself)

					// this counts as being on ground.

					// stick to surface - helps character stick to ground - specially when running down slopes
					if (velocity.y <= 0)
					{
						//TODO hacer el movimiento fluido;

						// Colocamos en el ground a pelo
						physx::PxTransform px_trans = rigid->rigidBody->getGlobalPose();
						px_trans.p = buf.touches[i].position + physx::PxVec3(0, 0.1f, 0);
						rigid->rigidBody->setGlobalPose(px_trans);
					}

					// Comprobar velocidad aculumada en caida
					if (velocity.y < max_vel_y){
						//TO DO: usar el metodo para eliminar enemigo
						CEntity* e = CHandle(this).getOwner();
						e->sendMsg(TGroundHit(e, velocity.y));
					}

					onGround = true;
					//rigidbody.useGravity = false;
					break;
				}
			}

		}

		// Sweeptest para saber si, aunque el raycast no encuentre nada, estamos sobre algo
		/*if (velocity.y < 0)
		{
		physx::PxSweepBuffer hit;              // [out] Sweep results
		physx::PxSphereGeometry sweepShape = physx::PxSphereGeometry(enemy_width / 2);    // [in] swept shape
		physx::PxTransform initialPose = physx::PxTransform(px_trans.p, px_trans.q);  // [in] initial shape pose (at distance=0)
		physx::PxVec3 sweepDirection = physx::PxVec3(0, -1, 0);    // [in] normalized sweep direction

		const physx::PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
		physx::PxSweepHit hitBuffer[bufferSize];  // [out] User provided buffer for results
		physx::PxSweepBuffer s_buf(hitBuffer, bufferSize); // [out] Blocking and touching hits will be stored here

		Physics.gScene->sweep(sweepShape, initialPose, sweepDirection, (enemy_height / 2.f) + 0.5f - enemy_width, s_buf);

		for (int i = 0; i < s_buf.nbTouches; i++)
		{
		if (s_buf.touches[i].actor != enemy_rigidbody) {
		onGround = true;
		//rigidbody.useGravity = false;

		// Colocamos en el ground a pelo
		physx::PxTransform px_trans = enemy_rigidbody->getGlobalPose();
		px_trans.p.y = buf.touches[i].position.y + ((enemy_height / 2.f) + 0.1f);
		enemy_rigidbody->setGlobalPose(px_trans);
		break;
		}
		}
		}*/

		// remember when we were last in air, for jump delay
		if (!onGround) lastAirTime = CApp::get().total_time;
	}

	void SetFriction()
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

	void HandleGroundedVelocities()
	{
		onAir = false;
		physx::PxVec3 groundMove = physx::PxVec3(moveInput.x, velocity.y, moveInput.z) * moveSpeedMultiplier;
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
		}
	}

	void HandleAirborneVelocities()
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
	physx::PxVec3 Vector3Lerp(physx::PxVec3 start, physx::PxVec3 end, float percent)
	{
		return (start + percent * (end - start));
	}
	// TODO NOTE provisional para rotar objetos

	bool OnGround(){
		return onGround;
	}

	bool IsJumping(){
		return (onAir && (velocity.y > 0) && (!onGround));
	}

	physx::PxReal getMass(){
		return ((TCompRigidBody*)rigidbody)->rigidBody->getMass();
	}
};

#endif