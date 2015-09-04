#include "mcv_platform.h"
#include "comp_local_rotation.h"
#include "comp_transform.h"
#include "comp_rigid_body.h"

void TCompLocalRotation::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
	speed = atts.getFloat("speed", 1);
	axis = atts.getInt("axis", 0);
	move_with_speed = atts.getBool("moveSpeedParent", false);

	TCompTransform* c_transform = h_transform;
	origin = c_transform->position;
}

void TCompLocalRotation::update(float elapsed) {

	TCompTransform* c_transform = h_transform;

	CHandle parent = ((TCompTransform*)h_transform)->getParent();
	if (parent.isValid()) {
		h_rigidbody = ((CEntity*)(parent.getOwner()))->get<TCompRigidBody>();
	}

	if (c_transform->hasParent()) {

		XMVECTOR rot_vec = c_transform->local_transform.getFront();
		XMVECTOR rot_dot_vec = c_transform->getFront();
		if (axis == 0) {
			rot_vec = c_transform->local_transform.getFront();
			rot_dot_vec = c_transform->getFront();
		}
		if (axis == 1) {
			rot_vec = -c_transform->local_transform.getLeft();
			rot_dot_vec = -c_transform->getLeft();
		}
		if (axis == 2) {
			rot_vec = c_transform->local_transform.getUp();
			rot_dot_vec = c_transform->getUp();
		}		

		float parent_speed = 1;
		if (move_with_speed) {
			TCompRigidBody* c_rigidbody = h_rigidbody;
			if (c_rigidbody) {
				
				float magnitude = c_rigidbody->rigidBody->getLinearVelocity().magnitude();
				XMVECTOR velocity = Physics.PxVec3ToXMVECTOR(c_rigidbody->rigidBody->getLinearVelocity().getNormalized());

				XMVECTOR to_origin = XMVector3Normalize(c_transform->position - origin);

				if (magnitude > 0) {
					
					XMVECTOR angle2 = XMVector3Cross(to_origin, rot_dot_vec);
					rot_dot_vec = XMVector3Rotate(rot_dot_vec, XMQuaternionRotationAxis(angle2, deg2rad(90)));
					float dot = XMVectorGetX(XMVector3Dot(rot_dot_vec, velocity));

					parent_speed = dot * magnitude;
				}
				else {
					parent_speed = 0;
				}
			}
		}

		XMVECTOR rot = XMQuaternionRotationAxis(rot_vec, speed * parent_speed * elapsed);
		c_transform->local_transform.rotation = XMQuaternionMultiply(c_transform->local_transform.rotation, rot);
	}
	else {
		XMVECTOR rot = XMVectorSet(0, 0, 0, 1);
		if (axis == 0) {
			rot = XMQuaternionRotationAxis(c_transform->getFront(), speed * elapsed);
		}
		if (axis == 1) {
			rot = XMQuaternionRotationAxis(-c_transform->getLeft(), speed * elapsed);
		}
		if (axis == 2) {
			rot = XMQuaternionRotationAxis(c_transform->getUp(), speed * elapsed);
		}

		c_transform->rotation = XMQuaternionMultiply(c_transform->rotation, rot);
	}
}