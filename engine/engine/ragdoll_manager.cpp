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

		float radius = atts.getFloat("radius", 0.2);
		float half_height = atts.getFloat("height", 0.2) / 2;

		XMVECTOR rotation_in_max_coords = atts.getQuat("rotation");
		XMVECTOR max2mcv = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), deg2rad(0));

		XMVECTOR rotation = XMQuaternionMultiply(rotation_in_max_coords, max2mcv);

		std::string bone_name = atts.getString("name", "unknown");
		int bone_id = model->getBoneId(bone_name);

		CalCoreBone* cal_bone = model->getCoreSkeleton()->getCoreBone(bone_name);
		const CalVector& cal_pos = cal_bone->getTranslationAbsolute();
		CalQuaternion& cal_mtx = (CalQuaternion)cal_bone->getRotationAbsolute();

		XMVECTOR corrector = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), deg2rad(90));

		//cal_mtx *= DX2CalQuat(corrector);

		XMVECTOR dx_pos = Cal2DX(cal_pos);
		XMVECTOR dx_rot = Cal2DX(cal_mtx);

		physx::PxMaterial* mat = Physics.gPhysicsSDK->createMaterial(0.5, 0.5, 0.5);

		PxShape* collider = Physics.gPhysicsSDK->createShape(
			physx::PxCapsuleGeometry(
			radius,
			half_height
			),
			*mat
			,
			true);

		// Rotate the capsule (we want the capsule in a vertical position)
		//physx::PxTransform relativePose(physx::PxVec3((radius + half_height), 0, 0), physx::PxQuat(deg2rad(90), physx::PxVec3(0, 0, 1)));
		//physx::PxTransform relativePose(physx::PxVec3(0, (radius + half_height), 0));
		physx::PxTransform relativePose(physx::PxVec3(radius + half_height, 0, 0)/*, Physics.XMVECTORToPxQuat(rotation)*/);
		collider->setLocalPose(relativePose);

		/*PxShape* collider = Physics.gPhysicsSDK->createShape(
			physx::PxSphereGeometry(
			radius
			),
			*mat
			,
			true);*/

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

		PxVec3 joint_position = Physics.XMVECTORToPxVec3(atts.getPoint("jointPosition"));
		PxQuat joint_rotation = Physics.XMVECTORToPxQuat(atts.getQuat("jointRotation"));

		std::string bone_name1 = atts.getString("actor1", "unknown");
		int bone_id1 = model->getBoneId(bone_name1);

		std::string bone_name2 = atts.getString("actor2", "unknown");
		int bone_id2 = model->getBoneId(bone_name2);

		PxRigidDynamic* m_ridig_dynamic1 = bone_map[bone_id1];
		PxRigidDynamic* m_ridig_dynamic2 = bone_map[bone_id2];

		PxTransform t_1;
		PxTransform t_2;

		// Get joint transforms relative to rigidbodies
		PxTransform joint_abs = PxTransform(joint_position, joint_rotation);

		PxTransform rigid_abs_inverse1 = m_ridig_dynamic1->getGlobalPose();
		rigid_abs_inverse1 = rigid_abs_inverse1.getInverse();

		PxTransform rigid_abs_inverse2 = m_ridig_dynamic2->getGlobalPose();
		rigid_abs_inverse2 = rigid_abs_inverse2.getInverse();

		t_1 = rigid_abs_inverse1.transform(joint_abs);
		t_2 = rigid_abs_inverse2.transform(joint_abs);


		// Create a joint
		PxD6Joint* mJoint = PxD6JointCreate(
			*Physics.gPhysicsSDK
			, m_ridig_dynamic1
			, t_1
			, m_ridig_dynamic2
			, t_2
			);

		//mJoint->set

		mJoint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLOCKED);
		mJoint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);
		mJoint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);

		mJoint->setMotion(PxD6Axis::eX, PxD6Motion::eLOCKED);
		mJoint->setMotion(PxD6Axis::eY, PxD6Motion::eLOCKED);
		mJoint->setMotion(PxD6Axis::eZ, PxD6Motion::eLOCKED);

		//std::string res_order[3] = { "swing1Mode", "swing2Mode", "twistMode" };
		//std::string a_order[3] = { "swing1Angle", "swing2Angle", "twistAngle" };

		std::string res_order[3] = { "twistMode", "swing2Mode", "swing1Mode" };
		std::string a_order[3] = { "twistAngle", "swing2Angle", "swing1Angle" };

		PxD6Motion::Enum swing1_motion = (PxD6Motion::Enum)(atts.getInt(res_order[0].c_str(), 0) - 1);
		PxD6Motion::Enum swing2_motion = (PxD6Motion::Enum)(atts.getInt(res_order[1].c_str(), 0) - 1);
		PxD6Motion::Enum twist_motion = (PxD6Motion::Enum)(atts.getInt (res_order[2].c_str(), 0) - 1);

		float swing1Angle = atts.getFloat(a_order[0].c_str(), 45);
		float swing2Angle = atts.getFloat(a_order[1].c_str(), 45);
		float twistAngle = atts.getFloat(a_order[2].c_str(), 45);

		// SWING 1: Rot Y
		// SWING 2: Rot X
		// TWIST: Rot Z
		mJoint->setMotion(PxD6Axis::eSWING1, swing1_motion);
		mJoint->setMotion(PxD6Axis::eSWING2, swing2_motion);
		mJoint->setMotion(PxD6Axis::eTWIST, twist_motion);

		// Primer valor: Swing 1 angle / 2, segundo valor: Swing 2 angle / 2
		mJoint->setSwingLimit(PxJointLimitCone(deg2rad(swing1Angle), deg2rad(swing2Angle), -1));

		// Twist limit
		mJoint->setTwistLimit(PxJointAngularLimitPair(deg2rad(-twistAngle), deg2rad(twistAngle), -1));

		articulations.push_back(mJoint);
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

