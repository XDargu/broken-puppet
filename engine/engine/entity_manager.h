#ifndef INC_ENTITY_MANAGER_H_
#define INC_ENTITY_MANAGER_H_

#include "mcv_platform.h"
#include "aabb.h"

class CHandle;

struct TEntityChunk {
	char name[32];
	std::vector< CHandle > entities;
	AABB bounds;

	TEntityChunk(char the_name[32], XMVECTOR min, XMVECTOR max) {
		std::strcpy(name, the_name);
		bounds.min = min;
		bounds.max = max;
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		if (elem == "name") {
			std::strcpy(name, atts.getString("name", "unnamed").c_str());
		}
		if (elem == "aabb") {
			bounds.min = atts.getPoint("min");
			bounds.max = atts.getPoint("max");
		}
	}
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

	TEntityChunk* getCurrentRoom(XMVECTOR position);
	VEntities getCurrentRoomEntities(XMVECTOR position);

	bool removeFromChunk(CHandle the_handle);

	TEntityChunk* addChunk(char name[32], XMVECTOR min, XMVECTOR max);	

protected:
	// General entity vector
	VEntities entities;
	VEntities handles_to_destroy;

	// Vector of entity chuncks
	MEntities entity_chunks;
};


#endif
