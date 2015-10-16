#include "mcv_platform.h"
#include "fsm_substitute.h"
#include "../components/all_components.h"
#include "components\comp_skeleton.h"
#include "components\comp_ragdoll.h"
#include "components\comp_player_controller.h"
#include "components\comp_skeleton_lookat.h"
#include "components\comp_audio_source.h"
#include "handle\prefabs_manager.h"
#include "io\iostatus.h"

using namespace DirectX;

fsm_substitute::fsm_substitute()
{
	last_loop = 0;
	last_loop_delay = 0.f;
	last_conversation = 0;

	conversation_list.push_back("SUBS_HANGED_16");
	conversation_list.push_back("SUBS_HANGED_17");
	conversation_list.push_back("SUBS_HANGED_18");
	conversation_list.push_back("SUBS_HANGED_19");
	conversation_list.push_back("SUBS_HANGED_20");
	conversation_list.push_back("SUBS_HANGED_21");
	conversation_list.push_back("SUBS_HANGED_22");
	conversation_list.push_back("SUBS_HANGED_23");
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
	AddState("fbp_LoopTalk8", (statehandler)&fsm_substitute::LoopTalk8);
	AddState("fbp_LoopTalk9", (statehandler)&fsm_substitute::LoopTalk9);
	AddState("fbp_JustHanged", (statehandler)&fsm_substitute::JustHanged);
	AddState("fbp_CallingBoss", (statehandler)&fsm_substitute::CallingBoss);
	AddState("fbp_Hanged", (statehandler)&fsm_substitute::Hanged);	
	AddState("fbp_JustTied", (statehandler)&fsm_substitute::JustTied);
	AddState("fbp_TiedLoop", (statehandler)&fsm_substitute::TiedLoop);
	

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
		if (skeleton)
			skeleton->setFollowAnimation(false);
	}

	last_loop_delay += elapsed;
	if (last_loop_delay > 3){
		last_loop_delay = 0;

		int loop = calculateLoop();
		switch (loop)
		{
		case 0:
			ChangeState("fbp_LoopTalk8");
			break;

		case 1:
			ChangeState("fbp_LoopTalk9");
			break;

		default:
			break;
		}
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

		XMVECTOR sound_pos = XMVectorSet(0, 0, 0, 0);
		if (trans)
			sound_pos = trans->position;

		CSoundManager::get().playEvent("SUBS_SPEECH", sound_pos);
		CLogicManager::get().playSubtitles("SUBS_SPEECH");
	}
	if (state_time >= 29.15f){	
		ChangeState("fbp_Idle");
	}		
}

void fsm_substitute::LoopTalk8(){
	if (on_enter){
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		TCompSkeleton* skeleton = comp_skeleton;

		// Little Talk animation
		stopAllAnimations();
		skeleton->playAnimation(3);
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(true);

		XMVECTOR sound_pos = XMVectorSet(0, 0, 0, 0);
		if (trans)
			sound_pos = trans->position;

		CSoundManager::get().playEvent("SUBS_WAIT_LOOP_8", sound_pos);
		CLogicManager::get().playSubtitles("SUBS_WAIT_LOOP_8");
	}
	if (state_time >= 1.9){
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(false);
		ChangeState("fbp_Idle");
	}
}

void fsm_substitute::LoopTalk9(){
	if (on_enter){
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		TCompSkeleton* skeleton = comp_skeleton;

		// Little Talk animation
		stopAllAnimations();
		skeleton->playAnimation(4);
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(true);

		XMVECTOR sound_pos = XMVectorSet(0, 0, 0, 0);
		if (trans)
			sound_pos = trans->position;

		CSoundManager::get().playEvent("SUBS_WAIT_LOOP_9", sound_pos);
		CLogicManager::get().playSubtitles("SUBS_WAIT_LOOP_9");

	}
	if (state_time >= 1.9){
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(false);
		ChangeState("fbp_Idle");
	}
}

void fsm_substitute::JustHanged(){
	if (on_enter){
		// Get the audioSource and play sound
		TCompAudioSource* audio_source = ((CEntity*)entity)->get<TCompAudioSource>();
		if (audio_source)
			audio_source->play();
	}	
	if (state_time >= 2.5f){
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(false);
		ChangeState("fbp_CallingBoss");
	}
}

void fsm_substitute::Hanged(float elapsed){

	if (on_enter){
		last_loop_delay = 0;
	}

	last_loop_delay += elapsed;
	if (last_loop_delay > 6){
		last_loop_delay = 0;

		// Taking sound position
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		XMVECTOR sound_pos = XMVectorSet(0, 0, 0, 0);
		if (trans)
			sound_pos = trans->position;
		
		// Play conversation
		int conversation = calculateConversation();
		CSoundManager::get().playEvent(conversation_list[conversation], sound_pos);
		CLogicManager::get().playSubtitles(conversation_list[conversation]);
	}
}

void fsm_substitute::CallingBoss(){

	if (on_enter){
		// Taking sound position
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		XMVECTOR sound_pos = XMVectorSet(0, 0, 0, 0);
		if (trans)
			sound_pos = trans->position;
		CSoundManager::get().playEvent("SUBS_PRE_BOSS", sound_pos);
		CLogicManager::get().playSubtitles("SUBS_PRE_BOSS");
		CLogicManager::get().onSubstituteHang();
	}
	if (state_time >= 3){
		((TCompSkeleton*)comp_skeleton)->setFollowAnimation(false);
		ChangeState("fbp_Hanged");
	}
}

void fsm_substitute::JustTied(){
	if (on_enter){
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		TCompSkeleton* skeleton = comp_skeleton;
		// Tied Talk animation
		stopAllAnimations();
		skeleton->playAnimation(5);

		XMVECTOR sound_pos = XMVectorSet(0, 0, 0, 0);
		if (trans)
			sound_pos = trans->position;
		CSoundManager::get().playEvent("SUBS_ROPE_THROWN", sound_pos);
		CLogicManager::get().playSubtitles("SUBS_ROPE_THROWN");
	}
	if (state_time >= 3.9){
		ChangeState("fbp_TiedLoop");
	}
}

void fsm_substitute::TiedLoop(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimation(6, true);
	}
}

int fsm_substitute::calculateConversation() {

	int next_conversation = last_conversation;
	last_conversation++;

	if (last_conversation >= conversation_list.size()){
		last_conversation = 0;
	}

	return next_conversation;
}

int fsm_substitute::calculateLoop() {
	
	int next_loop = last_loop;
	last_loop++;

	if (last_loop >= 2){		
		last_loop = 0;
	}
		
	return next_loop;
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
