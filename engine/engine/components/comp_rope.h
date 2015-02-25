#ifndef INC_COMP_ROPE_H
#define INC_COMP_ROPE_H

#include "base_component.h"

struct TCompRope : TBaseComponent {
private:
	CHandle needle_1;
	CHandle needle_2;
	CHandle joint;
public:

	float max_distance;
	float width;

	TCompRope() : max_distance( 20 ) , width ( 0.02f ) {}

	void create(/*CHandle the_needle_1, CHandle the_needle_2*/) {
		joint = assertRequiredComponent<TCompDistanceJoint>(this);

		/*needle_1 = the_needle_1;
		needle_2 = the_needle_2;*/
	}

	void loadFromAtts(MKeyValue& atts) {
		joint = assertRequiredComponent<TCompDistanceJoint>(this);

		width = atts.getFloat("width", 0.02f);
		max_distance = atts.getFloat("maxDistance", 20);
	}

	void fixedUpdate(float elapsed) {
		// If the joint distance exceeds the max distance, the destroy the entity
		float dist = ((TCompDistanceJoint*)joint)->joint->getDistance();
		if (dist > max_distance * max_distance) {
			CEntityManager::get().remove(CHandle(this).getOwner());
		}
	}
};

#endif