#ifndef INC_OPTIONS_PARSER_H_
#define INC_OPTIONS_PARSER_H_

#include "handle\handle.h"
#include "XMLParser.h"

class COptionsParser : public CXMLParser {
private:
	int x_res;
	int y_res;
	bool fullscreen;
	bool loaded;
public:
	void onStartElement(const std::string &elem, MKeyValue &atts);
	void getResolution(int &xres, int &yres, bool &mode);
};

#endif

