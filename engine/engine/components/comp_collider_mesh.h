
#include "base_component.h"
#include "nav_mesh_manager.h"
#include "physics_manager.h"
#include "render\collision_mesh.h"
#include "comp_aabb.h"
#include "collider.h"
#ifndef INC_COMP_COLLIDER_MESH_H
#define INC_COMP_COLLIDER_MESH_H

#include "base_component.h"
#include "comp_aabb.h"

struct TCompColliderMesh : public CCollider, TBaseComponent {
private:
	float* m_v;
	int*   t_v;
public:
	char path[64];
	TCompColliderMesh() : CCollider() { m_v = nullptr, t_v = nullptr; }
	TCompColliderMesh(physx::PxShape* the_mesh_collider) { collider = the_mesh_collider; strcpy(path, "unknown"); m_v = nullptr, t_v = nullptr; }
	~TCompColliderMesh();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void addInputNavMesh();

	void setCollisionGroups();

	void setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems);
};

#endif
