#include "mcv_platform.h"
#include "comp_trigger.h"
#include "entity_manager.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "font\font.h"
#include "ai\logic_manager.h"
#include "ai\aimanager.h"

TCompTrigger::~TCompTrigger() {
	CLogicManager::get().unregisterTrigger(CHandle(this));
	delete inside;
}

TCompTrigger::TCompTrigger() : m_transform(CHandle()), m_aabb(CHandle()), player_only(false), bots_only(false) {
	inside = new std::vector<CHandle>();
}

void TCompTrigger::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	player_only = atts.getBool("playerOnly", false);
	bots_only = atts.getBool("botsOnly", false);
	if (bots_only)
		player_only = true;

	TCompTransform* transform = (TCompTransform*)m_transform;
	TCompAABB* aabb = (TCompAABB*)m_aabb;

	CLogicManager::get().registerTrigger(CHandle(this));

	first_enter = false;
}

void TCompTrigger::init() {
	player_entity = CEntityManager::get().getByName("Player");
}

void TCompTrigger::update(float elapsed) {
	TCompAABB* aabb = (TCompAABB*)m_aabb;
	TCompTransform* transform = (TCompTransform*)m_transform;

	onEnter();
	onExit();
}

bool TCompTrigger::onEnter(){

	if (player_only) {
		TCompAABB* aabb = (TCompAABB*)m_aabb;
		CEntity* e = player_entity;
		if (e->has<TCompAABB>()){
			if (!checkIfInside(e)){
				TCompAABB* i_aabb = e->get<TCompAABB>();
				CEntity* own = CHandle(this).getOwner();
				if ((e != own) && (aabb->intersects(i_aabb) && (std::strcmp(e->tag, "level") != 0))){
					inside->push_back(e);
					CLogicManager::get().onTriggerEnter(CHandle(CHandle(this).getOwner()), CHandle(e));
					//XDEBUG("On enter: %s", e->getName());
					if (!first_enter) {						
						CLogicManager::get().onTriggerFirstEnter(CHandle(CHandle(this).getOwner()), CHandle(e));
						first_enter = true;
					}
					return true;
				}
			}
		}
	}

	if (bots_only) {
		
		aimanager& ai_manager = aimanager::get();

		for (int i = 0; i < ai_manager.bots.size(); ++i){
			TCompAABB* aabb = (TCompAABB*)m_aabb;
			CEntity* e = (CEntity*)ai_manager.bots[i]->GetEntity();
			if (e->has<TCompAABB>()){
				if (!checkIfInside(e)){
					TCompAABB* i_aabb = e->get<TCompAABB>();
					CEntity* own = CHandle(this).getOwner();
					if ((e != own) && (aabb->intersects(i_aabb) && (std::strcmp(e->tag, "level") != 0))){
						inside->push_back(e);
						CLogicManager::get().onTriggerEnter(CHandle(CHandle(this).getOwner()), CHandle(e));
						//XDEBUG("On enter: %s", e->getName());
						if (!first_enter) {
							CLogicManager::get().onTriggerFirstEnter(CHandle(CHandle(this).getOwner()), CHandle(e));
							first_enter = true;
						}
						return true;
					}
				}
			}
		}
	}

	if (!(player_only || bots_only)) {

		CEntityManager& entity_manager = CEntityManager::get();

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			TCompAABB* aabb = (TCompAABB*)m_aabb;
			CEntity* e = entity_manager.rigid_list[i];
			if (e->has<TCompAABB>()){
				if (!checkIfInside(e)){
					TCompAABB* i_aabb = e->get<TCompAABB>();
					CEntity* own = CHandle(this).getOwner();
					if ((e != own) && (aabb->intersects(i_aabb) && (std::strcmp(e->tag, "level") != 0))){
						inside->push_back(e);
						CLogicManager::get().onTriggerEnter(CHandle(CHandle(this).getOwner()), CHandle(e));
						//XDEBUG("On enter: %s", e->getName());
						if (!first_enter) {
							CLogicManager::get().onTriggerFirstEnter(CHandle(CHandle(this).getOwner()), CHandle(e));
							first_enter = true;
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool TCompTrigger::onExit(){
	if (inside->size() > 0){
		for (std::vector<CEntity*>::size_type i = 0; i != inside->size(); i++) {
			TCompAABB* aabb = (TCompAABB*)m_aabb;
			CEntity* e = (*inside)[i];
			if (e->has<TCompAABB>()){
				TCompAABB* i_aabb = e->get<TCompAABB>();
				CEntity* own = CHandle(this).getOwner();
				if ((e != own) && (!aabb->intersects(i_aabb) && (std::strcmp(e->tag, "level") != 0))){
					remove(*inside, i);
					CLogicManager::get().onTriggerExit(CHandle(CHandle(this).getOwner()), CHandle(e));
					//XDEBUG("On exit: %s", e->getName());
					return true;
				}
			}
		}
	}
	return false;
}

bool TCompTrigger::checkIfInside(CHandle entity){
	for (std::vector<CEntity*>::size_type i = 0; i != inside->size(); ++i) {
		if ((*inside)[i] == entity) {
			return true;
		}
	}
	return false;
	/*auto it = std::find(inside.begin(), inside.end(), entity);
	return it != inside.end();*/
}

void TCompTrigger::remove(std::vector<CHandle>& vec, size_t pos)
{
	std::vector<CHandle>::iterator it = vec.begin();
	std::advance(it, pos);
	vec.erase(it);
}

void TCompTrigger::renderDebug3D() {
	std::string a = "";
	for (std::vector<CHandle>::size_type i = 0; i < inside->size(); i++) {
		CEntity* e = (CEntity*)(*inside)[i];
		a += e->getName();
		a += "\n";
	}

	TCompTransform* transform = (TCompTransform*)m_transform;
	font.print3D(transform->position, a.c_str());
}