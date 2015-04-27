#ifndef INC_COMP_SPHERE_COLLIDER_H_
#define INC_COMP_SPHERE_COLLIDER_H_

#include "base_component.h"
#include "comp_aabb.h"
#include "collider.h"

struct TCompColliderSphere : public CCollider, TBaseComponent {
public:
	TCompColliderSphere() : CCollider() {}

	void setShape(float radius, float static_friction, float dynamic_friction, float restitution);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void addInputNavMesh();
};

#endif
