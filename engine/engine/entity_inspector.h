#ifndef INC_ENTITY_INSPECTOR_H_
#define INC_ENTITY_INSPECTOR_H_

#include <AntTweakBar.h>
#include "handle\handle.h"

class CEntity;

class CEntityInspector
{
private:
	CHandle target_entity;
public:
	static CEntityInspector& get();

	CEntityInspector();
	~CEntityInspector();
	void init();
	void update();
	void inspectEntity(CHandle the_entity);
	CHandle getInspectedEntity() { return target_entity; }
};

class CEntityLister
{
private:
	unsigned int	m_entity_event_count;
	std::string		prevSearch;
public:
	std::string		searchIn;

	static CEntityLister& get();

	CEntityLister();
	~CEntityLister();
	void init();
	void update();
	void resetEventCount();
};

class CEntityActioner
{
public:
	static CEntityActioner& get();

	CEntityActioner();
	~CEntityActioner();
	void init();
	void update();
};

class CDebugOptioner
{
public:
	static CDebugOptioner& get();

	CDebugOptioner();
	~CDebugOptioner();
	void init();
};

#endif