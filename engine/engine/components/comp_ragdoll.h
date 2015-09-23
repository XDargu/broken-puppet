#ifndef INC_COMP_RAGDOLL_H_
#define INC_COMP_RAGDOLL_H_

#include "base_component.h"
#include "ragdoll_manager.h"
#include "physics_manager.h"

struct TCompRagdoll : TBaseComponent {
private:
	CHandle skeleton;
	CCoreRagdoll* ragdoll;

	bool ragdoll_active;
public:

	TCompRagdoll();
	~TCompRagdoll();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void fixedUpdate(float elapsed);

	void setActive(bool active);
	bool isRagdollActive();

	PxRigidDynamic* getBoneRigid(int id);

	void setCollisonPlayer(bool active);

	void setCollisonBoss(bool active);

	void setCollisonEnemy(bool active);

	void setCollisonPlayerBone(bool active, int bone_id);

	void setCollisonBossBone(bool active, int bone_id);

	void setCollisonEnemyBone(bool active, int bone_id);

	void setCollisionGroups();

	void breakJoints();
	void breakJoint(int id);

	PxRigidDynamic* getBoneRigidRaycast(XMVECTOR origin, XMVECTOR dir);

	void disableBoneTree(PxRigidDynamic* rigid_bone);
	void disableBoneTree(int bone);
	void enableBoneTree(int bone);
	void cancelLinearVelocity();
	void addForce(PxVec3 force, PxForceMode::Enum mode);
};

#endif