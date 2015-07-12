#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "base_component.h"
#include "transform.h"

class CalModel;
class CalBone;
struct CalTransform;

struct TCompSkeleton : TBaseComponent {
private:
	CHandle h_ragdoll;
	CalTransform* bone_ragdoll_transforms;
	CCoreModel* core_model;
	float time_since_last_ragdoll;
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

	void cancelAnimation(int id);
	void stopAnimation(int id);
	void loopAnimation(int id);
	void playAnimation(int id);
	void resetAnimationTime();

	float getAnimationDuration(int id);

	void uploadBonesToGPU() const;
	XMVECTOR getPositionOfBone(int id);
	XMVECTOR getRotationOfBone(int id);

	void ragdollUnactive();

	CCoreModel* getCCoreModel() { return core_model; }

	float getCancelTime(int id);
};

#endif
