#include "mcv_platform.h"
#include "ragdoll_manager.h"
#include "skeletons\skeleton_manager.h"
#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"

CRagdollManager ragdoll_manager;

CCoreRagdoll::CCoreRagdoll() {}

void CCoreRagdoll::onStartElement(const std::string &elem, MKeyValue &atts) {
	
	CCoreModel* model = (CCoreModel*)skeleton_manager.getByName(name.c_str());

	if (elem == "BoneRigidbody") {

		std::string bone_name = atts.getString("name", "unknown");
		int bone_id = model->getBoneId(bone_name);

		CalCoreBone* cal_bone = model->getCoreSkeleton()->getCoreBone(bone_name);
		const CalVector& cal_pos = cal_bone->getTranslationAbsolute();
		const CalQuaternion& cal_mtx = cal_bone->getRotation();

		XMVECTOR dx_pos = Cal2DX(cal_pos);
		XMVECTOR dx_rot = Cal2DX(cal_mtx);

		physx::PxMaterial* mat = Physics.gPhysicsSDK->createMaterial(0.5, 0.5, 0.5);

		PxShape* collider = Physics.gPhysicsSDK->createShape(
			physx::PxSphereGeometry(
			physx::PxReal(0.15)
			),
			*mat
			,
			true);

		PxRigidDynamic* rigidBody = physx::PxCreateDynamic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(dx_pos),
			Physics.XMVECTORToPxQuat(dx_rot))
			, *collider
			, 1000);

		Physics.gScene->addActor(*rigidBody);

		bone_map[bone_id] = rigidBody;
	}

	else if (elem == "jointD6") {

	}

}


bool CCoreRagdoll::load(const char* name) {
	setName(name);
	root_path = "data/skeletons/" + std::string(name) + "/";

	CalLoader::setLoadingMode(LOADER_ROTATE_X_AXIS);

	char full_name[MAX_PATH];
	sprintf(full_name, "%s%s.xml", root_path.c_str(), (std::string(name) + "_ragdoll").c_str());
	return xmlParseFile(full_name);
}