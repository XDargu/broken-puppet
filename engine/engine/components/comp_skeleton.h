#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "base_component.h"

class CalModel;
class CalBone;


struct TCompSkeleton : TBaseComponent {
private:
	CHandle h_ragdoll;
public:
	CHandle h_transform;
	CHandle h_rigidbody;

	CalModel*  model;

	TCompSkeleton() : model(nullptr) { }
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void update(float elapsed);
	void renderDebug3D() const;
	void renderBoneAxis(int bone_id) const;

	void uploadBonesToGPU() const;

};

#endif
