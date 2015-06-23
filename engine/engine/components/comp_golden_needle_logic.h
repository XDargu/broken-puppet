#ifndef INC_COMP_GOLDEN_NEEDLE_LOGIC_H_
#define INC_COMP_GOLDEN_NEEDLE_LOGIC_H_

#include "base_component.h"

struct TCompGNLogic : TBaseComponent {
private:
	CHandle m_transform;
	CHandle m_aabb;
	CHandle player_transform;
	CHandle player_aabb;
	CHandle player;
	XMVECTOR clue_point;
	bool used;
public:

	TCompGNLogic();
	~TCompGNLogic();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	bool checkPlayerInside();

	XMVECTOR getCluePoint();

	void throwGoldenNeedle();

	void renderDebug3D();
};

#endif

