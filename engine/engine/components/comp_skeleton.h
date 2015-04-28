#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "base_component.h"
#include "transform.h"

class CalModel;
class CalBone;


struct TCompSkeleton : TBaseComponent {
private:
	CHandle h_ragdoll;
	TTransform* bone_ragdoll_transforms;
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

	void stopAnimation(int id);
	void loopAnimation(int id);
	void playAnimation(int id);

	void uploadBonesToGPU() const;
	XMVECTOR getPositionOfBone(int id);
};

#endif
