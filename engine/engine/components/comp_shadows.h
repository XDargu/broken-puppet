#ifndef INC_COMP_RENDER_SHADOWS_H_
#define INC_COMP_RENDER_SHADOWS_H_

#include "base_component.h"
#include "render/render_to_texture.h"
#include "comp_camera.h"
#include "render/render_utils.h"
#include "render/render_manager.h"

struct TCompShadows : TBaseComponent {
	int               resolution;
	CRenderToTexture* rt;					// Not owned by us
	XMVECTOR          color;
	float intensity;
	bool character_camera;

	TCompShadows() : resolution(256), rt( nullptr ), intensity(1), character_camera(false){
		color = DirectX::XMVectorSet(1, 1, 1, 1);
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	// This generates the depth map from the ligth source
	void generate();

	// This renders the light volume into the light accumulation buffer
	void draw();


};

#endif
