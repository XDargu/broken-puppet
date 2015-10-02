#include "mcv_platform.h"
#include "silouette_step.h"
#include "render_utils.h"
#include "render\ctes\shader_ctes.h"

bool TSilouetteStep::create(const char* name, int axres, int ayres, int afactor) {

	//ctes_blur.create();

	amount = 0;
	factor = afactor;
	xres = axres;
	yres = ayres;
	rt_dynamic = new CRenderToTexture();
	rt_sobel = new CRenderToTexture();
	rt_final = new CRenderToTexture();

	// xres, yres = 800 x 600
	// by_x => 400x600
	// by_y => 400x300
	bool is_ok = rt_dynamic->create("sil_dynamic_pp", xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	is_ok &= rt_sobel->create("sil_sobel_pp", xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	is_ok &= rt_final->create("sil_final_pp", xres / factor, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::NO_ZBUFFER);
	return is_ok;
}

void TSilouetteStep::apply(CTexture* in) {

	XMVECTOR light_pos = XMVectorSet(0, 7, -30, 0);

	ctes_glow.activateInPS(3);

	float inv_resolution_x = 1.0f / (float)xres;
	float inv_resolution_y = 1.0f / (float)yres;
	TCtesGlow* cb = ctes_glow.get();
	cb->glow_delta = XMVectorSet(inv_resolution_x, inv_resolution_y, 0, 0);
	cb->glow_amount = amount;
	cb->glow_pos = light_pos;
	ctes_glow.uploadToGPU();

	rt_dynamic->activate();
	drawTexture2D(0, 0, render.xres, render.yres, in, "silouette_type");

	rt_sobel->activate();
	((CTexture*)rt_dynamic)->activate(9);
	drawTexture2D(0, 0, render.xres, render.yres, in, "silouette");

	// If we want to post-process the silouette
	/*rt_final->activate();
	((CTexture*)rt_sobel)->activate(9);
	drawTexture2D(0, 0, render.xres, render.yres, in, "silouette_glow");*/

}

CTexture* TSilouetteStep::getOutput() {
	return rt_sobel;
}

void TSilouetteStep::destroy() {
	//SAFE_DESTROY(rt_final);
	//SAFE_DESTROY(rt_sobel);
	//SAFE_DESTROY(rt_dynamic);
	//if (rt_final) { rt_final->destroyAll(); rt_final = nullptr; }
	//if (rt_sobel) { rt_sobel->destroyAll(); rt_sobel = nullptr; }
	//if (rt_dynamic) { rt_dynamic->destroyAll(); rt_dynamic = nullptr; }
}