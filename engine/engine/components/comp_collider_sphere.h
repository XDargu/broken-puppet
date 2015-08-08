#ifndef INC_COMP_SPHERE_COLLIDER_H_
#define INC_COMP_SPHERE_COLLIDER_H_

#include "base_component.h"
#include "comp_aabb.h"
#include "collider.h"

struct TCompColliderSphere : public CCollider, TBaseComponent {
private:
	XMVECTOR t_current;
	XMVECTOR t_previous;
	//float* m_v;
	//int*   t_v;
public:
	TCompColliderSphere() : CCollider() { /*m_v = nullptr, t_v = nullptr;*/ }

	//~TCompColliderSphere();

	void setShape(float radius, float static_friction, float dynamic_friction, float restitution);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void addInputNavMesh();

	void setCollisionGroups();

	void setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems);

	bool getIfUpdated();
};

#endif
