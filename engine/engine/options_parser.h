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
	std::string first_scene;

	COptionsParser() {
		// Default settings
		x_res = 800;
		y_res = 600;
		fullscreen = false;
		first_scene = "data/scenes/my_file.xml";
	}
	void onStartElement(const std::string &elem, MKeyValue &atts);
	void getResolution(int &xres, int &yres, bool &mode);
};

#endif

