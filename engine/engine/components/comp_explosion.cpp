#include "mcv_platform.h"
#include "comp_explosion.h"
#include "entity_manager.h"
#include "handle\app_msgs.h"
#include "comp_transform.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "comp_particle_group.h"
#include "comp_ai_boss.h"
#include "ai\logic_manager.h"
#include "item_manager.h"
#include "rope_manager.h"
#include "audio\sound_manager.h"

TCompExplosion::TCompExplosion(){}
TCompExplosion::~TCompExplosion(){}

void TCompExplosion::init(){
	mEntity = ((CEntity*)CHandle(this).getOwner());
	const char *mName = ((CEntity*)mEntity)->getName();
	comp_trans = assertRequiredComponent<TCompTransform>(this);

	force_threshold = 2000;
	count_down = 0;
	
}

void TCompExplosion::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	damage = atts.getFloat("damage", 20);
	radius = atts.getFloat("radius", 6);
	bomb_active = atts.getBool("active", false);
	just_boss = atts.getBool("just_boss", false);
}

void TCompExplosion::update(float elapsed){
	if (!bomb_active){
		count_down += elapsed;
		if (count_down >= 10.f){
			bomb_active = true;
		}
	}
}


void TCompExplosion::onDetonate(const TMsgOnDetonate& msg){

	if ((mEntity.isValid()) && (comp_trans.isValid())){
		
		float aux_force = msg.impact_force;

 		bool boss_check = true;
		if (just_boss){
			boss_check = msg.is_boss;
		}

		if ((msg.impact_force > force_threshold)&&(boss_check)){
			Explote(false);
		}

	}
}

void TCompExplosion::Explote(bool force_explosion){
	if ((force_explosion) || (bomb_active)){
		XMVECTOR m_pos = ((TCompTransform*)comp_trans)->position;
		CQuaterion m_rot = ((TCompTransform*)comp_trans)->rotation;
		CEntityManager& entity_manager = CEntityManager::get();

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];

			if ((e != ((CEntity*)mEntity)) && (!e->hasTag("player")) && (e->has<TCompTransform>())){
				XMVECTOR pos_e = ((TCompTransform*)e->get<TCompTransform>())->position;

				XMVECTOR v_distance = pos_e - m_pos;
				float distance = XMVectorGetX(XMVector3Length(v_distance));

				if (distance < radius){
					e->sendMsg(TMsgExplosion(m_pos, distance, damage));
				}
				CHandle m_boss = entity_manager.getByName("Boss");
				if (m_boss.isValid()){
					CHandle comp_boss = ((CEntity*)m_boss)->get<TCompAiBoss>();
					if (comp_boss.isValid()){
						((TCompAiBoss*)comp_boss)->stun();
					}
				}
			}
		}


		// Remove rope
		CRope_manager& rope_manager = CRope_manager::get();

		for (auto& string : CRope_manager::get().getStrings()) {
			TCompRope* rope = string;
			if (rope) {
				TCompDistanceJoint* mJoint = rope->joint;
				if (mJoint){
					PxDistanceJoint* px_joint = mJoint->joint;
					PxRigidActor* actor1;
					PxRigidActor* actor2;
					px_joint->getActors(actor1, actor2);

					if (actor1)	{
						if (mEntity == CHandle(actor1->userData)){
							rope_manager.removeString(string);
							// Remove needles
							CHandle needle1 = rope->transform_1_aux;
							CHandle needle2 = rope->transform_2_aux;
							if (needle1.isValid()){
								Citem_manager::get().removeNeedleFromVector(CHandle(needle1).getOwner());
								Citem_manager::get().removeNeedle(CHandle(needle1).getOwner());
								CEntityManager::get().remove(CHandle(needle1).getOwner());
							}
							if (needle2.isValid()){
								Citem_manager::get().removeNeedleFromVector(CHandle(needle2).getOwner());
								Citem_manager::get().removeNeedle(CHandle(needle2).getOwner());
								CEntityManager::get().remove(CHandle(needle2).getOwner());
							}
						}
					}
					if (actor2){
						if (mEntity == CHandle(actor2->userData)){
							rope_manager.removeString(string);
						}
					}
				}
			}
		}

		// Remove Entity
		CEntityManager::get().remove(mEntity);

		// Play Explosion sound
		CSoundManager::get().playEvent("event:/test_event", m_pos);

		// Adding particle sistem
		CHandle particle_entity = CLogicManager::get().instantiateParticleGroup("ps_prota_jump_ring", m_pos, m_rot);

		if (particle_entity.isValid()) {
			TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
			pg->destroy_on_death = true;
			if (pg->particle_systems->size() > 0)
			{
				(*pg->particle_systems)[0].emitter_generation->inner_radius = radius / 2.f;
				(*pg->particle_systems)[0].emitter_generation->radius = radius;
			}
		}
	}	
}