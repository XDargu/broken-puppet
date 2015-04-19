#ifndef INC_COMP_RAGDOLL_H_
#define INC_COMP_RAGDOLL_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompRagdoll : TBaseComponent {
private:
	CHandle skeleton;
	std::map<int, PxRigidDynamic*> bone_map;
	std::vector<PxD6Joint*> articulations;
	bool ragdoll_active;
public:

	TCompRagdoll();
	~TCompRagdoll();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void fixedUpdate(float elapsed);

	void setActive(bool active);
	bool isRagdollActive();

	PxRigidDynamic* getBoneRigid(int id);
};

#endif