#ifndef INC_COMP_COLLIDER_BOX_H
#define INC_COMP_COLLIDER_BOX_H

#include "base_component.h"
#include "comp_aabb.h"
#include "collider.h"

struct TCompColliderBox : public CCollider, TBaseComponent {
private:
	XMVECTOR t_current;
	XMVECTOR t_previous;
public:
	TCompColliderBox() : CCollider() { }
	void setShape(float boxX, float boxY, float boxZ, float static_friction, float dynamic_friction, float restitution);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void addInputNavMesh();

	void setCollisionGroups();

	bool  getIfUpdated();
};

#endif
