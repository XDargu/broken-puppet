#ifndef INC_COMP_HITCH_H_
#define INC_COMP_HITCH_H_

#include "base_component.h"
#include "handle\app_msgs.h"

struct TCompHitch : TBaseComponent {

private:
	bool is_heart;
	CHandle m_boss;

public:
	TCompHitch();
	~TCompHitch();

	void init();
	void onRopeTensed(const TMsgRopeTensed& msg);
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	
};

#endif
