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
	if (strings.size() <= max_strings){
		strings.push_back(CHandle(string));
	}else{
		removeString();
	}
}

void CRope_manager::removeString(){
	if (strings.size() > 0){
		CHandle c_rope = strings.back();
		strings.pop_front();
		if (c_rope.isValid())
			CEntityManager::get().remove(c_rope.getOwner());
	}
}

void CRope_manager::clearStrings(){
	if (!strings.empty()){
		strings.clear();
		for (int i = 0; i < CEntityManager::get().getEntities().size(); ++i)
		{
			TCompRope* rope = ((CEntity*)CEntityManager::get().getEntities()[i])->get<TCompRope>();
			if (rope) {
				CEntityManager::get().remove(CHandle(rope).getOwner());
			}
		}
	}
}

CRope_manager& CRope_manager::get() {
	return the_rope_manager;
}
