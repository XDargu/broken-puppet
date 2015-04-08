#ifndef INC_COMP_NAME_H_
#define INC_COMP_NAME_H_

#include "mcv_platform.h"
#include "base_component.h"
#include "../handle/app_msgs.h"

struct TCompName : TBaseComponent {     // 1
	char name[64];
	TCompName() { name[0] = 0x00; }
	TCompName(const char *aname) { strcpy(name, aname); }
	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		strcpy(name, atts["name"].c_str());
	}

	std::string toString() {
		return name;
	}
  void onDebugMsg(const TMsgExplosion& msg) {
	  dbg("TCompName recv msg of type %f\n", msg.damage);
  }

};

#endif
