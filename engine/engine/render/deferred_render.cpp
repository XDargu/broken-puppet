#include "mcv_platform.h"
#include "deferred_render.h"
#include "render/render_manager.h"

bool CDeferredRender::create(int xres, int yres) {

	// Not owned by me, as it's the texture manager who will delete it
	rt_lights = new CRenderToTexture;
	rt_albedo = new CRenderToTexture;
	rt_normals = new CRenderToTexture;

	if (!rt_lights->create("rt_lights", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::USE_BACK_ZBUFFER))
		return false;
	if (!rt_albedo->create("rt_albedo", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	if (!rt_normals->create("rt_normals", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	return true;
}

void CDeferredRender::generateGBuffer(const CCamera* camera) {
	CTraceScoped scope("gbuffer");

	ID3D11RenderTargetView* rts[3] = {
		rt_albedo->render_target_view
		, rt_normals->render_target_view
		, rt_lights->render_target_view
	};
	::render.ctx->OMSetRenderTargets(3, rts, ::render.depth_stencil_view);
	rt_albedo->activateViewport();

	float black[4] = { 0.2f, 0.2f, 0.2f, 0.f };
	rt_albedo->clearColorBuffer(black);
	rt_normals->clearColorBuffer(black);
	rt_lights->clearColorBuffer(black);

	texture_manager.getByName("Zthe_light")->activate(6);

	render_manager.renderAll(camera);
}

void CDeferredRender::generateLightBuffer() {
	CTraceScoped scope("light_buffer");
	rt_lights->activate();
	float black[4] = { 0.2f, 0.2f, 0.2f, 0.f };
	rt_lights->clearColorBuffer(black);
}

void CDeferredRender::render(const CCamera* camera) {
	CTraceScoped scope("deferred");
	generateGBuffer(camera);
	//generateLightBuffer();
}

