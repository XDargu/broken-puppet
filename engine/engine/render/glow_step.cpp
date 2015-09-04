#include "mcv_platform.h"
#include "glow_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TGlowStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	amount = 0;
	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_lights = new CRenderToTexture();
	rt_glow = new CRenderToTexture();

	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_lights->create(name_lights, xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	is_ok &= rt_glow->create(name_glow, xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TGlowStep::apply(CTexture* in) {

	XMVECTOR light_pos = XMVectorSet(0, 7, -30, 0);

	ctes_glow.activateInPS(3);

	float inv_resolution_x = 1.0 / (float)xres;
	float inv_resolution_y = 1.0 / (float)yres;
	TCtesGlow* cb = ctes_glow.get();
	cb->glow_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->glow_amount = amount;
	cb->glow_pos = light_pos;
	ctes_glow.uploadToGPU();

	rt_lights->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "glow_lights");
	
	rt_glow->activate();
	((CTexture*)rt_lights)->activate(1);
	drawTexture2D(0, 0, render.xres, render.yres, in, "glow");

}

CTexture* TGlowStep::getOutput() {
	return rt_glow;
}