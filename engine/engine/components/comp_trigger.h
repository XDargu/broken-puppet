#ifndef INC_COMP_TRIGGER_H_
#define INC_COMP_TRIGGER_H_

#include "base_component.h"

struct TCompTrigger : TBaseComponent {
private:
	CHandle player_entity;
	CHandle m_transform;
	CHandle m_aabb;
	std::vector<CHandle> inside;
	bool player_only;
	bool bots_only;
public:

	TCompTrigger() : m_transform(CHandle()), m_aabb(CHandle()), player_only(false), bots_only(false) {}
	~TCompTrigger();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	bool onEnter();

	bool onExit();

	bool checkIfInside(CHandle entity);

	void remove(std::vector<CHandle>& vec, size_t pos);

	void renderDebug3D();
};

#endif