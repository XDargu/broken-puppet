#ifndef INC_COMP_UNITY_CHARACTER_CONTROLLER_H_
#define INC_COMP_UNITY_CHARACTER_CONTROLLER_H_

#include "base_component.h"

class AdvancedSettings
{
public:
	float stationaryTurnSpeed = 180; // additional turn speed added when the player is stationary (added to animation root rotation)
	float movingTurnSpeed = 360; // additional turn speed added when the player is moving (added to animation root rotation)
	float headLookResponseSpeed = 2; // speed at which head look follows its target
	float crouchHeightFactor = 0.6f; // collider height is multiplied by this when crouching
	float crouchChangeSpeed = 4; // speed at which capsule changes height when crouching/standing
	float autoTurnThresholdAngle = 100; // character auto turns towards camera direction if facing away by more than this angle
	float autoTurnSpeed = 2; // speed at which character auto-turns towards cam direction
	//PhysicMaterial zeroFrictionMaterial; // used when in motion to enable smooth movement
	//PhysicMaterial highFrictionMaterial; // used when stationary to avoid sliding down slopes
	float jumpRepeatDelayTime = 0.25f; // amount of time that must elapse between landing and being able to jump again
	float runCycleLegOffset = 0.2f; // animation cycle offset (0-1) used for determining correct leg to jump off
	float groundStickyEffect = 5.f; // power of 'stick to ground' effect - prevents bumping down slopes.
};

struct TCompUnityCharacterController : TBaseComponent {    // 2 ...

private:

	/***********************************
			Serializable settings
	***********************************/ 

	float jumpPower = 12; // determines the jump force applied when jumping (and therefore the jump height)
	float airSpeed = 6; // determines the max speed of the character while airborne
	float airControl = 2; // determines the response speed of controlling the character while airborne
	float gravityMultiplier = 2; // gravity modifier - often higher than natural gravity feels right for game characters
	float moveSpeedMultiplier = 1; // how much the move speed of the character will be multiplied by
	float animSpeedMultiplier = 1; // how much the animation of the character will be multiplied by
	AdvancedSettings advancedSettings; // Container for the advanced settings class , thiss allows the advanced settings to be in a foldout in the inspector

	/***********************************
			Other settings
	***********************************/

	bool onGround; // Is the character on the ground
	physx::PxVec3 currentLookPos; // The current position where the character is looking
	float originalHeight; // Used for tracking the original height of the characters capsule collider
	//Animator animator; // The animator for the character
	float lastAirTime; // USed for checking when the character was last in the air for controlling jumps
	CHandle capsule_collider; // The collider for the character
	const float half = 0.5f; // whats it says, it's a constant for a half
	physx::PxVec3 moveInput;
	bool crouchInput;
	bool jumpInput;
	float turnAmount;
	float forwardAmount;
	physx::PxVec3 velocity;
	//IComparer rayHitComparer;

	CHandle m_entity = CHandle(this).getOwner();

public:

	CHandle lookTarget; // The point where the character will be looking at
	 //LayerMask groundCheckMask;
	 //LayerMask crouchCheckMask;
	 float lookBlendTime;
	 float lookWeight;
	 CHandle lookTarget; // The point where the character will be looking at
	 //LayerMask groundCheckMask;
	 //LayerMask crouchCheckMask;
	 float lookBlendTime;
	 float lookWeight;

	void loadFromAtts(MKeyValue &atts) {
	}

	void init(){
		//animator = GetComponentInChildren<Animator>();
		capsule = m_entity.getType<TcompC>;

		// as can return null so we need to make sure thats its not before assigning to it
		if (capsule == null)
		{
			Debug.LogError(" collider cannot be cast to CapsuleCollider");
		}
		else
		{
			originalHeight = capsule.height;
			capsule.center = Vector3.up*originalHeight*half;
		}

		rayHitComparer = new RayHitComparer();

		SetUpAnimator();

		// give the look position a default in case the character is not under control
		currentLookPos = Camera.main.transform.position;
	}
	void update(float elapsed){
	}
	void fixedUpdate(float elapsed){
	}

};

#endif
