#ifndef INC_COMP_TAG_H_
#define INC_COMP_TAG_H_

#include "base_component.h"

struct TCompTag : TBaseComponent {   
	std::string tag;

	TCompTag() : tag("untagged") { }
	TCompTag(const char* tagged) : tag(tagged) { }

	void loadFromAtts(MKeyValue &atts) {
		tag = atts.getString("tagName", "untagged");
	}

	std::string toString() {
		return "Tag: " + tag;
	}

	std::string getTag() {
		return tag;
	}
};

#endif