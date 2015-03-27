#ifndef INC_COMP_TRIGGER_H_
#define INC_COMP_TRIGGER_H_

#include "base_component.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "entity_manager.h"
#include "font\font.h"

struct TCompTrigger : TBaseComponent {
private:
	CHandle m_transform;
	CHandle m_aabb;
	vector<CEntity*> inside;
public:

    TCompTrigger(){}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
	}

	void init() {
		CEntity* e = CHandle(this).getOwner();
		m_transform = e->get<TCompTransform>();
		TCompTransform* transform = (TCompTransform*)m_transform;

		assert(transform || fatal("TCompTrigger requieres a transform componenet"));

		m_aabb = e->get<TCompAABB>();
		TCompAABB* aabb = (TCompAABB*)m_aabb;

		assert(aabb || fatal("TCompTrigger requieres a AABB component"));

	}

	void update(float elapsed) {
		TCompAABB* aabb = (TCompAABB*)m_aabb;
		TCompTransform* transform = (TCompTransform*)m_transform;

		onEnter();
		onExit();
	}


	std::string toString() {
		return "Trigger";
	}

	bool onEnter(){
		for (int i = 0; i < CEntityManager::get().getEntities().size(); ++i){
			TCompAABB* aabb = (TCompAABB*)m_aabb;
			CEntity* e = (CEntity*)CEntityManager::get().getEntities()[i];
			string nameOther = e->getName();
			if (e->has<TCompAABB>()){
				if (!checkIfInside(e)){
					TCompAABB* i_aabb = e->get<TCompAABB>();
					CEntity* own = CHandle(this).getOwner();
					if ((aabb->intersects(i_aabb) && (std::strcmp(e->tag, "level") != 0) && (e != own))){
						inside.push_back(e);
						CEntity* own = CHandle(this).getOwner();
						XDEBUG("On enter: %s", e->getName());
						//own->sendMsg(TVictoryCondition(e));
						return true;
					}
				}
			}
		}
		return false;
	}

	bool onExit(){
		if (inside.size() > 0){
			for (std::vector<CEntity*>::size_type i = 0; i != inside.size(); i++) {
				TCompAABB* aabb = (TCompAABB*)m_aabb;
				CEntity* e = (CEntity*)inside[i];
				if (e->has<TCompAABB>()){
					TCompAABB* i_aabb = e->get<TCompAABB>();
					CEntity* own = CHandle(this).getOwner();
					if ((!aabb->intersects(i_aabb) && (std::strcmp(e->tag, "level") != 0) && (e != own))){
						remove(inside, i);
						XDEBUG("On exit: %s", e->getName());
						return true;
					}
				}
			}
		}
		return false;
	}

	bool checkIfInside(CEntity* entity){
		for (std::vector<CEntity*>::size_type i = 0; i != inside.size(); i++) {
			if (inside[i] == entity){
				return true;
			}
		}
		return false;
	}

	void remove(vector<CEntity*>& vec, size_t pos)
	{
		std::vector<CEntity*>::iterator it = vec.begin();
		std::advance(it, pos);
		vec.erase(it);
	}

	void renderDebug3D() {
		std::string a = "";
		for (std::vector<CEntity*>::size_type i = 0; i < inside.size(); i++) {
			CEntity* e = (CEntity*)inside[i];
			a += e->getName();
			a += "\n";
		}

		TCompTransform* transform = (TCompTransform*)m_transform;
		font.print3D(transform->position, a.c_str());
	}
};

#endif