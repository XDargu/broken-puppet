#ifndef INC_COMP_GOLDEN_NEEDLE_ITEM_H_
#define INC_COMP_GOLDEN_NEEDLE_ITEM_H_

#include "base_component.h"

struct TCompGNItem : TBaseComponent {
private:
	CHandle m_transform;
	CHandle m_aabb;
	CHandle player_transform;
	CHandle player_aabb;
	CHandle player;
	bool taked;
public:

	TCompGNItem();
	~TCompGNItem();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void renderDebug3D();
};

#endif

