#include "mcv_platform.h"
#include "entity.h"

using namespace DirectX;

CEntity::CEntity()
{
  position = XMVectorSet(0.f, 0.f, 0.f, 1.f);
  rotation = XMQuaternionIdentity();
  scale = XMVectorSet(1.f, 1.f, 1.f, 1.f);

  name[0] = 0x00;
}

CEntity::~CEntity()
{
}

XMMATRIX CEntity::getWorld() const {
  XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
  XMMATRIX m = XMMatrixAffineTransformation(scale, zero, rotation, position);
  return m;
}

XMVECTOR CEntity::getFront() const {
  XMMATRIX m = XMMatrixRotationQuaternion(rotation);
  return m.r[2];
}

XMVECTOR CEntity::getLeft() const {
  XMMATRIX m = XMMatrixRotationQuaternion(rotation);
  return m.r[0];
}

XMVECTOR CEntity::getUp() const {
  XMMATRIX m = XMMatrixRotationQuaternion(rotation);
  return m.r[1];
}

void CEntity::setRotation(XMVECTOR new_rotation) {
  rotation = XMQuaternionNormalize(new_rotation);
}

// Returns true if the point is in the positive part of my front
bool CEntity::isInFront(XMVECTOR loc) const {
  return XMVectorGetX(XMVector3Dot(getFront(), loc - position)) > 0.f;
}

bool CEntity::isInLeft(XMVECTOR loc) const {
  return XMVectorGetX(XMVector3Dot(getLeft(), loc - position)) > 0.f;
}

bool CEntity::isInFov(XMVECTOR loc, float fov_in_rad) const {
  XMVECTOR unit_delta = XMVector3Normalize(loc - position);
  float cos_angle = XMVectorGetX(XMVector3Dot(getFront(), unit_delta));
  return(cos_angle < cos(fov_in_rad * 0.5f));
}

// ----------------------------------------------------------
CEntityManager entity_manager;

CEntity* CEntityManager::create(const char *name) {
  CEntity *e = new CEntity;
  strcpy(e->name, name);
  entities.push_back(e);
  return e;
}

bool CEntityManager::destroy(CEntity* e) {
  auto it = std::find(entities.begin(), entities.end(), e);
  if (it == entities.end())
    return false;
  delete e;
  entities.erase(it);
  return true;
}

CEntity* CEntityManager::getByName(const char *name) {

  for(auto& it : entities) {
    if (strcmp(it->name, name) == 0)
      return it;
  };
  return nullptr;
}

