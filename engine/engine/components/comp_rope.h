#ifndef INC_COMP_ROPE_H
#define INC_COMP_ROPE_H

#include "base_component.h"
#include "physics_manager.h"

struct TCompRope : TBaseComponent {
private:
	CHandle transform_1;
	CHandle transform_2;

	CHandle joint;

	float remove_counter;
public:

	CHandle transform_1_aux;
	CHandle transform_2_aux;
	CHandle joint_aux;

	XMVECTOR pos_1;
	XMVECTOR pos_2;

	bool tensed;

	float max_distance;
	float width;
	

	TCompRope() : max_distance(20), width(0.02f), remove_counter(0), tensed(false){}

	~TCompRope();

	void setPositions(CHandle the_transform_1, XMVECTOR the_pos_2);

	void setPositions(XMVECTOR the_pos_1, CHandle the_transform_2);

	// With two transforms, a distance joiny is required
	void setPositions(CHandle the_transform_1, CHandle the_transform_2);

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void fixedUpdate(float elapsed);

	bool getStaticPosition(XMVECTOR& position);

	void tenseRope();
};

#endif