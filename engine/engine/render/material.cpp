#include "mcv_platform.h"
#include "material.h"

CMaterialManager material_manager;

void CMaterial::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "std_material") {
		std::string diffuse_name = atts["diffuse"];
		std::string tech_name = atts["tech"];
		std::string my_name = atts["name"];
		if (tech_name.empty())
			tech_name = "textured";
		diffuse = texture_manager.getByName(diffuse_name.c_str());
		tech = render_techniques_manager.getByName(tech_name.c_str());
		XASSERT(diffuse->isValid(), "Texture not loaded: %s", diffuse_name.c_str());
		XASSERT(tech, "Technique not loaded: %s", tech_name.c_str());
	}
}

void CMaterial::activateTextures() const {
	// textures ... activate
	diffuse->activate(0);
}



bool CMaterial::load(const char* name) {
	CErrorContext ce("Loading material", name);
	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s.xml", "data/materials", name);

	bool mat_load = xmlParseFile(full_name);
	XASSERT(mat_load, "Material: %s not found", name);
	return xmlParseFile(full_name);
}

void CMaterial::destroy() {
	
}
