#include "mcv_platform.h"
#include "ssao_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TSSAOStep::create(const char* name, int axres, int ayres, int afactor) {
	radius = 0.2f;

	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_ssao = new CRenderToTexture();

	bool is_ok = rt_ssao->create(name, xres / factor, yres / factor, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);

	return is_ok;
}

void TSSAOStep::apply(CTexture* in) {
	ctes_ssao.activateInPS(3);
	texture_manager.getByName("rt_depth")->activate(2);

	// Sample to the left & right
	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesSSAO* cb = ctes_ssao.get();
	cb->ssao_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->radius = radius;
	ctes_ssao.uploadToGPU();
	rt_ssao->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "ssao");
}

CTexture* TSSAOStep::getOutput() {
	return rt_ssao;
}