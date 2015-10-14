#include "mcv_platform.h"
#include "fsm_substitute.h"
#include "../components/all_components.h"
#include "components\comp_skeleton.h"
#include "components\comp_ragdoll.h"
#include "components\comp_player_controller.h"
#include "components\comp_skeleton_lookat.h"
#include "handle\prefabs_manager.h"
#include "io\iostatus.h"

using namespace DirectX;

fsm_substitute::fsm_substitute()
{
}

fsm_substitute::~fsm_substitute()
{
}

void fsm_substitute::init()
{
	// Insert all states in the map
	AddState("fbp_IdleSit", (statehandler)&fsm_substitute::IdleSit);
	AddState("fbp_Idle", (statehandler)&fsm_substitute::Idle);
	AddState("fbp_LittleTalk", (statehandler)&fsm_substitute::LittleTalk);
	

	comp_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	comp_ragdoll = ((CEntity*)entity)->get<TCompRagdoll>();
	((TCompSkeleton*)comp_skeleton)->setFollowAnimation(true);
	// Reset the state
	ChangeState("fbp_IdleSit");
	//ChangeState("fbp_Idle");
}

void fsm_substitute::IdleSit(float elapsed){
	if (on_enter){
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(false);
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimation(2, true);
	}

	if (CIOStatus::get().becomesPressed(CIOStatus::P)){
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->setActive(false);
		ChangeState("fbp_LittleTalk");
	}
}

void fsm_substitute::Idle(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimation(1, true);
		
	}

	if (CIOStatus::get().becomesPressed(CIOStatus::P)){
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->setActive(false);
		ChangeState("fbp_LittleTalk");
	}
}

void fsm_substitute::LittleTalk(float elapsed){
	if (on_enter){
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		TCompSkeleton* skeleton = comp_skeleton;

		// Little Talk animation
		stopAllAnimations();
		skeleton->playAnimation(0);
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(true);
	}
	if (state_time >= 27.9f)
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(false);
		ChangeState("fbp_Idle");
}



/*********************************
			ANIMACIONES
/*********************************/
void fsm_substitute::stopAllAnimations() {
	TCompSkeleton* m_skeleton = comp_skeleton;

	for (int i = 0; i < 50; ++i) {
		m_skeleton->model->getMixer()->clearCycle(i, 0.3f);
	}
}

void fsm_substitute::loopAnimation(int id, bool restart) {
	TCompSkeleton* m_skeleton = comp_skeleton;
	if (restart) {
		m_skeleton->resetAnimationTime();
	}
	m_skeleton->loopAnimation(id);	
}

void fsm_substitute::stopAnimation(int id) {
	TCompSkeleton* m_skeleton = comp_skeleton;
	m_skeleton->stopAnimation(id);
}

float fsm_substitute::getAnimationDuration(int id) {
	TCompSkeleton* m_skeleton = comp_skeleton;

	float res = m_skeleton->model->getMixer()->getAnimationDuration();
	return res;
}
