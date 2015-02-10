#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_

#include "mcv_platform.h"

#include <PxPhysicsAPI.h>// BORRAR, POR EL AMOR DE DIOS --TODO
using namespace physx;// BORRAR, POR EL AMOR DE DIOS --TODO

class CEntity {
  XMVECTOR rotation;
  XMVECTOR position;
  XMVECTOR scale;

public:

  char     name[32];
  PxRigidDynamic*	rigid;	// BORRAR, POR EL AMOR DE DIOS --TODO
  XMMATRIX getWorld() const;
  XMVECTOR getFront() const;
  XMVECTOR getLeft() const;
  XMVECTOR getUp() const;
  void setRotation(XMVECTOR new_rotation);
  void setPosition(XMVECTOR new_position) { position = new_position; }
  XMVECTOR getRotation() const { return rotation; }
  XMVECTOR getPosition() const { return position; }
  XMVECTOR getScale() const { return scale; }

  bool isInFront(XMVECTOR loc) const;
  bool isInLeft(XMVECTOR loc) const;
  bool isInFov(XMVECTOR loc, float fov_in_rad ) const;

  CEntity();
  ~CEntity();
};

class CEntityManager {

public:

  typedef std::vector< CEntity* > VEntities;

  CEntity* create(const char *name);
  bool destroy(CEntity* e);
  CEntity* getByName(const char *name);
  
  void renderDebug() const; 

  const VEntities& getEntities() const { return entities; }

protected:
  VEntities entities;

};

extern CEntityManager entity_manager;


#endif
