#ifndef INC_COMP_CHARACTER_CONTROLLER_H_
#define INC_COMP_CHARACTER_CONTROLLER_H_

#include "base_component.h"

struct TCompUnityCharacterController : TBaseComponent {

private:

	CHandle transform;						// Reference to the transform

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
	physx::PxMaterial* zeroFrictionMaterial;	// used when in motion to enable smooth movement
	physx::PxMaterial* highFrictionMaterial;	// used when stationary to avoid sliding down slopes
	float jumpRepeatDelayTime;					// amount of time that must elapse between landing and being able to jump again

	float enemy_width;
	float enemy_height;
	float enemy_density;
	physx::PxShape* enemy_collider;
	physx::PxRigidDynamic* enemy_rigidbody;
	physx::PxD6Joint* mJoint;				// Joint used to lock the player
	float moveSpeedMultiplier;				// how much the move speed of the character will be multiplied by
	float lerpRotation;
	float jumpPower;						// determines the jump force applied when jumping (and therefore the jump height)

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		transform = assertRequiredComponent<TCompTransform>(this);
		TCompTransform* trans = (TCompTransform*)transform;

		enemy_width = 0.5;
		enemy_height = 1.5f;
		enemy_density = 25;
		max_vel_y = -10.f;

		// Create player material
		physx::PxMaterial* pMaterial = Physics.gPhysicsSDK->createMaterial(0, 0, 0);
		pMaterial->setFrictionCombineMode(physx::PxCombineMode::eMULTIPLY);
		pMaterial->setRestitutionCombineMode(physx::PxCombineMode::eMULTIPLY);

		// Create player capsule collider
		enemy_collider = Physics.gPhysicsSDK->createShape(
			physx::PxCapsuleGeometry(enemy_width / 2, enemy_height / 2 - enemy_width / 2),
			&pMaterial
			,
			true);

		setupFiltering(enemy_collider, FilterGroup::eENEMY, FilterGroup::eENEMY);
		physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
		enemy_collider->setLocalPose(relativePose);

		// Create player rigidbody
		const physx::PxTransform &mPxTrans = physx::PxTransform(
			Physics.XMVECTORToPxVec3(trans->position),
			Physics.XMVECTORToPxQuat(trans->rotation/*XMQuaternionMultiply(trans->rotation, XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), deg2rad(90))))*/));

		enemy_rigidbody = physx::PxCreateDynamic(
			*Physics.gPhysicsSDK,
			mPxTrans,
			*enemy_collider,
			enemy_density);

		enemy_rigidbody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
		enemy_rigidbody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		enemy_rigidbody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
		TCompName* name = assertRequiredComponent<TCompName>(this);
		enemy_rigidbody->setName(name->name);

		setupFiltering(enemy_collider, FilterGroup::eENEMY, FilterGroup::eENEMY);
		physx::PxReal threshold = 3500.f;
		enemy_rigidbody->setContactReportThreshold(threshold);


		// Set it as a CCD (Continiuous Collision Detection)
		//player_rigidbody->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

		// Freeze rotation in axis x, z

