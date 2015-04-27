#ifndef INC_COLLISION_PARSER_H_
#define INC_COLLISION_PARSER_H_

#include "handle\handle.h"
#include "XMLParser.h"

class CCollisionParser : public CXMLParser {
private:
	std::string own_tag;
	std::string not_collides;
	bool loaded;
	void convertInCollisionFilter(std::string own, std::string not_col);
public:
	void onStartElement(const std::string &elem, MKeyValue &atts);
};

#endif
