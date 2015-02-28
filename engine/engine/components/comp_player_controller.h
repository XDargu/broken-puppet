#ifndef INC_COMP_PLAYER_CONTROLLER_H_
#define INC_COMP_PLAYER_CONTROLLER_H_

#include "base_component.h"

struct TCompPlayerController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_pivot_transform;
	XMVECTOR delta_pos;
	XMVECTOR delta_jump;

public:

	bool jumping;
	bool grounded;
	bool falling;
	float jumpHeight;
	float jumpSpeed;

	//Variables usadas para limitar el número de tramas de hilo disponibles
	static const unsigned int max_num_string = 8;
	unsigned int current_num_string;

	physx::PxController* character_controller;
	physx::PxRigidDynamic* kinematic_capsule;

	float movement_velocity;
	float rotation_velocity;

	TCompPlayerController()
		: movement_velocity(5.0f)
		, rotation_velocity(deg2rad(90.f))
		, delta_pos(XMVectorZero())
		, jumpHeight(20)
		, jumpSpeed(20)
		, jumping(false)
		, grounded(false)
		, falling(false)
		, delta_jump(XMVectorZero())
		, current_num_string(0)
	{}

	void loadFromAtts(MKeyValue &atts) {
		movement_velocity = atts.getFloat("movementVelocity", 5);
		rotation_velocity = deg2rad(atts.getFloat("rotationVelocity", 90));

		CEntity* e = CHandle(this).getOwner();
		m_transform = e->get<TCompTransform>();
		TCompTransform* trans = (TCompTransform*)m_transform;

		assert(trans || fatal("TPlayerController requieres a TTransform component"));

		// Create player material
		physx::PxMaterial* pMaterial = Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.5f);

		// Character controller creation
		physx::PxCapsuleControllerDesc desc;

		desc.setToDefault();
		desc.material = pMaterial;
		desc.radius = 0.25f;
		desc.height = 1;
		desc.upDirection = physx::PxVec3(0, 1, 0);
		desc.slopeLimit = 0;
		desc.stepOffset = 0.2f;
		desc.position.x = XMVectorGetX(trans->position);
		desc.position.y = XMVectorGetY(trans->position);
		desc.position.z = XMVectorGetZ(trans->position);
		desc.callback = NULL;
		desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
		bool r = desc.isValid();

		character_controller = Physics.gManager->createController(desc);
		physx::PxRigidDynamic* c = character_controller->getActor();
		
		grounded = true;

		// Kinematic capsule creation
		/*kinematic_capsule = physx::PxCreateDynamic(
			  *Physics.gPhysicsSDK
			, physx::PxTransform(
				  Physics.XMVECTORToPxVec3(trans->position)
				  , Physics.XMVECTORToPxQuat(
						XMQuaternionMultiply(
							  trans->rotation
							, XMQuaternionRotationAxis(
								  XMVectorSet(0, 0, 1, 0)
								, deg2rad(90)
							)
						)
					)
				)
			, physx::PxCapsuleGeometry(0.25f, 0.5f)
			, *Physics.gPhysicsSDK->createMaterial(
				0.5f
				, 0.5f
				, 0.5f
			)
			, 1
		);

		kinematic_capsule->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

		Physics.gScene->addActor(*kinematic_capsule);*/

		
	}

	void init() {
		CEntity* e_player = CEntityManager::get().getByName("PlayerPivot");

		assert(e_player || fatal("TPlayerController requieres a player pivot entity"));

		player_pivot_transform = e_player->get<TCompTransform>();
		TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;

		assert(player_pivot_trans || fatal("TPlayerController requieres a player pivot entity with a TTransform component"));
		
	}

	void update(float elapsed) {
		TCompTransform* transform = (TCompTransform*)m_transform;
		TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;

		bool re_rotate = false;

		delta_pos += Physics.PxVec3ToXMVECTOR(Physics.gScene->getGravity()) * elapsed;

		
		// Que teclas se pulsan -> que cambios hacer
		if (isKeyPressed('W')) {
			delta_pos += elapsed * movement_velocity * transform->getFront();
			re_rotate = true;
		}
		else if (isKeyPressed('S')) {
			delta_pos -= elapsed * movement_velocity * transform->getFront();
			re_rotate = true;
		}
		if (isKeyPressed('A')) {
			delta_pos += elapsed * movement_velocity * transform->getLeft();
			re_rotate = true;
		}
		else if (isKeyPressed('D')) {
			delta_pos -= elapsed * movement_velocity * transform->getLeft();
			re_rotate = true;
		}

		// Actualizar la posicion/rotacion
		//transform->position += delta_pos;

		if (re_rotate) {
			// Get player pivot Y rotation
			float player_pivot_yaw = getYawFromVector(player_pivot_trans->getFront());
			float m_yaw = getYawFromVector(transform->getFront());
			XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(player_pivot_trans->getUp(), player_pivot_yaw - m_yaw);
			transform->rotation = XMQuaternionSlerp(transform->rotation, XMQuaternionMultiply(transform->rotation, player_pivot_rot), 0.05f);
		}

		if (grounded) {
			if (isKeyPressed(' ')) {				
				jumping = true;
				//grounded = false;
			}
		}

		if (jumping) {
			delta_pos += XMVectorSet(0, jumpSpeed, 0, 0) * elapsed;
			delta_jump += delta_pos;

			// Jump finished
			if (XMVectorGetY(delta_jump) >= jumpHeight) {
				falling = true;
				jumping = false;
				delta_jump = XMVectorZero();
			}
		}

		if (falling) {
			if (grounded) {
				falling = false;
			}
		}

		physx::PxExtendedVec3 newPos = character_controller->getFootPosition();
		transform->position = Physics.PxVec3ToXMVECTOR(physx::PxVec3(newPos.x, newPos.y, newPos.z));
	}

	void isGrounded() {
		
	}

	void fixedUpdate(float elapsed) {
		TCompTransform* transform = (TCompTransform*)m_transform;		
		// Move the character controller
		physx::PxControllerCollisionFlags collisionFlags = character_controller->move(Physics.XMVECTORToPxVec3(delta_pos), 0.1, elapsed, NULL, NULL);
		delta_pos = XMVectorZero();

		grounded = collisionFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
		
		//physx::PxExtendedVec3 newPos = character_controller->getFootPosition();
		//transform->position = Physics.PxVec3ToXMVECTOR(physx::PxVec3(newPos.x, newPos.y, newPos.z));

		// Kinematic capsule:
		/*kinematic_capsule->setKinematicTarget(
			physx::PxTransform(
				  Physics.XMVECTORToPxVec3(transform->position)
				, Physics.XMVECTORToPxQuat(
					XMQuaternionMultiply(
						  transform->rotation
						, XMQuaternionRotationAxis(
							  XMVectorSet(0, 0, 1, 0)
							, deg2rad(90)
						)
					)
				)
			)
		);*/
	}


	std::string toString() {
		return "Player controller";
	}
};

#endif
