#ifndef INC_COMP_COLLIDER_CAPSULE_H_
#define INC_COMP_COLLIDER_CAPSULE_H_

#include "base_component.h"
#include "comp_aabb.h"
#include "collider.h"

struct TCompColliderCapsule : public CCollider, TBaseComponent {
private:
	XMVECTOR t_current;
	XMVECTOR t_previous;
public:
	TCompColliderCapsule() : CCollider() {}

	void setShape(float radius, float half_height, float static_friction, float dynamic_friction, float restitution);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	// Returns the half height of the capsule
	float getHalfHeight();

	// Returns the radius of the capsule
	float getRadius();

	void addInputNavMesh();

	void setCollisionGroups();

	void setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems);

	bool  getIfUpdated();
};

#endif
