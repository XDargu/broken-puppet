#include "mcv_platform.h"
#include "options_parser.h"

void COptionsParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "resolution") {
		x_res=atts.getInt("xres", 800);
		y_res=atts.getInt("yres", 600);
		fullscreen = atts.getBool("fullscreen", false);
		loaded = true;
	}
	if (elem == "first_scene") {
		first_scene = "data/scenes/" + atts.getString("name", "my_file") + ".xml";
	}
}

void COptionsParser::getResolution(int &xres, int &yres, bool &mode) {
	if (loaded){
		xres = x_res;
		yres = y_res;
		mode = fullscreen;
	}else{
		XDEBUG("Settings not loaded");
	}
}

