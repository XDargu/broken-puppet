
#include "base_component.h"
#include "nav_mesh_manager.h"
#include "physics_manager.h"
#include "render\collision_mesh.h"
#include "comp_aabb.h"
#include "collider.h"
#ifndef INC_COMP_COLLIDER_CONVEX_H
#define INC_COMP_COLLIDER_CONVEX_H

#include "base_component.h"
#include "comp_aabb.h"

struct TCompColliderConvex : public CCollider, TBaseComponent {
private:
	float* m_v;
	int*   t_v;
public:
	char path[64];

	TCompColliderConvex() : CCollider() { m_v = nullptr, t_v = nullptr; }

	TCompColliderConvex(physx::PxShape* the_convex_collider) { collider = the_convex_collider; strcpy(path, "unknown"); m_v = nullptr, t_v = nullptr; }
	
	~TCompColliderConvex();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void addInputNavMesh();

	void setCollisionGroups();

	void setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems);
};

#endif

