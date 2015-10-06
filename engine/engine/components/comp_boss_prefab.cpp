#include "mcv_platform.h"
#include "comp_boss_prefab.h"
#include "comp_transform.h"
#include "rope_manager.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"

TCompBossPrefab::TCompBossPrefab(){}
TCompBossPrefab::~TCompBossPrefab(){}

void TCompBossPrefab::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	removing_deep = atts.getFloat("removing_deep", -10.f);

	m_trans = ((CEntity*)CHandle(this).getOwner())->get<TCompTransform>();
	m_entity = CHandle(this).getOwner();
}

void TCompBossPrefab::init(){
	int a = 2;
}

void TCompBossPrefab::update(float elapsed) {


	float aux_deep = XMVectorGetY(((TCompTransform*)m_trans)->position);
	
	if (aux_deep <= removing_deep){
		bool remove = true;

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
						if (m_entity == CHandle(actor1->userData)){
							remove = false;
						}
					}
					if (actor2){
						if (m_entity == CHandle(actor2->userData)){
							remove = false;
						}
					}
				}
			}
		}

		if (remove){
			CEntityManager::get().remove(m_entity);
		}
	}	
}