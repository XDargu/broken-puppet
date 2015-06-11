#ifndef INC_COMP_ROPE_H
#define INC_COMP_ROPE_H

#include "base_component.h"

struct TCompRope : TBaseComponent {
private:
	CHandle transform_1;
	CHandle transform_2;

	CHandle joint;
	float remove_counter;
public:

	XMVECTOR pos_1;
	XMVECTOR pos_2;

	float max_distance;
	float width;
	

	TCompRope() : max_distance(20), width(0.02f), remove_counter(0) {}

	void setPositions(CHandle the_transform_1, XMVECTOR the_pos_2);

	void setPositions(XMVECTOR the_pos_1, CHandle the_transform_2);

	// With two transforms, a distance joiny is required
	void setPositions(CHandle the_transform_1, CHandle the_transform_2);

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void fixedUpdate(float elapsed);
};

#endif