#include "mcv_platform.h"
#include "comp_explosion.h"
#include "entity_manager.h"
#include "handle\app_msgs.h"
#include "comp_transform.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "comp_particle_group.h"
#include "ai\logic_manager.h"
#include "rope_manager.h"
#include "audio\sound_manager.h"

TCompExplosion::TCompExplosion(){}
TCompExplosion::~TCompExplosion(){}

void TCompExplosion::init(){
	mEntity = ((CEntity*)CHandle(this).getOwner());
	const char *mName = ((CEntity*)mEntity)->getName();
	comp_trans = assertRequiredComponent<TCompTransform>(this);

	force_threshold = 2000;
}

void TCompExplosion::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	damage = atts.getFloat("damage", 20);
	radius = atts.getFloat("radius", 6);
}

void TCompExplosion::onDetonate(const TMsgOnDetonate& msg){

	CEntityManager& entity_manager = CEntityManager::get();
	bool rope_removed = false;

	float aux_force = msg.impact_force;

	if (msg.impact_force > force_threshold){
		XMVECTOR m_pos = ((TCompTransform*)comp_trans)->position;
		CQuaterion m_rot = ((TCompTransform*)comp_trans)->rotation;

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];

			if ((e != ((CEntity*)mEntity)) && (!e->hasTag("player")) && (e->has<TCompTransform>())){
				XMVECTOR pos_e = ((TCompTransform*)e->get<TCompTransform>())->position;
				

				XMVECTOR v_distance = pos_e - m_pos;
				float distance = XMVectorGetX(XMVector3Length(v_distance));

				if (distance < radius){
					e->sendMsg(TMsgExplosion(m_pos, distance, damage));
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