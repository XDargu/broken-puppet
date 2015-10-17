#include "mcv_platform.h"
#include "rope_manager.h"
#include "components\comp_rope.h"
#include "components\comp_needle.h"
#include "components\comp_distance_joint.h"
#include "ai\logic_manager.h"
#include "audio\sound_manager.h"
#include "item_manager.h"

static CRope_manager the_rope_manager;

CRope_manager::CRope_manager()
{
}


CRope_manager::~CRope_manager()
{
	strings.clear();
}

void CRope_manager::addString(CHandle string){
#ifndef FINAL_RELEASE
	CApp::get().rope_thrown = true;
#endif

	strings.push_back(CHandle(string));
	if (strings.size() > max_strings){
		removeString();
	}
}

void CRope_manager::removeBackString(){
	if (strings.size() > 0){
		CHandle c_rope = strings.back();
		strings.pop_back();
		if (c_rope.isValid()) {
			//CSoundManager::SoundParameter params[] = {
				//{ "type", 3 }
			//};

			//CSoundManager::get().playEvent("event:/Strings/stringEvents", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
			TCompRope* rope = c_rope;
			if (rope){
				float distance = 0.f;
				distance = V3DISTANCE(rope->pos_1, rope->pos_2);

				CSoundManager::SoundParameter params[] = {
					{ "LongCuerda", distance }
				};

				CSoundManager::get().playEvent("STRING_CANCEL", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
			}

			
			if (rope->joint_aux.isValid())
				CEntityManager::get().remove(rope->joint_aux.getOwner());

			CEntityManager::get().remove(c_rope.getOwner());
		}
	}
}

void CRope_manager::removeString(){
	if (strings.size() > 0){
		CHandle c_rope = strings.front();
		strings.pop_front();
		if (c_rope.isValid()) {
			TCompRope* rope = c_rope;
			
			if (rope->joint_aux.isValid())
			CEntityManager::get().remove(rope->joint_aux.getOwner());

			CEntityManager::get().remove(c_rope.getOwner());
		}
	}
}

void CRope_manager::removeString(CHandle rope){
	if (!strings.empty()){
		//TCompRope* rope_to_remove = (TCompRope*)rope;
		if (rope.isValid()){
			strings.erase(std::remove(strings.begin(), strings.end(), rope), strings.end());
			TCompRope* m_rope = rope;

			if (m_rope->joint_aux.isValid())
				CEntityManager::get().remove(m_rope->joint_aux.getOwner());

			CEntityManager::get().remove(rope.getOwner());
		}
	}
}

void CRope_manager::clearStrings(){
	if (!strings.empty()){
		
		for (auto string : strings)
		{
			TCompRope* rope = string;
			if (rope) {
				TCompRope* m_rope = rope;
				
				if (m_rope->joint_aux.isValid())
					CEntityManager::get().remove(m_rope->joint_aux.getOwner());

				CEntityManager::get().remove(CHandle(rope).getOwner());
				//CSoundManager::SoundParameter params[] = {
					//{ "type", 3 }
				//};
				float distance = 0.f;
				distance = V3DISTANCE(rope->pos_1, rope->pos_2);

				CSoundManager::SoundParameter params[] = {
					{ "LongCuerda", distance }
				};

				CSoundManager::get().playEvent("STRING_CANCEL", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));

				//CSoundManager::get().playEvent("event:/Strings/stringEvents", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
			}
		}

		strings.clear();
	}
}

CRope_manager& CRope_manager::get() {
	return the_rope_manager;
}


void CRope_manager::removeRopesTiedToObject(CHandle entity) {
	// Remove rope
	TCompDistanceJoint* mJoint = nullptr;
	PxDistanceJoint* px_joint = nullptr;
	PxRigidActor* actor1 = nullptr;
	PxRigidActor* actor2 = nullptr;
	TCompRope* rope = nullptr;

	std::vector<CHandle> target_ropes;

	for (auto& string : strings) {
		rope = string;
		if (rope) {
			mJoint = rope->joint;

			if (mJoint){
				px_joint = mJoint->joint;
				px_joint->getActors(actor1, actor2);

				if (actor1)	{
					if (entity == CHandle(actor1->userData)){
						target_ropes.push_back(CHandle(rope));
					}
				}

				if (actor2){
					if (entity == CHandle(actor2->userData)){
						target_ropes.push_back(CHandle(rope));
					}
				}
			}
		}
	}

	for (auto& it : target_ropes) {
		if (it.isValid()) {
			rope = it;
			removeString(it);

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
}

void CRope_manager::removeJointTiedToObject(CHandle entity) {
	// Remove rope
	TCompDistanceJoint* mJoint = nullptr;
	PxDistanceJoint* px_joint = nullptr;
	PxRigidActor* actor1 = nullptr;
	PxRigidActor* actor2 = nullptr;
	TCompRope* rope = nullptr;

	for (auto& string : strings) {
		rope = string;
		if (rope) {
			mJoint = rope->joint;

			if (mJoint){
				px_joint = mJoint->joint;
				px_joint->getActors(actor1, actor2);

				if (actor1)	{
					if (entity == CHandle(actor1->userData)){
						rope->releaseJoint();
					}
				}

				if (actor2){
					if (entity == CHandle(actor2->userData)){
						rope->releaseJoint();
					}
				}
			}
		}
	}
}