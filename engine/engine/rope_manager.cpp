#include "mcv_platform.h"
#include "rope_manager.h"
#include "components\comp_rope.h"
#include "ai\logic_manager.h"
#include "audio\sound_manager.h"

static CRope_manager the_rope_manager;

CRope_manager::CRope_manager()
{
}


CRope_manager::~CRope_manager()
{
}

void CRope_manager::addString(CHandle string){
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
					{ "distance", distance }
				};

				CSoundManager::get().playEvent("event:/Strings/stringCancel", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
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
		strings.clear();
		for (int i = 0; i < CEntityManager::get().getEntities().size(); ++i)
		{
			TCompRope* rope = ((CEntity*)CEntityManager::get().getEntities()[i])->get<TCompRope>();
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
					{ "distance", distance }
				};

				CSoundManager::get().playEvent("event:/Strings/stringCancel", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));

				//CSoundManager::get().playEvent("event:/Strings/stringEvents", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
			}
		}
	}
}

CRope_manager& CRope_manager::get() {
	return the_rope_manager;
}
