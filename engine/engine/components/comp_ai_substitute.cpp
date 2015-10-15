#include "mcv_platform.h"
#include "comp_ai_substitute.h"
#include "comp_skeleton.h"
#include "comp_ragdoll.h"
#include "ai\logic_manager.h"

TCompSubstituteBoss::TCompSubstituteBoss() {
	m_fsm_substitute = new fsm_substitute;
}

TCompSubstituteBoss::~TCompSubstituteBoss() {
	SAFE_DELETE(m_fsm_substitute);
}

void TCompSubstituteBoss::loadFromAtts(const std::string& elem, MKeyValue &atts){

};

void TCompSubstituteBoss::init(){
	m_fsm_substitute->entity = CHandle(this).getOwner();
	m_fsm_substitute->init();
}

void TCompSubstituteBoss::update(float elapsed){
	m_fsm_substitute->update(elapsed);
}

void TCompSubstituteBoss::onRopeTensed(const TMsgRopeTensed& msg) {
	TCompRagdoll* comp_ragdoll = getSibling<TCompRagdoll>(this);
	comp_ragdoll->setActive(true);
	m_fsm_substitute->ChangeState("fbp_JustHanged");
	CLogicManager::get().onSubstituteHang();
}
void TCompSubstituteBoss::initLittleTalk(){
	m_fsm_substitute->ChangeState("fbp_LittleTalk");
}