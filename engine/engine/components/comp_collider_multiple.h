#ifndef INC_COMP_COLLIDER_MULTIPLE_H
#define INC_COMP_COLLIDER_MULTIPLE_H

#include "base_component.h"
#include "nav_mesh_manager.h"
#include "physics_manager.h"
#include "render\collision_mesh.h"
#include "comp_aabb.h"
#include "collider.h"

struct TCompColliderMultiple : public CCollider, TBaseComponent {
private:
	PxMaterial* mat;
	XMVECTOR t_current;
	XMVECTOR t_previous;
	float* m_v;
	int*   t_v;
public:
	std::vector<PxShape*> colliders;

	TCompColliderMultiple() : CCollider() { m_v = nullptr, t_v = nullptr; }

	~TCompColliderMultiple();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void addInputNavMesh();

	void setCollisionGroups(physx::PxShape* collider);

	void setCollisionGroups(PxU32 own_mask, PxU32* vector_masks, int num_elems);

	bool  getIfUpdated();
};

#endif