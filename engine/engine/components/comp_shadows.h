#ifndef INC_COMP_RENDER_SHADOWS_H_
#define INC_COMP_RENDER_SHADOWS_H_

#include "render/render_to_texture.h"
#include "comp_camera.h"
#include "render/render_utils.h"
#include "render/render_manager.h"

struct TCompShadows {
	int              resolution;
	CRenderToTexture rt;

	TCompShadows() : resolution(256) {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		resolution = atts.getInt("res", 256);
	}

	void init() {

		CEntity* e = CHandle(this).getOwner();
		assert(e);

		bool is_ok = rt.create(e->getName(), resolution, resolution
			, DXGI_FORMAT_UNKNOWN     // I do NOT want COLOR Buffer
			, DXGI_FORMAT_R32_TYPELESS
			);
		assert(is_ok);
	}

	void generate()  {
		CTraceScoped s("shadows");

		// Activate the camera component which is the source of the light
		CEntity* e = CHandle(this).getOwner();
		TCompCamera* camera = e->get<TCompCamera>();
		assert(camera || fatal("TCompShadows requieres a TCompCamera component"));

		// Start rendering in the rt of the depth buffer
		rt.activate();
		rt.clearDepthBuffer();

		activateCamera(*camera, 1);
		
		// 
		render_techniques_manager.getByName("gen_shadows")->activate();

		render_manager.renderShadowsCasters();

	}


};

#endif
