#include "mcv_platform.h"
#include "comp_hitch.h"
#include "entity_manager.h"
#include "handle\app_msgs.h"
#include "comp_ai_boss.h"

TCompHitch::TCompHitch(){}
TCompHitch::~TCompHitch(){}

void TCompHitch::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	is_heart = atts.getBool("is_heart", false);
}

void TCompHitch::init(){
	m_boss = CEntityManager::get().getByName("Boss");
	
}

void TCompHitch::onRopeTensed(const TMsgRopeTensed& msg){
	m_boss = CEntityManager::get().getByName("Boss");
	if (m_boss.isValid()){
		TCompAiBoss* ai_boss = ((CEntity*)m_boss)->get<TCompAiBoss>();
		if (ai_boss){
			if (!is_heart){
				ai_boss->breakHitch(CHandle(this).getOwner());
			}	
		}		
	}	

}
