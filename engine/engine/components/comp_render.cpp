#include "mcv_platform.h"
#include "comp_render.h"
#include "render/render_manager.h"

void TCompRender::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	if (elem == "draw") {
		std::string mesh_name = atts.getString("mesh", "missing_mesh");
		std::string mat_name = atts.getString("mat", "missing_mat");
		mesh = mesh_manager.getByName(mesh_name.c_str());
		submesh_id = atts.getInt("sub_mesh", -1);
		assert(submesh_id >= 0);
		mat = material_manager.getByName(mat_name.c_str());

		render_manager.addKey(mesh, mat, submesh_id, CHandle(this));

		RKey k = { mesh_name, mat_name, submesh_id };
		keys.push_back(k);
	}
}
