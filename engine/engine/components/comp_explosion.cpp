#include "mcv_platform.h"
#include "comp_explosion.h"
#include "entity_manager.h"
#include "handle\app_msgs.h"
#include "comp_transform.h"

void TCompExplosion::init(){
	mEntity = ((CEntity*)CHandle(this).getOwner());
	const char *mName = ((CEntity*)mEntity)->getName();
	comp_trans = assertRequiredComponent<TCompTransform>(this);

	force_threshold = 2000;
}

void TCompExplosion::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	damage = atts.getFloat("damage", 0);
	radius = atts.getFloat("radius", 10);
}

void TCompExplosion::onDetonate(const TMsgOnDetonate& msg){

	CEntityManager& entity_manager = CEntityManager::get();

	float aux_force = msg.impact_force;

	for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
		CEntity* e = entity_manager.rigid_list[i];
		if (e->has<TCompTransform>()){
			XMVECTOR pos_e = ((TCompTransform*)e->get<TCompTransform>())->position;
			XMVECTOR m_pos = ((TCompTransform*)comp_trans)->position;

			XMVECTOR v_distance = pos_e - m_pos;
			float distance = XMVectorGetX(XMVector3Length(v_distance));

			if (distance < radius){
				e->sendMsg(TMsgExplosion(m_pos, distance, damage));
			}			
		}
	}
	
}