		mJoint = PxD6JointCreate(*Physics.gPhysicsSDK, enemy_rigidbody, physx::PxTransform::createIdentity(), NULL, enemy_rigidbody->getGlobalPose());
		mJoint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);
		mJoint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eFREE);
		mJoint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eFREE);
		Physics.gScene->addActor(*enemy_rigidbody);


	}

	void init(){
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


		highFrictionMaterial = Physics.gPhysicsSDK->createMaterial(1.f, 1.f, 0.f);
		zeroFrictionMaterial = Physics.gPhysicsSDK->createMaterial(0.f, 0.f, 0.f);

		zeroFrictionMaterial->setFrictionCombineMode(physx::PxCombineMode::eMULTIPLY);
		zeroFrictionMaterial->setRestitutionCombineMode(physx::PxCombineMode::eMULTIPLY);

		highFrictionMaterial->setFrictionCombineMode(physx::PxCombineMode::eMAX);
		highFrictionMaterial->setRestitutionCombineMode(physx::PxCombineMode::eMIN);

		originalHeight = enemy_height;

		onAir = false;

		physx::PxRaycastBuffer;

		// give the look position a default in case the character is not under control
		currentLookPos = Physics.XMVECTORToPxVec3(((TCompTransform*)transform)->position);
	}

	void update(float elapsed){


	}

	void fixedUpdate(float elapsed) {

		// remember when we were last in air, for jump delay
		if (!onGround) lastAirTime = CApp::get().total_time;

		//((TCompTransform*)transform)->position = Physics.PxVec3ToXMVECTOR(enemy_rigidbody->getGlobalPose().p + physx::PxVec3(0, -(enemy_height / 2.0f + 0.1f), 0));
		physx::PxVec3 rot = physx::PxVec3(0, -(enemy_height / 2.0f + 0.1f), 0);
		rot = enemy_rigidbody->getGlobalPose().q.rotate(rot);
		((TCompTransform*)transform)->position = Physics.PxVec3ToXMVECTOR(enemy_rigidbody->getGlobalPose().p + rot);
		//((TCompTransform*)transform)->rotation = Physics.PxQuatToXMVECTOR(enemy_rigidbody->getGlobalPose().q);
	}

	void Move(physx::PxVec3 move, bool crouch, bool jump, physx::PxVec3 lookPos)
	{
		if (move.magnitude() > 1) move.normalize();

		// transfer input parameters to member variables.
		moveInput = move;
		crouchInput = crouch;
		jumpInput = jump;
		currentLookPos = lookPos;

		// grab current velocity, we will be changing it.
		velocity = enemy_rigidbody->getLinearVelocity();
		physx::PxVec3 prev_velocity = enemy_rigidbody->getLinearVelocity();

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
		enemy_rigidbody->setLinearVelocity(velocity);
	}

	void ConvertMoveInput()
	{
		// convert the world relative moveInput vector into a local-relative
		// turn amount and forward amount required to head in the desired
		// direction. 		

		// TODO Ojo, no est� seguro que funcione bien
		physx::PxTransform px_trans = enemy_rigidbody->getGlobalPose();
		physx::PxVec3 localMove = px_trans.transformInv(moveInput);


	}
	void TurnTowardsCameraForward()
	{
		physx::PxVec3 lookAt = enemy_rigidbody->getGlobalPose().q.rotate(currentLookPos) + enemy_rigidbody->getGlobalPose().p;
		lookAt.y = enemy_rigidbody->getGlobalPose().p.y - (enemy_height / 2.0f + 0.1f);

		((TCompTransform*)transform)->aimAt(Physics.PxVec3ToXMVECTOR(lookAt), XMVectorSet(0, 1, 0, 0), lerpRotation);

		//enemy_rigidbody->setGlobalPose(physx::PxTransform(enemy_rigidbody->getGlobalPose().p, Physics.XMVECTORToPxQuat(((TCompTransform*)transform)->rotation)));
	}

	void GroundCheck()
	{
		physx::PxTransform &px_trans = enemy_rigidbody->getGlobalPose();

		physx::PxRaycastBuffer buf;

		Physics.raycastAll(px_trans.p + physx::PxVec3(0, 1, 0) *.1f, -physx::PxVec3(0, 1, 0), (enemy_height / 2.f) + 0.25f, buf);

		if (velocity.y < jumpPower*.5f)
		{
			onGround = false;
			// Dejo el rigidbody usando la gravity todo el tiempo
			//rigidbody.useGravity = true;				


			for (int i = 0; i < buf.nbTouches; i++)
			{
				if (buf.touches[i].actor != enemy_rigidbody) {
					// check whether we hit a non-trigger collider (and not the character itself)

					// this counts as being on ground.

					// stick to surface - helps character stick to ground - specially when running down slopes
					if (velocity.y <= 0)
					{
						//TODO hacer el movimiento fluido;

						// Colocamos en el ground a pelo
						physx::PxTransform px_trans = enemy_rigidbody->getGlobalPose();
						px_trans.p = buf.touches[i].position + physx::PxVec3(0, (enemy_height / 2.f) + 0.1f, 0);
						enemy_rigidbody->setGlobalPose(px_trans);
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
		// TODO Hacer el cambio de las propiedades del material, no del material en s� (es mucho m�s r�pido y no da problemas)

		if (onGround)
		{

			// set friction to low or high, depending on if we're moving
			if (moveInput.magnitude() == 0)
			{
				// when not moving this helps prevent sliding on slopes:
				ChangeMaterial(enemy_collider, highFrictionMaterial);
			}
			else
			{
				// but when moving, we want no friction:
				ChangeMaterial(enemy_collider, zeroFrictionMaterial);
			}
		}
		else
		{
			// while in air, we want no friction against surfaces (walls, ceilings, etc)
			ChangeMaterial(enemy_collider, zeroFrictionMaterial);
		}
	}

	void ChangeMaterial(physx::PxShape* collider, physx::PxMaterial* m_material){

		const physx::PxU16 n_materials = collider->getNbMaterials();
		assert(n_materials < 16 || fatal("enemy_collider no puede tener m�s de 16 materiales"));
		physx::PxMaterial* buffer[16];
		physx::PxU16 first_material = 0;
		enemy_collider->getMaterials(buffer, 16);
		buffer[first_material] = m_material;
		//collider->setMaterials(buffer, first_material + 1);
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
		// TODO a�n no tenemos animaciones
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
		onAir = true;

		// we allow some movement in air, but it's very different to when on ground
		// (typically allowing a small change in trajectory)
		physx::PxVec3 airMove = physx::PxVec3(moveInput.x*airSpeed, velocity.y, moveInput.z*airSpeed);
		velocity = Vector3Lerp(velocity, airMove, CApp::get().delta_time*airControl);

		// NOTES siempre utilizamos la gravedad
		//enemy_rigidbody->useGravity = true;

		// apply extra gravity from multiplier:		
		physx::PxVec3 extraGravityForce = (Physics.gScene->getGravity()*gravityMultiplier) - Physics.gScene->getGravity();
		enemy_rigidbody->addForce(extraGravityForce);

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
		return enemy_rigidbody->getMass();
	}
};

#endif