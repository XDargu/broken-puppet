#ifndef INC_COMP_CHARACTER_CONTROLLER_H_
#define INC_COMP_CHARACTER_CONTROLLER_H_

#include "base_component.h"
#include "physics_manager.h"

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
	double lastAirTime;						// USed for checking when the character was last in the air for controlling jumps
	CHandle capsule_collider;				// The collider for the character
	physx::PxVec3 moveInput;
	bool crouchInput;
	bool jumpInput;
	float max_vel_y;						// constante de velocidad maxima en y para muerte por caida
	physx::PxVec3 velocity;
	CHandle m_entity;
	PxVec3 ground_velocity;

	// PRUEBAS
	PxVec3 last_platform_speed;
	

public:
	float crouchHeightFactor;					// collider height is multiplied by this when crouching
	float crouchChangeSpeed;					// speed at which capsule changes height when crouching/standing
	float jumpRepeatDelayTime;					// amount of time that must elapse between landing and being able to jump again

	float moveSpeedMultiplier;				// how much the move speed of the character will be multiplied by
	float lerpRotation;
	float jumpPower;						// determines the jump force applied when jumping (and therefore the jump height)

	XMVECTOR high_friction_material;		// High friction material properties
	XMVECTOR zero_friction_material;		// Zero friction material properties

	TCompCharacterController();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void fixedUpdate(float elapsed);

	void Move(physx::PxVec3 move, bool crouch, bool jump, physx::PxVec3 lookPos);

	void ConvertMoveInput();

	void TurnTowardsCameraForward();

	void GroundCheck(float elapsed);

	void SetFriction();

	void HandleGroundedVelocities();

	void HandleAirborneVelocities();

	physx::PxVec3 Vector3Lerp(physx::PxVec3 start, physx::PxVec3 end, float percent);

	bool OnGround();

	bool IsJumping();

	physx::PxReal getMass();

	void Move(physx::PxVec3 move, bool crouch, bool jump, physx::PxVec3 lookPos, float elapsed);

	CHandle getRigidBody();
};

#endif