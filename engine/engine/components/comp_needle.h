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

	void loadFromAtts(const std::string& elem, MKeyValue& atts) {

	}

	void create(XMVECTOR the_offset_pos, XMVECTOR the_offset_rot, CHandle target_rigidbody) {
		rigidbody = target_rigidbody;
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

	TCompTransform* getTransform(){
		return (TCompTransform*)m_transform;
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