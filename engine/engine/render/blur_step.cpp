#include "mcv_platform.h"
#include "blur_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TBlurStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	amount = 1;
	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_blur_x = new CRenderToTexture();
	//rt_blur_y = new CRenderToTexture();
	
	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_blur_x->create("blur_pp", xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	//is_ok &= rt_blur_y->create(name_by_y, xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TBlurStep::apply(CTexture* in) {
	ctes_blur.activateInPS(3);

	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesBlur* cb = ctes_blur.get();
	cb->blur_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->blur_amount = amount;
	ctes_blur.uploadToGPU();
	rt_blur_x->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "blur");
	/*drawTexture2D(0, 0, render.xres, render.yres, in, "blur_by_x");
	((CTexture*)rt_blur_x)->activate(1);
	rt_blur_y->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "blur_by_y");*/
}

CTexture* TBlurStep::getOutput() {
	return rt_blur_x;
}

void TBlurStep::destroy() {
	//SAFE_DESTROY(rt_blur_x);
	if (rt_blur_x) { rt_blur_x->destroyAll(); rt_blur_x = nullptr; }
}