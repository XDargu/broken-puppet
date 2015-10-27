#include "mcv_platform.h"
#include "lens_flare_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TLensFlareStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	amount = 1;
	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_lens = new CRenderToTexture();

	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_lens->create("dlens_flare_pp", xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TLensFlareStep::apply(CTexture* in) {
	ctes_blur_camera.activateInPS(3);

	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesBlurCamera* cb = ctes_blur_camera.get();
	cb->blur_camera_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->blur_camera_amount = amount;
	ctes_blur_camera.uploadToGPU();

	texture_manager.getByName("lens_flare_mascara")->activate(9);
	texture_manager.getByName("lens_flare_suciedad")->activate(10);
	rt_lens->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "lens_flare");
}

CTexture* TLensFlareStep::getOutput() {
	return rt_lens;
}

void TLensFlareStep::destroy() {
	//SAFE_DESTROY(rt_blur);
	//if (rt_blur) { rt_blur->destroyAll(); rt_blur = nullptr; }
}