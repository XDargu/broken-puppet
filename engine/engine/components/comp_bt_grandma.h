#ifndef INC_COMP_BT_GRANDMA_H_
#define INC_COMP_BT_GRANDMA_H_

#include "base_component.h"
#include "../ai/bt_grandma.h"
#include "handle\app_msgs.h"

struct TCompBtGrandma : TBaseComponent {

	bt_grandma* m_ai_controller;

	TCompBtGrandma();
	~TCompBtGrandma();
	TCompBtGrandma(bt_grandma* ai_controller);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void actorHit(const TActorHit& msg);

	void warWarning(const TWarWarning& msg);

	void notifyPlayerFound(const TPlayerFound& msg);

	void groundHit(const TGroundHit& msg);

	void onRopeTensed(const TMsgRopeTensed& msg);

	void renderDebug3D();
};
#endif


