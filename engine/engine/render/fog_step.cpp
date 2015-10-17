#include "mcv_platform.h"
#include "fog_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TFogStep::create(const char* name, int axres, int ayres, int afactor) {


	amount = 1;

	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_fog = new CRenderToTexture();

	bool is_ok = rt_fog->create("fog_pp", xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);

	fog_level = -1000;
	fog_color = XMVectorSet(0.4f, 0.4f, 0.4f, 0.4f);
	fog_distance = 1000;

	return is_ok;
}

void TFogStep::apply(CTexture* in) {
	ctes_fog.activateInPS(3);

	// Sample to the left & right
	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesFog* cb = ctes_fog.get();
	cb->fog_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->fog_amount = amount;
	cb->fog_color = fog_color;
	cb->fog_level = fog_level;
	cb->fog_distance = fog_distance;
	ctes_fog.uploadToGPU();
	rt_fog->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "fog");
}

CTexture* TFogStep::getOutput() {
	return rt_fog;
}

void TFogStep::destroy() {
	//SAFE_DESTROY(rt_sharpen);
	//if (rt_sharpen) { rt_sharpen->destroyAll(); rt_sharpen = nullptr; }
}