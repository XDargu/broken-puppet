#ifndef INC_COMP_TRIGGER_H_
#define INC_COMP_TRIGGER_H_

#include "base_component.h"

struct TCompTrigger : TBaseComponent {
private:
	CHandle m_transform;
	CHandle m_aabb;
	std::vector<CEntity*> inside;
public:

    TCompTrigger(){}
	~TCompTrigger();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	bool onEnter();

	bool onExit();

	bool checkIfInside(CEntity* entity);

	void remove(std::vector<CEntity*>& vec, size_t pos);

	void renderDebug3D();
};

#endif