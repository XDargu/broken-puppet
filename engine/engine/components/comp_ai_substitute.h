#ifndef INC_COMP_AI_SUBSTITUTE_H_
#define INC_COMP_AI_SUBSTITUTE_H_

#include "base_component.h"
#include "physics_manager.h"
#include "../ai/fsm_substitute.h"
#include "handle\app_msgs.h"

struct TCompSubstituteBoss : TBaseComponent {
private:
	fsm_substitute* m_fsm_substitute;

public:
	TCompSubstituteBoss();
	~TCompSubstituteBoss();

	void init();
	void update(float elapsed);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void onRopeTensed(const TMsgRopeTensed& msg);

	void initLittleTalk();
	void hitSubstitute();
};


#endif