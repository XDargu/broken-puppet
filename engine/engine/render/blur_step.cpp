#include "mcv_platform.h"
#include "blur_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TBlurStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_down_by_x = new CRenderToTexture();
	rt_down_by_y = new CRenderToTexture();
	sprintf(name_by_x, "%s_x", name);
	sprintf(name_by_y, "%s_y", name);
	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_down_by_x->create(name_by_x, xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	is_ok &= rt_down_by_y->create(name_by_y, xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TBlurStep::apply(CTexture* in) {
	ctes_blur.activateInPS(3);

	// Sample to the left & right
	float inv_resolution_x = 1.0 / (float)xres;
	float inv_resolution_y = 1.0 / (float)yres;
	TCtesBlur* cb = ctes_blur.get();
	cb->blur_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	ctes_blur.uploadToGPU();
	rt_down_by_y->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "blur");

	// Sample up & down
	/*float inv_resolution_y = 1.0 / (float)yres;
	cb->blur_delta = XMVectorSet(0, inv_resolution_y, 0, 0);
	ctes_blur.uploadToGPU();
	rt_down_by_y->activate();
	drawTexture2D(0, 0, render.xres, render.yres, rt_down_by_x, "blur");*/
}

CTexture* TBlurStep::getOutput() {
	return rt_down_by_y;
}