#ifndef INC_COMP_BOSS_PREFAB_H_
#define INC_COMP_BOSS_PREFAB_H_

#include "base_component.h"


struct TCompBossPrefab : TBaseComponent {

private:
	CHandle m_trans;
	CHandle m_entity;
	CHandle m_boss;
	float removing_deep;

public:
	TCompBossPrefab();
	~TCompBossPrefab();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void update(float elapsed);
};
#endif
