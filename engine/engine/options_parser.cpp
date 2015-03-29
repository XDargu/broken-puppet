#include "mcv_platform.h"
#include "options_parser.h"

void COptionsParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem != "options") {
		x_res=atts.getInt("xres", 0);
		y_res=atts.getInt("yres", 0);
		fullscreen = atts.getBool("fullscreen", false);
		loaded = true;
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

