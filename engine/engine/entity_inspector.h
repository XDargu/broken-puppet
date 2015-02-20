#ifndef INC_ENTITY_INSPECTOR_H_
#define INC_ENTITY_INSPECTOR_H_

#include <AntTweakBar.h>

class CEntity;

class CEntityInspector
{
private:
	CEntity* target_entity;
public:
	CEntityInspector();
	~CEntityInspector();
	void init();
	void update();
	void inspectEntity(CEntity* the_entity);
};

class CEntityLister
{
private:
	unsigned int	m_entity_event_count;
	std::string		prevSearch;
public:
	std::string		searchIn;

	CEntityLister();
	~CEntityLister();
	void init();
	void update();
};

class CEntityActioner
{
public:
	CEntityActioner();
	~CEntityActioner();
	void init();
	void update();
};

class CDebugOptioner
{
public:

	CDebugOptioner();
	~CDebugOptioner();
	void init();
};

#endif