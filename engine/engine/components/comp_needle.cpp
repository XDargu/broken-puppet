#include "mcv_platform.h"
#include "comp_needle.h"
#include "comp_rigid_body.h"
#include "comp_transform.h"

TCompNeedle::TCompNeedle() : physx_rigidbody(nullptr) {}

void TCompNeedle::loadFromAtts(const std::string& elem, MKeyValue& atts) {

}

/*void TCompNeedle::create(XMVECTOR the_offset_pos, XMVECTOR the_offset_rot, CHandle target_rigidbody) {
	rigidbody = target_rigidbody;

	TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;
	if (rigid) {
		physx_rigidbody = rigid->rigidBody;
	}

	m_transform = assertRequiredComponent<TCompTransform>(this);
	offset_pos = the_offset_pos;
	offset_rot = the_offset_rot;

	if (!rigidbody.isValid()) {
		TCompTransform* trans = (TCompTransform*)m_transform;

		// Static position
		trans->position = offset_pos;

		// World rot = local rot * rigid rotation
		trans->rotation = offset_rot;
	}
}*/

void TCompNeedle::create(XMVECTOR the_offset_pos, XMVECTOR the_offset_rot, PxRigidActor* target_rigidbody) {

	physx_rigidbody = target_rigidbody;
	m_transform = assertRequiredComponent<TCompTransform>(this);
	offset_pos = the_offset_pos;
	offset_rot = the_offset_rot;

	if (!rigidbody.isValid()) {
		TCompTransform* trans = (TCompTransform*)m_transform;

		// Static position
		trans->position = offset_pos;

		// World rot = local rot * rigid rotation
		trans->rotation = offset_rot;
	}
}

void TCompNeedle::fixedUpdate(float elapsed) {
	// If the needle is attached to a rigidbody

	if (physx_rigidbody->isRigidDynamic())
	{
		TCompTransform* trans = (TCompTransform*)m_transform;

		XMVECTOR rot = Physics.PxQuatToXMVECTOR(physx_rigidbody->getGlobalPose().q);
		XMVECTOR pos = Physics.PxVec3ToXMVECTOR(physx_rigidbody->getGlobalPose().p);

		// Get the world position and rotation
		// World pos = local pos * rigid rotation + rigid pos
		trans->position = XMVector3Rotate(offset_pos, rot) + pos;

		// World rot = local rot * rigid rotation
		trans->rotation = XMQuaternionMultiply(offset_rot, rot);
	}

	/*if (rigidbody.isValid()) {
		TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;
		TCompTransform* trans = (TCompTransform*)m_transform;

		// Get the world position and rotation
		// World pos = local pos * rigid rotation + rigid pos
		trans->position = XMVector3Rotate(offset_pos, rigid->getRotation()) + rigid->getPosition();

		// World rot = local rot * rigid rotation
		trans->rotation = XMQuaternionMultiply(offset_rot, rigid->getRotation());
	}*/
}