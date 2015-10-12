#include "mcv_platform.h"
#include "comp_explosion.h"
#include "entity_manager.h"
#include "handle\app_msgs.h"
#include "comp_transform.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "comp_particle_group.h"
#include "comp_skeleton.h"
#include "comp_ai_boss.h"
#include "comp_ai_boss.h"
#include "comp_needle.h"
#include "ai\logic_manager.h"
#include "item_manager.h"
#include "rope_manager.h"
#include "entity_manager.h"
#include "audio\sound_manager.h"

TCompExplosion::TCompExplosion(){}
TCompExplosion::~TCompExplosion(){}

void TCompExplosion::init(){
	mEntity = ((CEntity*)CHandle(this).getOwner());
	boss_entity = CEntityManager::get().getByName("Boss");

	if (boss_entity.isValid())
		boss_trans = ((CEntity*)boss_entity)->get<TCompTransform>();

	const char *mName = ((CEntity*)mEntity)->getName();
	comp_trans = assertRequiredComponent<TCompTransform>(this);

	force_threshold = 10;
	count_down = 0;

}

void TCompExplosion::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	damage = atts.getFloat("damage", 20);
	radius = atts.getFloat("radius", 6);
	bomb_active = atts.getBool("active", false);
	just_boss = atts.getBool("just_boss", false);
	auto_stun = atts.getBool("auto_stun", false);
}

void TCompExplosion::update(float elapsed){
	if (!bomb_active){
		count_down += elapsed;
		if (count_down >= 10.f){
			bomb_active = true;
		}
	}
	// auto bomb, follow in plane, when is near head explote and stun;
	if (auto_stun){
		/**/
		if (boss_entity.isValid() && boss_trans.isValid() && comp_trans.isValid()){

			// check head position: Bone 06
			TCompSkeleton* skeleton = ((CEntity*)boss_entity)->get<TCompSkeleton>();
			if (skeleton){
				// Follow in plane				
				XMVECTOR bomb_pos = ((TCompTransform*)comp_trans)->position;
				XMVECTOR boss_front = ((TCompTransform*)boss_trans)->getFront();
				XMVECTOR head_pos = skeleton->getPositionOfBone(6) + boss_front * 10;
				XMVECTOR aux_head_pos = XMVectorSetY(head_pos, XMVectorGetY(bomb_pos));												

				//((TCompTransform*)comp_trans)->position = aux_head_pos;

				float bomb_pos_y = XMVectorGetY(bomb_pos);
				float head_pos_y = XMVectorGetY(head_pos);
				
				// is near the head
				if (bomb_pos_y <= head_pos_y){
					// make the bomb explode	
					Explote(true);
					// make a head hit
					CHandle comp_ai_boss = ((CEntity*)boss_entity)->get<TCompAiBoss>();
					if (comp_ai_boss.isValid()){
						((TCompAiBoss*)comp_ai_boss)->stun();
					}
				}				
			}
		}
		/**/
	}
}


void TCompExplosion::onDetonate(const TMsgOnDetonate& msg){

	if ((mEntity.isValid()) && (comp_trans.isValid())){
		
		float aux_force = msg.impact_force;

 		bool boss_check = true;
		if (just_boss){
			boss_check = msg.is_boss;
		}

		if ((boss_check)){
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

		TCompDistanceJoint* mJoint = nullptr;
		PxDistanceJoint* px_joint = nullptr;
		PxRigidActor* actor1 = nullptr;
		PxRigidActor* actor2 = nullptr;
		TCompRope* rope = nullptr;

		std::vector<CHandle> target_ropes;

		for (auto& string : CRope_manager::get().getStrings()) {
			rope = string;
			if (rope) {
				mJoint = rope->joint;

				if (mJoint){
					px_joint = mJoint->joint;
					px_joint->getActors(actor1, actor2);

					if (actor1)	{
						if ((CHandle)mEntity == CHandle(actor1->userData)){
							target_ropes.push_back(CHandle(rope));
						}
					}

					if (actor2){
						if ((CHandle)mEntity == CHandle(actor2->userData)){
							target_ropes.push_back(CHandle(rope));
						}
					}
				}
			}
		}

		for (auto& it : target_ropes) {
			if (it.isValid()) {
				rope = it;
				rope_manager.removeString(it);

				// Remove needles
				CHandle needle1 = rope->transform_1_aux;
				if (needle1.isValid()) {
					CEntity* e1 = CHandle(needle1).getOwner();
					if (e1) {
						CHandle c_needle1 = e1->get<TCompNeedle>();

						if (c_needle1.isValid()){
							Citem_manager::get().removeNeedleFromVector(c_needle1);
							Citem_manager::get().removeNeedle(c_needle1);
							CEntityManager::get().remove(CHandle(needle1).getOwner());
						}
					}
				}

				CHandle needle2 = rope->transform_2_aux;
				if (needle2.isValid()){
					CEntity* e2 = CHandle(needle2).getOwner();
					if (e2) {
						CHandle c_needle2 = e2->get<TCompNeedle>();
						if (c_needle2.isValid()){
							Citem_manager::get().removeNeedleFromVector(c_needle2);
							Citem_manager::get().removeNeedle(c_needle2);
							CEntityManager::get().remove(CHandle(needle2).getOwner());
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
		CHandle particle_entity = CLogicManager::get().instantiateParticleGroup("ps_explosion_bomb_big", m_pos, m_rot);

		if (particle_entity.isValid()) {
			TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
			pg->destroy_on_death = true;
			//pg->restart();
		}
	}	
}