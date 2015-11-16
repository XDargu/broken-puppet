#include "mcv_platform.h"
#include "comp_switch_pull_controller.h"
#include "comp_joint_prismatic.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "ai\logic_manager.h"

void TCompSwitchPullController::loadFromAtts(const std::string& elem, MKeyValue &atts) {

}

void TCompSwitchPullController::init() {
	TCompJointPrismatic* joint = assertRequiredComponent<TCompJointPrismatic>(this);

	actor1 = joint->getActor1();
	actor2 = joint->getActor2();

	// check if there are two actors
	XASSERT(actor1.isValid(), "The switch needs two valid bodies in a joint");
	XASSERT(actor2.isValid(), "The switch needs two valid bodies in a joint");

	limit = joint->getLinealPosition();

	bool is_static = false;
	bool is_rigid = false;

	CHandle r1;
	CHandle s1;
	CHandle r2;
	CHandle s2;

	r1 = ((CEntity*)actor1)->get<TCompRigidBody>();
	s1 = ((CEntity*)actor1)->get<TCompStaticBody>();


	r2 = ((CEntity*)actor2)->get<TCompRigidBody>();
	s2 = ((CEntity*)actor2)->get<TCompStaticBody>();

	if (r1.isValid()){
		px_actor1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody;
		is_rigid = true;
	}
	else if (s1.isValid()){
		px_actor1 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody;
		is_static = true;
	}

	if (r2.isValid()){
		px_actor2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody;
		is_rigid = true;
	}
	if (s2.isValid()){
		px_actor2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody;
		is_static = true;
	}

	XASSERT(is_static && is_rigid, "The switch pull needs ans static body and an rigidbody");
}

void TCompSwitchPullController::update(float elapsed) {
	PxTransform Pos1 = px_actor1->getGlobalPose();
	PxTransform Pos2 = px_actor2->getGlobalPose();

	if ((pressed == false) && ((Pos1.p - Pos2.p).magnitudeSquared() >= (limit*limit) * 3 / 5)){
		// Call on press function
		onPress();
		pressed = true;
	}
	else if ((pressed == true) && ((Pos1.p - Pos2.p).magnitudeSquared() < (limit*limit) * 3 / 5)){
		// Call on press function
		onLeave();
		pressed = false;
	}
}

void TCompSwitchPullController::onPress() {
	PxTransform Pos1 = px_actor1->getGlobalPose();

	CLogicManager::get().onSwitchPressed(CHandle(this).getOwner());
	CSoundManager::get().playEvent("PULL_SWITCH_ON", Physics.PxVec3ToXMVECTOR(Pos1.p));
}

void TCompSwitchPullController::onLeave() {
	PxTransform Pos1 = px_actor1->getGlobalPose();

	CLogicManager::get().onSwitchReleased(CHandle(this).getOwner());
	CSoundManager::get().playEvent("PULL_SWITCH_OFF", Physics.PxVec3ToXMVECTOR(Pos1.p));
}