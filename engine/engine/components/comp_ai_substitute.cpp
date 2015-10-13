#include "mcv_platform.h"
#include "comp_ai_substitute.h"


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