#include "mcv_platform.h"
#include "chromatic_aberration_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TChromaticAberrationStep::create(const char* name, int axres, int ayres, int afactor) {
	amount = 0.03f;

	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_ca = new CRenderToTexture();

	bool is_ok = rt_ca->create(name, xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);

	return is_ok;
}

void TChromaticAberrationStep::apply(CTexture* in) {
	ctes_chromatic_aberration.activateInPS(3);
	texture_manager.getByName("rt_depth")->activate(2);

	// Sample to the left & right
	float inv_resolution_x = 1.0 / (float)xres;
	float inv_resolution_y = 1.0 / (float)yres;
	TCtesChromaticAberration* cb = ctes_chromatic_aberration.get();
	cb->ca_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->chromatic_amount = amount;
	ctes_chromatic_aberration.uploadToGPU();
	rt_ca->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "chromatic_aberration");
}

CTexture* TChromaticAberrationStep::getOutput() {
	return rt_ca;
}