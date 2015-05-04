#ifndef INC_RAGDOLL_MANAGER_H_
#define INC_RAGDOLL_MANAGER_H_

#include "items_by_name.h"
#include "physics_manager.h"
#include "cal3d/cal3d.h"
#include "XMLParser.h"

// Mas otras cosas que cal3d no tiene
class CCoreRagdoll : public CXMLParser {
	std::string               root_path;
	std::string               name;
	int						  uid;

	void onStartElement(const std::string &elem, MKeyValue &atts);

	PxTransform getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);
public:
	std::map<int, PxRigidDynamic*> bone_map;
	std::vector<PxD6Joint*> articulations;

	CCoreRagdoll();
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const std::string& getName() const { return name; }
};

typedef CItemsByName< CCoreRagdoll > CRagdollManager;
extern CRagdollManager ragdoll_manager;

#endif
