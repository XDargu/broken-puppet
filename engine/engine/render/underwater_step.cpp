#include "mcv_platform.h"
#include "underwater_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TUnderwaterEffect::create(const char* name, int axres, int ayres, int afactor) {


	amount = 1;

	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_underwater = new CRenderToTexture();

	bool is_ok = rt_underwater->create(name, xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);

	return is_ok;
}

void TUnderwaterEffect::apply(CTexture* in) {
	ctes_underwater.activateInPS(3);

	// Sample to the left & right
	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesUnderwater* cb = ctes_underwater.get();
	cb->uw_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->uw_amount = amount;
	cb->water_level = water_level;
	ctes_underwater.uploadToGPU();
	rt_underwater->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "underwater");
}

CTexture* TUnderwaterEffect::getOutput() {
	return rt_underwater;
}