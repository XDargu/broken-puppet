#include "mcv_platform.h"
#include "render/render_manager.h"
#include "comp_transform.h"
#include "comp_render.h"

TCompRender::~TCompRender() {
	render_manager.removeKeysFromOwner(CHandle(this));
}

void TCompRender::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	
	assertRequiredComponent<TCompTransform>(this);

	if (elem == "draw") {		
		std::string mesh_name = atts.getString("mesh", "missing_mesh");
		std::string mat_name = atts.getString("mat", "missing_mat");
		mesh = mesh_manager.getByName(mesh_name.c_str());
		submesh_id = atts.getInt("sub_mesh", -1);

		XASSERT(submesh_id >= 0, "Invalid submesh id: %i", submesh_id);
		mat = material_manager.getByName(mat_name.c_str());

		render_manager.addKey(mesh, mat, submesh_id, CHandle(this), &active);

		RKey k = { mesh_name, mat_name, submesh_id };
		keys.push_back(k);
	}
}