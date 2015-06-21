#include "mcv_platform.h"
#include "ssrr_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TSSRRStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	amount = 1;
	quality = 2;
	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_ssrr = new CRenderToTexture();

	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_ssrr->create(name, xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TSSRRStep::apply(CTexture* in) {
	ctes_ssrr.activateInPS(3);

	float inv_resolution_x = 1.0 / (float)xres;
	float inv_resolution_y = 1.0 / (float)yres;
	TCtesSSRR* cssrr = ctes_ssrr.get();
	cssrr->ssrr_amount = amount;
	cssrr->ssrr_quality = quality;
	ctes_ssrr.uploadToGPU();
	rt_ssrr->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "ssrr");
}

CTexture* TSSRRStep::getOutput() {
	return rt_ssrr;
}