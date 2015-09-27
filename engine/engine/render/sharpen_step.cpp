#include "mcv_platform.h"
#include "sharpen_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TSharpenStep::create(const char* name, int axres, int ayres, int afactor) {

	
	amount = 1;

	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_sharpen = new CRenderToTexture();

	bool is_ok = rt_sharpen->create("sharpen_pp", xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);

	return is_ok;
}

void TSharpenStep::apply(CTexture* in) {
	ctes_sharpen.activateInPS(3);

	// Sample to the left & right
	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesSharpen* cb = ctes_sharpen.get();	
	cb->sharpen_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->amount = amount;
	ctes_sharpen.uploadToGPU();
	rt_sharpen->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "sharpen");
}

CTexture* TSharpenStep::getOutput() {
	return rt_sharpen;
}

void TSharpenStep::destroy() {
	//SAFE_DESTROY(rt_sharpen);
	if (rt_sharpen) { rt_sharpen->destroyAll(); rt_sharpen = nullptr; }
}