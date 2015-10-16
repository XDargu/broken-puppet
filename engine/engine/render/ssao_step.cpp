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
	rt_ssao_blur = new CRenderToTexture();

	bool is_ok = rt_ssao->create("ssao_pp", xres , yres , DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	is_ok &= rt_ssao_blur->create("ssao_blur_pp", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);

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
	texture_manager.getByName("noise")->activate(9);
	drawTexture2D(0, 0, render.xres, render.yres, in, "ssao");

	rt_ssao_blur->activate();
	((CTexture*)rt_ssao)->activate(9);
	drawTexture2D(0, 0, render.xres, render.yres, in, "ssao_blur");
}

CTexture* TSSAOStep::getOutput() {
	return rt_ssao_blur;
}

void TSSAOStep::destroy() {
	//SAFE_DESTROY(rt_ssao);
	//if (rt_ssao) { rt_ssao->destroyAll(); rt_ssao = nullptr; }
}