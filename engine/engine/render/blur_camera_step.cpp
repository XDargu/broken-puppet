#include "mcv_platform.h"
#include "blur_camera_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TBlurCameraStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	amount = 1;
	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_blur = new CRenderToTexture();

	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_blur->create(name, xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TBlurCameraStep::apply(CTexture* in) {
	ctes_blur_camera.activateInPS(3);

	// Check if neccessary
	// Binary compare matrix not working
	/*TCtesCamera* cc = ctes_camera.get();
	int equal = memcmp(&cc->GameViewProjection, &cc->ViewProjection, sizeof(XMMATRIX));
	if (equal == 0) {
		rt_blur->activate();
		drawTexture2D(0, 0, render.xres, render.yres, in);
		return;
	}*/

	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesBlurCamera* cb = ctes_blur_camera.get();
	cb->blur_camera_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->blur_camera_amount = amount;
	ctes_blur_camera.uploadToGPU();
	rt_blur->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "blur_camera");
}

CTexture* TBlurCameraStep::getOutput() {
	return rt_blur;
}

void TBlurCameraStep::destroy() {
	SAFE_DESTROY(rt_blur);
}