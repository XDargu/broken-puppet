#ifndef INC_COMP_GOLDEN_NEEDLE_H_
#define INC_COMP_GOLDEN_NEEDLE_H_

#include "base_component.h"
#include "comp_transform.h"
#include "physics_manager.h"

struct TCompGoldenNeedle : TBaseComponent {
private:
	XMVECTOR finish_position;
	XMVECTOR initial_position;
	XMVECTOR first_reached_pos;
	CHandle m_transform;
	bool used;
	bool first_reached;
public:

	TCompGoldenNeedle();

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	//void init();

	//void create(XMVECTOR the_offset_pos, XMVECTOR the_offset_rot, CHandle target_rigidbody);
	void create(XMVECTOR init_pos, XMVECTOR init_rot, XMVECTOR finish_pos);

	TCompTransform* getTransform() {
		return (TCompTransform*)m_transform;
	}

	void fixedUpdate(float elapsed);

	bool getUsed();
};
#endif