PxTransform CCoreRagdoll::getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot) {

	// Global to local (we need the joint position relative to the transform)
	// Using CalVector and CalQuaternion, they have the needed methods

	// Parent = transform
	// Rigid = joint

	PxTransform anchor = PxTransform(0, 0, 0, PxQuat(deg2rad(90), PxVec3(0, 0, 1)));
	
	CalQuaternion parent_to_local = DX2CalQuat(Physics.PxQuatToXMVECTOR(body_transform.q));
	parent_to_local.invert();

	CalQuaternion new_local_rotation = DX2CalQuat(Physics.PxQuatToXMVECTOR(joint_rot));
	new_local_rotation *= parent_to_local;

	anchor.q = Physics.XMVECTORToPxQuat(Cal2DX(new_local_rotation));

	CalVector delta_abs_pos = DX2Cal(Physics.PxVec3ToXMVECTOR(joint_pos)) - DX2Cal(Physics.PxVec3ToXMVECTOR(body_transform.p));
	CalQuaternion my_rotation_to_local = DX2CalQuat(Physics.PxQuatToXMVECTOR(body_transform.q));
	my_rotation_to_local.invert();

	CalVector delta_in_local = delta_abs_pos;
	delta_in_local *= my_rotation_to_local;

	anchor.p = Physics.XMVECTORToPxVec3(Cal2DX(delta_in_local));

	

	/*PxVec3 diff_pos;

	PxTransform anchor = PxTransform(0, 0, 0);
	diff_pos = joint_pos - body_transform.p;
	diff_pos = body_transform.q.rotateInv(diff_pos);
	anchor.p = anchor.p + diff_pos;

	anchor.q = joint_rot * body_transform.q.getConjugate();
	//anchor.q *= PxQuat(deg2rad(90), PxVec3(0, 0, 1));*/

	return anchor;
}