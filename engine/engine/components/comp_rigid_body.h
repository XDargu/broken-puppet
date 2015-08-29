#ifndef INC_COMP_RIGID_BODY_H_
#define INC_COMP_RIGID_BODY_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompRigidBody : TBaseComponent {
private:
	CHandle transform;
	physx::PxD6Joint* block_joint;
	float density;
public:
	physx::PxRigidDynamic* rigidBody;
	double impact_timestamp;
	bool auto_translate_transform;
	bool auto_rotate_transform;
	int boss_level;

	TCompRigidBody() :
		rigidBody(nullptr),
		auto_translate_transform(true),
		auto_rotate_transform(true)
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
};

#endif
