#include "mcv_platform.h"
#include "entity_manager.h"
#include "handle\handle.h"

using namespace DirectX;

CEntityOld::CEntityOld()
{
  position = XMVectorSet(0.f, 0.f, 0.f, 1.f);
  rotation = XMQuaternionIdentity();
  scale = XMVectorSet(1.f, 1.f, 1.f, 1.f);

  name[0] = 0x00;
}

CEntityOld::~CEntityOld()
{
}

XMMATRIX CEntityOld::getWorld() const {
  XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
  XMMATRIX m = XMMatrixAffineTransformation(scale, zero, rotation, position);
  return m;
}

XMVECTOR CEntityOld::getFront() const {
  XMMATRIX m = XMMatrixRotationQuaternion(rotation);
  return m.r[2];
}

XMVECTOR CEntityOld::getLeft() const {
  XMMATRIX m = XMMatrixRotationQuaternion(rotation);
  return m.r[0];
}

XMVECTOR CEntityOld::getUp() const {
  XMMATRIX m = XMMatrixRotationQuaternion(rotation);
  return m.r[1];
}

void CEntityOld::setRotation(XMVECTOR new_rotation) {
  rotation = XMQuaternionNormalize(new_rotation);
}

// Returns true if the point is in the positive part of my front
bool CEntityOld::isInFront(XMVECTOR loc) const {
  return XMVectorGetX(XMVector3Dot(getFront(), loc - position)) > 0.f;
}

bool CEntityOld::isInLeft(XMVECTOR loc) const {
  return XMVectorGetX(XMVector3Dot(getLeft(), loc - position)) > 0.f;
}

bool CEntityOld::isInFov(XMVECTOR loc, float fov_in_rad) const {
  XMVECTOR unit_delta = XMVector3Normalize(loc - position);
  float cos_angle = XMVectorGetX(XMVector3Dot(getFront(), unit_delta));
  return(cos_angle < cos(fov_in_rad * 0.5f));
}

// ----------------------------------------------------------
COldEntityManager old_entity_manager;

CEntityOld* COldEntityManager::create(const char *name) {
  CEntityOld *e = new CEntityOld;
  strcpy(e->name, name);
  old_entities.push_back(e);
  return e;
}

bool COldEntityManager::destroy(CEntityOld* e) {
  auto it = std::find(old_entities.begin(), old_entities.end(), e);
  if (it == old_entities.end())
    return false;
  delete e;
  old_entities.erase(it);
  return true;
}

CEntityOld* COldEntityManager::getByName(const char *name) {

	for (auto& it : old_entities) {
    if (strcmp(it->name, name) == 0)
      return it;
  };
  return nullptr;
}

// ---------------------------------------------------------------------

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