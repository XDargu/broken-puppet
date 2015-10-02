#include "mcv_platform.h"
#include "importer_resource_loader.h"
#include "skeletons\skeleton_manager.h"

void CImporterResourceLoader::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "draw") {
		std::string mesh_name = atts.getString("mesh", "missing_mesh");
		std::string mat_name = atts.getString("mat", "missing_mat");

		mesh_manager.getByName(mesh_name.c_str());
		material_manager.getByName(mat_name.c_str());

	}
	if (elem == "skeleton") {
		std::string skeleton_name = atts.getString("name", "missing_mesh");
		skeleton_manager.getByName(skeleton_name.c_str());
	}
}