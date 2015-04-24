#ifndef INC_COMP_NEEDLE_H_
#define INC_COMP_NEEDLE_H_

#include "base_component.h"
#include "comp_transform.h"

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

	TCompNeedle();

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void create(XMVECTOR the_offset_pos, XMVECTOR the_offset_rot, CHandle target_rigidbody);

	TCompTransform* getTransform() {
		return (TCompTransform*)m_transform;
	}

	void fixedUpdate(float elapsed);
};

#endif