#ifndef INC_COMP_BT_SOLDIER_H_
#define INC_COMP_BT_SOLDIER_H_

#include "base_component.h"
#include "../ai/bt_soldier.h"
#include "handle\app_msgs.h"

struct TCompBtSoldier : TBaseComponent {

	bt_soldier* m_ai_controller;
	CHandle m_aabb;
	CHandle player_entity;
	CHandle player_transform;
	float footstep_counter;

	TCompBtSoldier();
	~TCompBtSoldier();
	TCompBtSoldier(bt_soldier* ai_controller);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void actorHit(const TActorHit& msg);

	void warWarning(const TWarWarning& msg);

	void notifyPlayerFound(const TPlayerFound& msg);

	void notifyPlayerTouch(const TPlayerTouch& msg);

	void groundHit(const TGroundHit& msg);

	void onRopeTensed(const TMsgRopeTensed& msg);

	void onNeedleHit(const TMsgNeedleHit& msg);

	void renderDebug3D();
};
#endif

