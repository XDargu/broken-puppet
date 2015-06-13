#include "mcv_platform.h"
#include "rope_manager.h"
#include "components\comp_rope.h"

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
			TCompRope* rope = c_rope;
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
				CEntityManager::get().remove(m_rope->joint_aux.getOwner());
				CEntityManager::get().remove(CHandle(rope).getOwner());
			}
		}
	}
}

CRope_manager& CRope_manager::get() {
	return the_rope_manager;
}
