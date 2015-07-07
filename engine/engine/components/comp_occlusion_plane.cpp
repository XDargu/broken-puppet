#include "mcv_platform.h"
#include "render/render_manager.h"
#include "comp_transform.h"
#include "comp_occlusion_plane.h"

void TCompOcclusionPlane::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	TCompTransform* m_trans = assertRequiredComponent<TCompTransform>(this);
	float width = atts.getFloat("width", 1);
	float heigth = atts.getFloat("heigth", 1);

	render_manager.addOcclusionPlane(m_trans, width, heigth);
}