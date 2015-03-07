#ifndef INC_ENTITY_MANAGER_H_
#define INC_ENTITY_MANAGER_H_

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

class CHandle;

class CEntityManager {
	typedef std::vector< CHandle > VEntities;
	unsigned int entity_event_count;
public:
	static CEntityManager& get();

	void add(CHandle the_entity);
	bool remove(CHandle the_handle);
	void destroyRemovedHandles();
	bool clear();
	CHandle getByName(const char *name);
	const VEntities& getEntities() const { return entities; }

	CHandle createEmptyEntity();

	unsigned int getEntityEventCount();

protected:
	// General entity vector
	VEntities entities;
	VEntities handles_to_destroy;
};


#endif
