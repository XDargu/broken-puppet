#ifndef INC_COMP_PLAYER_CONTROLLER_H_
#define INC_COMP_PLAYER_CONTROLLER_H_

#include "base_component.h"

struct TCompPlayerController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_pivot_transform;
public:

	physx::PxRigidDynamic*	 playerRigid; // Kinematic player. Should be changed to Physx Character Controller
	float movement_velocity;
	float rotation_velocity;

	TCompPlayerController()
		: movement_velocity(5.0f)
		, rotation_velocity(deg2rad(90.f))
	{}

	void loadFromAtts(MKeyValue &atts) {
		movement_velocity = atts.getFloat("movementVelocity", 5);
		rotation_velocity = deg2rad(atts.getFloat("rotationVelocity", 90));

		// Kinematic player creation
		playerRigid = physx::PxCreateDynamic(*Physics.gPhysicsSDK, physx::PxTransform(physx::PxVec3(0, 0, 0)), physx::PxCapsuleGeometry(0.5f, 1.0f),
			*Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f), 100.0f);
		playerRigid->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		Physics.gScene->addActor(*playerRigid);
	}

	void init() {
		CEntity* e_player = CEntityManager::get().getByName("PlayerPivot");

		assert(e_player || fatal("TPlayerController requieres a player pivot entity"));

		player_pivot_transform = e_player->get<TCompTransform>();
		TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;

		assert(player_pivot_trans || fatal("TPlayerController requieres a player pivot entity with a TTransform component"));

		CEntity* e = CHandle(this).getOwner();
		m_transform = e->get<TCompTransform>();
		TCompTransform* trans = (TCompTransform*)m_transform;

		assert(trans || fatal("TPlayerController requieres a TTransform component"));

		// Teleport the kinematic player to the player position
		physx::PxVec3 position_player = Physics.XMVECTORToPxVec3(trans->position);
		position_player.y *= 0.5f;

		physx::PxQuat rotation_player = Physics.XMVECTORToPxQuat(trans->rotation);
		rotation_player *= physx::PxQuat(deg2rad(90), physx::PxVec3(0, 0, 1));

		playerRigid->setGlobalPose(physx::PxTransform(position_player, rotation_player), true);
	}

	void update(float elapsed) {
		TCompTransform* transform = (TCompTransform*)m_transform;
		TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;

		XMVECTOR delta_pos = XMVectorZero();
		bool re_rotate = false;

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
		transform->position += delta_pos;

		if (re_rotate) {
			// Get player pivot Y rotation
			float player_pivot_yaw = getYawFromVector(player_pivot_trans->getFront());
			float m_yaw = getYawFromVector(transform->getFront());
			XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(player_pivot_trans->getUp(), player_pivot_yaw - m_yaw);
			transform->rotation = XMQuaternionSlerp(transform->rotation, XMQuaternionMultiply(transform->rotation, player_pivot_rot), 0.05f);
		}
	}

	void fixedUpdate(float elapsed) {
		TCompTransform* trans = (TCompTransform*)m_transform;

		// Kinematic player update
		// Rotate the capsule
		physx::PxVec3 position_player = Physics.XMVECTORToPxVec3(trans->position);
		position_player.y *= 0.5f;

		physx::PxQuat rotation_player = Physics.XMVECTORToPxQuat(trans->rotation);
		rotation_player *= physx::PxQuat(deg2rad(90), physx::PxVec3(0, 0, 1));

		playerRigid->setKinematicTarget(physx::PxTransform(position_player, rotation_player));
	}


	std::string toString() {
		return "Player controller";
	}
};

#endif
