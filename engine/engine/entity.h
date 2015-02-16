#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_

#include "mcv_platform.h"

class CEntityOld {
  XMVECTOR rotation;
  XMVECTOR position;
  XMVECTOR scale;

public:

  char     name[32];

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

  CEntityOld();
  ~CEntityOld();
};

class COldEntityManager {

public:

  typedef std::vector< CEntityOld* > VOldEntities;

  CEntityOld* create(const char *name);
  bool destroy(CEntityOld* e);
  CEntityOld* getByName(const char *name);
  
  void renderDebug() const; 

  const VOldEntities& getEntities() const { return old_entities; }

protected:
	VOldEntities old_entities;

};

extern COldEntityManager old_entity_manager;

class CEntityManager {
	typedef std::vector< CEntity* > VEntities;

public:
	static CEntityManager& get();

	void add(CEntity* the_entity);
	bool remove(CEntity* the_entity);
	CEntity* getByName(const char *name);
	const VEntities& getEntities() const { return entities; }

	CEntity* createEmptyEntity();

protected:
	// General entity vector
	VEntities entities;
};


#endif
