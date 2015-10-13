#include "mcv_platform.h"
#include "fsm_substitute.h"
#include "../components/all_components.h"
#include "components\comp_skeleton.h"
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
	AddState("fbp_Idle", (statehandler)&fsm_substitute::Idle);
	AddState("fbp_LittleTalk", (statehandler)&fsm_substitute::LittleTalk);
	

	comp_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	((TCompSkeleton*)comp_skeleton)->follow_animation = true;
	// Reset the state
	ChangeState("fbp_Idle");
}

void fsm_substitute::Idle(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(0, true);
	}

	if (CIOStatus::get().becomesPressed(CIOStatus::P)){
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
	}
	if (state_time >= 27.9f)
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

void fsm_substitute::loopAnimationIfNotPlaying(int id, bool restart) {
	TCompSkeleton* m_skeleton = comp_skeleton;
	if (id != last_anim_id) {
		if (restart) {
			m_skeleton->resetAnimationTime();
		}
		stopAnimation(last_anim_id);
		last_anim_id = id;
		m_skeleton->loopAnimation(id);
	}
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
