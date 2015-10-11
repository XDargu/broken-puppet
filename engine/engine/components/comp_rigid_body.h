#ifndef INC_COMP_RIGID_BODY_H_
#define INC_COMP_RIGID_BODY_H_

#include "base_component.h"
#include "physics_manager.h"
#include "handle\app_msgs.h"

struct TCompRigidBody : TBaseComponent {
private:
	CHandle rigid_entity;
	CHandle transform;
	physx::PxD6Joint* block_joint;
	float density;
	bool underwater;
public:
	physx::PxRigidDynamic* rigidBody;
	double impact_timestamp;
	bool auto_translate_transform;
	bool auto_rotate_transform;
	int boss_level;
	enum colliderType {
		BOX,
		SPHERE,
		CAPSULE,
		MULTIPLE,
		CONVEX,
		MESH
	};
	colliderType kind;

	TCompRigidBody() :
		rigidBody(nullptr),
		auto_translate_transform(true),
		auto_rotate_transform(true),
		underwater(false)
	{}

	~TCompRigidBody() { Physics.gScene->removeActor(*rigidBody); }

	void create(float density, bool is_kinematic, bool use_gravity);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void fixedUpdate(float elapsed);

	XMVECTOR getPosition();

	XMVECTOR getRotation();

	void setKinematic(bool is_kinematic);

	bool isKinematic();

	void setUseGravity(bool use_gravity);

	bool isUsingGravity();

	void setLockXPos(bool locked);

	void setLockYPos(bool locked);

	void setLockZPos(bool locked);

	void setLockXRot(bool locked);

	void setLockYRot(bool locked);

	void setLockZRot(bool locked);

	physx::PxReal getMass();

	void setCollisionMask();

	// Explosion

	void onExplosion(const TMsgExplosion& msg);

	void checkIfInsideRecastAABB();
};

#endif
