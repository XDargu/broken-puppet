#ifndef INC_ENTITY_MANAGER_H_
#define INC_ENTITY_MANAGER_H_

#include "mcv_platform.h"

class CHandle;

struct TEntityChunk {
	char name[32];
	std::vector< CHandle > entities;
	XMVECTOR min;
	XMVECTOR max;
};

class CEntityManager {
	typedef std::vector< CHandle > VEntities;
	typedef std::vector< TEntityChunk > MEntities;
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

	// Vector of entity chuncks
	MEntities entity_chunks;
};


#endif
