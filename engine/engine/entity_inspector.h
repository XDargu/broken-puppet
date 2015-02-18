#ifndef INC_ENTITY_INSPECTOR_H_
#define INC_ENTITY_INSPECTOR_H_

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
public:
	CEntityLister();
	~CEntityLister();
	void init();
	void update();
};

#endif