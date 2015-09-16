#ifndef INC_CINEMATIC_PARSER_H_
#define INC_CINEMATIC_PARSER_H_

#include "handle\handle.h"
#include "XMLParser.h"
#include "rigid_animation.h"
#include "handle\handle.h"

class CCinematicParser : public CXMLParser {
public:
	CRigidAnimation animation;
	CHandle target_transform;

	CCinematicParser() : CXMLParser() { }

	void onStartElement(const std::string &elem, MKeyValue &atts);
};

#endif
