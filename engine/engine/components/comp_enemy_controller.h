#ifndef INC_COMP_ENEMY_CONTROLLER_H_
#define INC_COMP_ENEMY_CONTROLLER_H_

#include "base_component.h"

struct TCompEnemyController : TBaseComponent {
private:
	CHandle m_transform;	
	XMVECTOR delta_jump;
	physx::PxVec3 gravity;
public:
	XMVECTOR delta_pos;

	bool jumping;
	bool grounded;
	bool falling;
	float jumpHeight;
	float jumpSpeed;

	physx::PxController* character_controller;

	float movement_velocity;
	float rotation_velocity;

	TCompEnemyController()
		: movement_velocity(5.0f)
		, rotation_velocity(deg2rad(90.f))
		, delta_pos(XMVectorZero())
		, jumpHeight(20)
		, jumpSpeed(20)
		, jumping(false)
		, grounded(false)
		, falling(false)
		, delta_jump(DirectX::XMVectorZero())
	{}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		movement_velocity = atts.getFloat("movementVelocity", 5);
		rotation_velocity = deg2rad(atts.getFloat("rotationVelocity", 90));

		m_transform = assertRequiredComponent<TCompTransform>(this);
		TCompTransform* trans = (TCompTransform*)m_transform;

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
		desc.position.y = XMVectorGetY(trans->position) + 10;
		desc.position.z = XMVectorGetZ(trans->position);
		desc.callback = NULL;
		bool r = desc.isValid();

		character_controller = Physics.gManager->createController(desc);
		physx::PxRigidDynamic* c = character_controller->getActor();

		grounded = true;
	}

	void init() {
		CEntity* e_player = CEntityManager::get().getByName("PlayerPivot");
		delta_pos = XMVectorSet(1, 1, 1, 0);
		gravity = Physics.gScene->getGravity();
	}

	void addDeltaPos(XMVECTOR delta) {
		delta_pos += delta;
	}

	bool getIsGrounded(){
		return grounded;
	}

	void fixedUpdate(float elapsed) {
		TCompTransform* transform = (TCompTransform*)m_transform;
		delta_pos +=  Physics.PxVec3ToXMVECTOR( gravity ) * elapsed;

		// Move the character controller
		PxControllerCollisionFlags collisionFlags = character_controller->move(Physics.XMVECTORToPxVec3(delta_pos), 0.1f, elapsed, NULL, NULL);
		delta_pos = XMVectorZero();

		grounded = collisionFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);

		PxExtendedVec3 newPos = character_controller->getFootPosition();
		transform->position = Physics.PxVec3ToXMVECTOR(PxVec3((PxReal)newPos.x, (PxReal)newPos.y, (PxReal)newPos.z));
	}


	std::string toString() {
		return "Player controller";
	}
};

#endif
