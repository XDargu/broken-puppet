#include "mcv_platform.h"
#include "material.h"

CMaterialManager material_manager;

void CMaterial::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "std_material") {
		casts_shadows = atts.getBool("casts_shadows", true);
		solid = atts.getBool("solid", true);
		std::string diffuse_name = atts["diffuse"];
		std::string ao_name = atts["ao"];
		std::string normal_name = atts["normal"];
		std::string specular_name = atts["specular"];
		std::string glossiness_name = atts["glossiness"];
		std::string emissive_name = atts["emissive"];
		std::string cubemap_name = atts["cubemap"];
		std::string tech_name = atts["tech"];
		std::string my_name = atts["name"];
		if (tech_name.empty())
			tech_name = "deferred_gbuffer";
		diffuse = texture_manager.getByName(diffuse_name.c_str());
		if (normal_name != "")
			normal = texture_manager.getByName(normal_name.c_str());
		else 
			normal = texture_manager.getByName("black");
		if (ao_name != "")
			ao = texture_manager.getByName(ao_name.c_str());
		if (specular_name != "")
			specular = texture_manager.getByName(specular_name.c_str());
		else
			specular = texture_manager.getByName("black");
		if (glossiness_name != "")
			glossiness = texture_manager.getByName(glossiness_name.c_str());
		if (emissive_name != "")
			emissive = texture_manager.getByName(emissive_name.c_str());
		else
			emissive = texture_manager.getByName("black");
		if (cubemap_name != "")
			cubemap = texture_manager.getByName(cubemap_name.c_str());
		tech = render_techniques_manager.getByName(tech_name.c_str());
		assert(diffuse && tech);
	}
}

void CMaterial::activateTextures() const {
	// textures ... activate
	diffuse->activate(0);
	if (normal)
		normal->activate(1);      // as per t1 in the deferred.fx
	if (emissive)
		emissive->activate(7);
	if (specular)
		specular->activate(5);

	/*if (specular)
		specular->activate(2);
	if (glossiness)
		glossiness->activate(3);
	if (emissive)
		emissive->activate(4);
	if (ao)
		ao->activate(5);
	if (cubemap)
		cubemap->activate(7);*/
}



bool CMaterial::load(const char* name) {

	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s.xml", "data/materials", name);

	return xmlParseFile(full_name);
}

void CMaterial::destroy() {}