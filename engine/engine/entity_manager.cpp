#include "mcv_platform.h"
#include "entity_manager.h"
#include "handle\handle.h"

using namespace DirectX;

static CEntityManager entity_manager;

CEntityManager& CEntityManager::get() {
	return entity_manager;
}

void CEntityManager::add(CHandle the_entity) {
	entity_event_count++;
	entities.push_back(the_entity);
}

bool CEntityManager::remove(CHandle the_handle) {
	// If the entity is already in the destroy list, do not add it
	auto it = std::find(handles_to_destroy.begin(), handles_to_destroy.end(), the_handle);
	if (it == handles_to_destroy.end() && the_handle.isValid()) {
		handles_to_destroy.push_back(the_handle);
	}
	return true;
}

void CEntityManager::destroyRemovedHandles() {

	for (auto& it : handles_to_destroy) {
		if (getObjManager<CEntity>()->getType() == it.getType()) { 
			// The handle is an entity
			auto it2 = std::find(entities.begin(), entities.end(), it);			
			if (it2 == entities.end()) { fatal(" Trying to destroy an entity %s not registered in the entity manager", ((CEntity*)it)->getName());  }
			entities.erase(it2);
			entity_event_count++;
		}
		it.destroy();
	};	

	handles_to_destroy.clear();
}

bool CEntityManager::clear() {

	for (int i = 0; i < entities.size(); ++i) {
		((CEntity*)entities[i])->destroyComponents();
		entities[i].destroy();
		entity_event_count++;
	}

	entity_event_count = 0;
	entities.clear();
	return true;
}

CHandle CEntityManager::getByName(const char *name) {

	for (auto& it : entities) {
		if (strcmp(((CEntity*)it)->getName(), name) == 0)
			return it;
	};
	return CHandle();
}

CHandle CEntityManager::createEmptyEntity() {
	CHandle the_entity = getObjManager<CEntity>()->createObj();
	entities.push_back(the_entity);
	entity_event_count++;
	return the_entity;
}

unsigned int CEntityManager::getEntityEventCount() {
	return entity_event_count;
}

TEntityChunk* CEntityManager::getCurrentRoom(XMVECTOR position) {
	for (auto& it : entity_chunks) {
		if (it.bounds.containts(position))
			return &it;
	};
	return nullptr;
}

std::vector< CHandle > CEntityManager::getCurrentRoomEntities(XMVECTOR position) {
	for (auto& it : entity_chunks) {
		if (it.bounds.containts(position))
			return it.entities;
	};

	return std::vector< CHandle >();
}

bool CEntityManager::removeFromChunk(CHandle the_handle) {
	return false;
}

TEntityChunk* CEntityManager::addChunk(char name[32], XMVECTOR min, XMVECTOR max) {
	TEntityChunk new_chunk(name, min, max);
	entity_chunks.push_back(new_chunk);

	return &new_chunk;
}
