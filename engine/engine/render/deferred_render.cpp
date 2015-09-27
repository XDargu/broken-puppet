#include "mcv_platform.h"
#include "deferred_render.h"
#include "render/render_manager.h"
#include "components/comp_point_light.h"
#include "components/comp_shadows.h"

bool CDeferredRender::create(int xres, int yres) {

	// Not owned by me, as it's the texture manager who will delete it
	rt_lights = new CRenderToTexture;
	rt_albedo = new CRenderToTexture;
	rt_normals = new CRenderToTexture;
	rt_specular = new CRenderToTexture;
	rt_gloss = new CRenderToTexture;
	rt_depth = new CRenderToTexture;

	technique_deferred_point_lights = render_techniques_manager.getByName("deferred_point_lights");
	technique_deferred_dir_lights = render_techniques_manager.getByName("deferred_dir_lights");


	if (!rt_lights->create("rt_lights", xres, yres, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, CRenderToTexture::USE_BACK_ZBUFFER, true))
		return false;
	if (!rt_albedo->create("rt_albedo", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	if (!rt_normals->create("rt_normals", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	if (!rt_specular->create("rt_specular", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	if (!rt_gloss->create("rt_gloss", xres, yres, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	if (!rt_depth->create("rt_depth", xres, yres, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_UNKNOWN))
		return false;
	return true;
}

void CDeferredRender::generateGBuffer(const CCamera* camera) {
	CTraceScoped scope("gbuffer");

	ID3D11RenderTargetView* rts[6] = {
		rt_albedo->render_target_view
		, rt_normals->render_target_view
		, rt_lights->render_target_view
		, rt_depth->render_target_view
		, rt_specular->render_target_view
		, rt_gloss->render_target_view
	};

	::render.ctx->OMSetRenderTargets(6, rts, ::render.depth_stencil_view);
	rt_albedo->activateViewport();

	float black[4] = { 0.f, 0.f, 0.f, 0.f };
	rt_albedo->clearColorBuffer(black);
	rt_normals->clearColorBuffer(black);
	rt_lights->clearColorBuffer(black);
	rt_specular->clearColorBuffer(black);
	rt_gloss->clearColorBuffer(black);
	float clear_depth[4] = { 1.f, 0.f, 1.f, 1.f };
	rt_depth->clearColorBuffer(clear_depth);

	//texture_manager.getByName("Zthe_light")->activate(6);

	render_manager.renderAll(camera, true, false);
}

// ----------------------------------------------------------
void CDeferredRender::generateLightBuffer() {
	CTraceScoped scope("light_buffer");
	rt_lights->activate();
	float black[4] = { 0.0f, 0.0f, 0.0f, 0.f };
	//rt_lights->clearColorBuffer(black);

	// Activate gbuffer textures
	rt_albedo->CTexture::activate(0);
	rt_normals->CTexture::activate(1);
	rt_depth->CTexture::activate(2);
	rt_specular->CTexture::activate(4);
	rt_gloss->CTexture::activate(5);

	activateRSConfig(RSCFG_REVERSE_CULLING);
	activateZConfig(ZCFG_INVERSE_TEST_NO_WRITE);
	activateBlendConfig(BLEND_CFG_ADDITIVE);

	if (1) {
		CTraceScoped scope("point_lights");
		technique_deferred_point_lights->activate();
		getObjManager<TCompPointLight>()->onAll(&TCompPointLight::draw);
	}
	
	if (1) {
		CTraceScoped scope("dir_lights");
		technique_deferred_dir_lights->activate();
		getObjManager<TCompShadows>()->onAll(&TCompShadows::draw);
	}

	activateBlendConfig(BLEND_CFG_DEFAULT);
	activateRSConfig(RSCFG_DEFAULT);
	activateZConfig(ZCFG_DEFAULT);
}

// ----------------------------------------------------------
void CDeferredRender::resolve(const CCamera* camera, CRenderToTexture& rt_out) {
	CTraceScoped t0("resolve");

	activateZConfig(ZConfig::ZCFG_DISABLE_ALL);
	rt_lights->CTexture::activate(3);
	drawTexture2D(0, 0, rt_out.xres, rt_out.yres, rt_albedo, "deferred_resolve");
}

// ----------------------------------------------------------
void CDeferredRender::render(const CCamera* camera, CRenderToTexture& rt_out) {
	CTraceScoped scope("deferred");
	generateGBuffer(camera);
	activateCamera(*camera, 1);
	generateLightBuffer();
	
	::render.ctx->GenerateMips(rt_lights->getResourceView());

	rt_out.activate();
	resolve(camera, rt_out);
}

void CDeferredRender::destroy() {
	if (rt_lights) { rt_lights->destroyAll(); rt_lights = nullptr; }
	if (rt_albedo) { rt_albedo->destroyAll(); rt_albedo = nullptr; }
	if (rt_normals) { rt_normals->destroyAll(); rt_normals = nullptr; }
	if (rt_specular) { rt_specular->destroyAll(); rt_specular = nullptr; }
	if (rt_gloss) { rt_gloss->destroyAll(); rt_gloss = nullptr; }
	if (rt_depth) { rt_depth->destroyAll(); rt_depth = nullptr; }
	
	/*SAFE_DESTROY(rt_lights);
	SAFE_DESTROY(rt_albedo);
	SAFE_DESTROY(rt_normals);
	SAFE_DESTROY(rt_specular);
	SAFE_DESTROY(rt_gloss);
	SAFE_DESTROY(rt_depth);*/
}