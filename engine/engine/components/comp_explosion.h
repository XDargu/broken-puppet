#ifndef INC_COMP_EXPLOSION_H_
#define INC_COMP_EXPLOSION_H_

#include "base_component.h"
#include "handle\app_msgs.h"

struct TCompExplosion : TBaseComponent {  

private:

	CHandle mEntity;
	CHandle comp_trans;
	float damage;
	float radius;
	float force_threshold;

public:
	void init();
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void onDetonate(const TMsgOnDetonate& msg);
	
};

#endif
