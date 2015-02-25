#ifndef INC_COMP_NEEDLE_H_
#define INC_COMP_NEEDLE_H_

#include "base_component.h"

struct TCompNeedle : TBaseComponent {
private:
	// Local position, if attached to rigidbody
	XMVECTOR offset_pos;
	// Local rotation, if attached to rigidbody
	XMVECTOR offset_rot;
	// Attached rigidbody
	CHandle rigidbody;
	CHandle m_transform;
public:

	TCompNeedle() {}

	void loadFromAtts(MKeyValue& atts) {

	}

	void fixedUpdate(float elapsed) {
		// If the needle is attached to a rigidbody
		if (rigidbody.isValid()) {
			TCompRigidBody* rigid = (TCompRigidBody*)rigidbody;
			TCompTransform* trans = (TCompTransform*)m_transform;
			
			// Get the world position and rotation
			// World pos = local pos * rigid rotation + rigid pos
			trans->position = XMVector3Rotate(offset_pos, rigid->getRotation()) + rigid->getPosition();

			// World rot = local rot * rigid rotation
			trans->rotation = XMQuaternionMultiply(offset_rot, rigid->getRotation());
		}
	}
};

#endif