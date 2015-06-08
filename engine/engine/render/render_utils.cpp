#include "mcv_platform.h"
#include "render_utils.h"
#include "camera.h"
#include "render/render_utils.h"
#include "components/comp_point_light.h"
#include "components/comp_shadows.h"

using namespace DirectX;

#include "render/ctes/shader_ctes.h"
CShaderCte<TCtesObject> ctes_object;
CShaderCte<TCtesCamera> ctes_camera;
CShaderCte<TCtesLight>  ctes_light;
CShaderCte<TCtesPointLight>  ctes_point_light;
CShaderCte<TCtesDirLight>    ctes_dir_light;
CShaderCte<TCtesSpotLight>    ctes_spot_light;
CShaderCte<TCtesBones>  ctes_bones;
CShaderCte<TCtesParticleSystem>  ctes_particle_system;

// Post process
CShaderCte<TCtesBlur> ctes_blur;
CShaderCte<TCtesSharpen> ctes_sharpen;
CShaderCte<TCtesSSAO> ctes_ssao;
CShaderCte<TCtesChromaticAberration> ctes_chromatic_aberration;
CShaderCte<TCtesGlow> ctes_glow;
CShaderCte<TCtesUnderwater> ctes_underwater;

CMesh        wire_cube;
CMesh        mesh_view_volume;
CMesh        mesh_line;
CMesh        mesh_textured_quad_xy;
CMesh        mesh_textured_quad_xy_centered;
CMesh        mesh_icosahedron;
CMesh        grid;
CMesh        axis;

bool createLine(CMesh& mesh);
bool createTexturedQuadXYCentered(CMesh& mesh);

// -----------------------
template<>
CVertexDecl* getVertexDecl< CVertexPosColor >() {
  return &vdcl_position_color;
}
template<>
CVertexDecl* getVertexDecl< CVertexPosUV >() {
	return &vdcl_position_uv;
}
template<>
CVertexDecl* getVertexDecl< CVertexPosUVNormal >() {
	return &vdcl_position_uv_normal;
}
template<>
CVertexDecl* getVertexDecl< CVertexPos >() {
	return &vdcl_position;
}

// --------------------------------------------
enum eSamplerType {
	SAMPLER_WRAP_LINEAR = 0
	, SAMPLER_CLAMP_LINEAR
	, SAMPLER_BORDER_LINEAR
	, SAMPLER_PCF_SHADOWS
	, SAMPLERS_COUNT
};

ID3D11SamplerState* all_samplers[SAMPLERS_COUNT];

bool createSamplers() {

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = ::render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_WRAP_LINEAR]);
	if (FAILED(hr))
		return false;

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = ::render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_CLAMP_LINEAR]);
	if (FAILED(hr))
		return false;

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	hr = ::render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_BORDER_LINEAR]);
	if (FAILED(hr))
		return false;

	// PCF sampling
	D3D11_SAMPLER_DESC sampler_desc = {
		D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// D3D11_FILTER Filter;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
		0,//FLOAT MipLODBias;
		0,//UINT MaxAnisotropy;
		D3D11_COMPARISON_LESS, //D3D11_COMPARISON_FUNC ComparisonFunc;
		0.0, 0.0, 0.0, 0.0,//FLOAT BorderColor[ 4 ];
		0,//FLOAT MinLOD;
		0//FLOAT MaxLOD;   
	};
	hr = ::render.device->CreateSamplerState(
		&sampler_desc, &all_samplers[SAMPLER_PCF_SHADOWS]);
	if (FAILED(hr))
		return false;

	return true;
}

void destroySamplers() {
	for (int i = 0; i < SAMPLERS_COUNT; ++i)
		SAFE_RELEASE(all_samplers[i]);
}

void activateTextureSamplers() {
	::render.ctx->PSSetSamplers(0, SAMPLERS_COUNT, all_samplers);
}

// ---------------------------------------
ID3D11DepthStencilState* z_cfgs[ZCFG_COUNT];

void activateZConfig(enum ZConfig cfg) {
	assert(z_cfgs[cfg] != nullptr);
	render.ctx->OMSetDepthStencilState(z_cfgs[cfg], 0);
}

bool createDepthStencilStates() {
	D3D11_DEPTH_STENCIL_DESC desc;
	memset(&desc, 0x00, sizeof(desc));
	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	desc.StencilEnable = FALSE;
	HRESULT hr;
	hr = render.device->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_DISABLE_ALL]);
	if (FAILED(hr))
		return false;
	setDbgName(z_cfgs[ZCFG_DISABLE_ALL], "ZCFG_DISABLE_ALL");

	// Default app, only pass those which are near than the previous samples
	memset(&desc, 0x00, sizeof(desc));
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = FALSE;
	hr = render.device->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_DEFAULT]);
	if (FAILED(hr))
		return false;
	setDbgName(z_cfgs[ZCFG_DEFAULT], "ZCFG_DEFAULT");

	// test but don't write. Used while rendering particles for example
	memset(&desc, 0x00, sizeof(desc));
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;    // don't write
	desc.DepthFunc = D3D11_COMPARISON_LESS;               // only near z
	desc.StencilEnable = FALSE;
	hr = render.device->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_TEST_BUT_NO_WRITE]);
	if (FAILED(hr))
		return false;
	setDbgName(z_cfgs[ZCFG_TEST_BUT_NO_WRITE], "ZCFG_TEST_BUT_NO_WRITE");

	// Inverse Z Test, don't write. Used while rendering the lights
	memset(&desc, 0x00, sizeof(desc));
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_GREATER;
	desc.StencilEnable = FALSE;
	hr = render.device->CreateDepthStencilState(&desc, &z_cfgs[ZCFG_INVERSE_TEST_NO_WRITE]);
	if (FAILED(hr))
		return false;
	setDbgName(z_cfgs[ZCFG_INVERSE_TEST_NO_WRITE], "ZCFG_INVERSE_TEST_NO_WRITE");

	return true;
}

void destroyDepthStencilStates() {
	for (int i = 0; i < ZCFG_COUNT; ++i)
		SAFE_RELEASE(z_cfgs[i]);
}

// -----------------------------------------------
ID3D11RasterizerState *rasterize_states[RSCFG_COUNT];

bool createRasterizationStates() {

	rasterize_states[RSCFG_DEFAULT] = nullptr;

	// Depth bias options when rendering the shadows
	D3D11_RASTERIZER_DESC desc = {
		D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
		D3D11_CULL_BACK,  // D3D11_CULL_MODE CullMode;
		FALSE,            // BOOL FrontCounterClockwise;
		13,               // INT DepthBias;
		0.0f,             // FLOAT DepthBiasClamp;
		2.0,              // FLOAT SlopeScaledDepthBias;
		TRUE,             // BOOL DepthClipEnable;
		FALSE,            // BOOL ScissorEnable;
		FALSE,            // BOOL MultisampleEnable;
		FALSE,            // BOOL AntialiasedLineEnable;
	};
	HRESULT hr = render.device->CreateRasterizerState(&desc, &rasterize_states[RSCFG_SHADOWS]);
	if (FAILED(hr))
		return false;
	setDbgName(rasterize_states[RSCFG_SHADOWS], "RS_DEPTH");

	// Culling is reversed. Used when rendering the light volumes
	D3D11_RASTERIZER_DESC rev_desc = {
		D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
		D3D11_CULL_FRONT, // D3D11_CULL_MODE CullMode;
		FALSE,            // BOOL FrontCounterClockwise;
		0,                // INT DepthBias;
		0.0f,             // FLOAT DepthBiasClamp;
		0.0,              // FLOAT SlopeScaledDepthBias;
		FALSE,            // BOOL DepthClipEnable;
		FALSE,            // BOOL ScissorEnable;
		FALSE,            // BOOL MultisampleEnable;
		FALSE,            // BOOL AntialiasedLineEnable;
	};
	hr = render.device->CreateRasterizerState(&rev_desc, &rasterize_states[RSCFG_REVERSE_CULLING]);
	if (FAILED(hr))
		return false;
	setDbgName(rasterize_states[RSCFG_REVERSE_CULLING], "RS_REVERSE_CULLING");

	return true;
}

void destroyRasterizationStates() {
	for (int i = 0; i < RSCFG_COUNT; ++i)
		SAFE_RELEASE(rasterize_states[i]);
}

void activateRSConfig(enum RSConfig cfg) {
	render.ctx->RSSetState(rasterize_states[cfg]);
}


// -----------------------------------------------
ID3D11BlendState *blend_states[BLEND_CFG_COUNT];

bool createBlendStates() {

	blend_states[BLEND_CFG_DEFAULT] = nullptr;

	D3D11_BLEND_DESC desc;
	HRESULT hr;

	// Combinative blending
	memset(&desc, 0x00, sizeof(desc));
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	hr = render.device->CreateBlendState(&desc, &blend_states[BLEND_CFG_BY_SRC_ALPHA]);
	if (FAILED(hr))
		return false;
	setDbgName(blend_states[BLEND_CFG_BY_SRC_ALPHA], "BLEND_BY_SRC_ALPHA");

	// Combinative blending
	memset(&desc, 0x00, sizeof(desc));
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	hr = render.device->CreateBlendState(&desc, &blend_states[BLEND_CFG_COMBINATIVE]);
	if (FAILED(hr))
		return false;
	setDbgName(blend_states[BLEND_CFG_COMBINATIVE], "BLEND_COMBINATIVE");

	// Combinative blending
	memset(&desc, 0x00, sizeof(desc));
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	hr = render.device->CreateBlendState(&desc, &blend_states[BLEND_CFG_COMBINATIVE_BY_SRC_ALPHA]);

	if (FAILED(hr))
		return false;
	setDbgName(blend_states[BLEND_CFG_COMBINATIVE_BY_SRC_ALPHA], "BLEND_COMBINATIVE_BY_SRC_ALPHA");

	// Additive blending
	memset(&desc, 0x00, sizeof(desc));
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;      // Color must come premultiplied
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	hr = render.device->CreateBlendState(&desc, &blend_states[BLEND_CFG_ADDITIVE]);
	if (FAILED(hr))
		return false;
	setDbgName(blend_states[BLEND_CFG_ADDITIVE], "BLEND_ADDITIVE");

	// Additive blending controlled by src alpha
	memset(&desc, 0x00, sizeof(desc));
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	hr = render.device->CreateBlendState(&desc, &blend_states[BLEND_CFG_ADDITIVE_BY_SRC_ALPHA]);
	if (FAILED(hr))
		return false;
	setDbgName(blend_states[BLEND_CFG_ADDITIVE_BY_SRC_ALPHA], "BLEND_ADDITIVE_BY_SRC_ALPHA");

	return true;
}

void destroyBlendStates() {
	for (int i = 0; i < RSCFG_COUNT; ++i)
		SAFE_RELEASE(blend_states[i]);
}

void activateBlendConfig(enum BlendConfig cfg) {
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };    // Not used
	UINT sampleMask = 0xffffffff;
	render.ctx->OMSetBlendState(blend_states[cfg], blendFactor, sampleMask);
}

// -----------------------------------------------
bool renderUtilsCreate() {
	bool is_ok = ctes_object.create();
	is_ok &= ctes_camera.create();
	is_ok &= ctes_light.create();
	is_ok &= ctes_point_light.create();
	is_ok &= ctes_dir_light.create();
	is_ok &= ctes_spot_light.create();
	is_ok &= ctes_bones.create();
	is_ok &= ctes_particle_system.create();

	ctes_object.activateInVS(0);
	ctes_camera.activateInVS(1);
	ctes_camera.activateInPS(1);

	is_ok &= ctes_blur.create();
	is_ok &= ctes_sharpen.create();
	is_ok &= ctes_ssao.create();
	is_ok &= ctes_chromatic_aberration.create();
	is_ok &= ctes_glow.create();
	is_ok &= ctes_underwater.create();	

	is_ok &= createGrid(grid, 10);
	is_ok &= createAxis(axis);
	is_ok &= createWiredCube(wire_cube);
	is_ok &= createViewVolume(mesh_view_volume);
	is_ok &= createLine(mesh_line);
	is_ok &= createTexturedQuadXY(mesh_textured_quad_xy);
	is_ok &= createTexturedQuadXYCentered(mesh_textured_quad_xy_centered);
	is_ok &= createIcosahedron(mesh_icosahedron);
	is_ok &= createSamplers();
	is_ok &= createDepthStencilStates();
	is_ok &= createRasterizationStates();
	is_ok &= createBlendStates();
	return is_ok;
}

void renderUtilsDestroy() {
	destroyBlendStates();
	destroyRasterizationStates();
	destroyDepthStencilStates();
	destroySamplers();
	ctes_object.destroy();
	ctes_dir_light.destroy();
	ctes_point_light.destroy();
	ctes_spot_light.destroy();
	ctes_light.destroy();
	ctes_camera.destroy();
	ctes_bones.destroy();
	ctes_particle_system.destroy();

	ctes_blur.destroy();
	ctes_sharpen.destroy();
	ctes_ssao.destroy();
	ctes_chromatic_aberration.destroy();
	ctes_glow.destroy();
	ctes_underwater.destroy();
	
	axis.destroy();
	grid.destroy();
	mesh_line.destroy();
	mesh_textured_quad_xy.destroy();
	mesh_textured_quad_xy_centered.destroy();
	mesh_view_volume.destroy();
	mesh_icosahedron.destroy();
	wire_cube.destroy();
}

void activateWorldMatrix( int slot ) {
  ctes_object.activateInVS(slot);
}

void activateTint(int slot) {
	ctes_object.activateInPS(slot);
}

void activateCamera(const CCamera& camera, int slot) {
	ctes_camera.activateInVS(slot);    // as set in the shader.fx!!
	ctes_camera.activateInPS(slot);    // as set in the shader.fx!!
	ctes_camera.get()->ViewProjection = camera.getViewProjection();
	ctes_camera.get()->cameraView = camera.getView();
	ctes_camera.get()->cameraWorldPos = camera.getPosition();
	ctes_camera.get()->cameraWorldFront = camera.getFront();
	ctes_camera.get()->cameraWorldLeft = -camera.getRight();
	ctes_camera.get()->cameraWorldUp = camera.getUp();
	ctes_camera.get()->cameraZFar = camera.getZFar();
	ctes_camera.get()->cameraZNear = camera.getZNear();
	ctes_camera.get()->cameraDummy1 = 0.f;
	
	D3D11_VIEWPORT vp = camera.getViewport();
	ctes_camera.get()->cameraHalfXRes = vp.Width / 2.f;
	ctes_camera.get()->cameraHalfYRes = vp.Height / 2.f;

	// tan( fov/2 ) = ( yres/2 ) / view_d
	ctes_camera.get()->cameraViewD = (vp.Height / 2.f) / tanf(camera.getFov() * 0.5f);

	ctes_camera.uploadToGPU();
}

void activateLight(const CCamera& light, int slot) {
	ctes_light.activateInVS(slot);    // as set in the shader.fx!!
	ctes_light.activateInPS(slot);    // as set in the shader.fx!!
	ctes_light.get()->LightWorldPos = light.getPosition();
	ctes_light.get()->LightViewProjection = light.getViewProjection();
	XMMATRIX offset = XMMatrixTranslation(0.5f, 0.5f, 0.f);
	XMMATRIX scale = XMMatrixScaling(0.5f, -0.5f, 1.f);
	XMMATRIX tmx = scale * offset;
	ctes_light.get()->LightViewProjectionOffset = light.getViewProjection() * tmx;
	ctes_light.uploadToGPU();
}

void activatePointLight(const TCompPointLight* plight, XMVECTOR light_pos, int slot) {
	ctes_point_light.activateInVS(slot);    // as set in the deferred.fx!!
	ctes_point_light.activateInPS(slot);    // as set in the deferred.fx!!
	TCtesPointLight *c = ctes_point_light.get();
	c->plight_color = plight->color;
	c->plight_world_pos = light_pos;
	c->plight_max_radius = plight->radius;
	c->plight_inv_delta_radius = 1.0f / (plight->radius - plight->radius * plight->decay_factor);
	ctes_point_light.uploadToGPU();
}

void activateDirLight(const TCompShadows* dir_light, XMVECTOR light_pos, XMVECTOR light_dir, float angle, int slot) {
	ctes_dir_light.activateInVS(slot);    // as set in the deferred.fx!!
	ctes_dir_light.activateInPS(slot);    // as set in the deferred.fx!!
	TCtesDirLight *c = ctes_dir_light.get();
	c->dir_light_color = dir_light->color;
	c->dir_light_world_pos = light_pos;
	c->dir_light_direction = light_dir;
	c->dir_light_angle = angle;
	//c->dir_light_max_radius = dir_light->radius;
	//c->dir_light_inv_delta_radius = 1.0f / (plight->radius - plight->radius * plight->decay_factor);
	ctes_dir_light.uploadToGPU();
}


// -----------------------------------------------------
bool createGrid(CMesh& mesh, int nsamples) {

  std::vector< CVertexPosColor > vtxs;

  XMFLOAT4 c2 = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
  XMFLOAT4 c1 = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

  float fsamples = (float)nsamples;
  CVertexPosColor v;

  // The lines from -x to +x
  for (int z = -nsamples; z <= nsamples; ++z) {
    v.Color = (z % 5) ? c2 : c1;
    v.Pos = XMFLOAT3(-fsamples, 0.f, (float)z);
    vtxs.push_back(v);
    v.Pos = XMFLOAT3(fsamples, 0.f, (float)z);
    vtxs.push_back(v);
  }  

  // The lines from -z to +z
  for (int x = -nsamples; x <= nsamples; ++x) {
    v.Color = (x % 5) ? c2 : c1;
    v.Pos = XMFLOAT3((float)x, 0.f, -fsamples);
    vtxs.push_back(v);
    v.Pos = XMFLOAT3((float)x, 0.f, fsamples);
    vtxs.push_back(v);
  }

  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createAxis(CMesh& mesh) {

  std::vector< CVertexPosColor > vtxs;
  vtxs.resize(6);
  CVertexPosColor *v = &vtxs[0];

  // Axis X
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(1.f, 0.f, 0.f); v->Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f); ++v;

  // Axis Y
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 2.f, 0.f); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;

  // Axis Z
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 3.f); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;

  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createViewVolume(CMesh& mesh) {

	std::vector< CVertexPos > vtxs;
	vtxs.resize(8);
	CVertexPos *v = &vtxs[0];

	// Axis X
	v->Pos = XMFLOAT3(-1.f, -1.f, 0.f); ++v;
	v->Pos = XMFLOAT3(1.f, -1.f, 0.f); ++v;
	v->Pos = XMFLOAT3(-1.f, 1.f, 0.f); ++v;
	v->Pos = XMFLOAT3(1.f, 1.f, 0.f); ++v;
	v->Pos = XMFLOAT3(-1.f, -1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.f, -1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-1.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.f, 1.f, 1.f); ++v;

	const CMesh::TIndex idxs[] = {
		0, 2, 1, 1, 2, 3,
		4, 5, 6, 5, 7, 6,
		0, 1, 4, 1, 5, 4,
		2, 6, 7, 2, 7, 3,
		0, 4, 2, 4, 6, 2,
		1, 3, 5, 5, 3, 7,
	};
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], sizeof(idxs) / sizeof(CMesh::TIndex), idxs, CMesh::TRIANGLE_LIST);
}


// -----------------------------------------------------
bool createTexturedQuadXY(CMesh& mesh) {
	std::vector< CVertexPosUV > vtxs;
	vtxs.resize(4);
	CVertexPosUV *v = &vtxs[0];
	v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->UV = XMFLOAT2(0, 0); ++v;
	v->Pos = XMFLOAT3(1.f, 0.f, 0.f); v->UV = XMFLOAT2(1, 0); ++v;
	v->Pos = XMFLOAT3(0.f, 1.f, 0.f); v->UV = XMFLOAT2(0, 1); ++v;
	v->Pos = XMFLOAT3(1.f, 1.f, 0.f); v->UV = XMFLOAT2(1, 1); ++v;
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::TRIANGLE_STRIP);
}

// -----------------------------------------------------
bool createTexturedQuadXYCentered(CMesh& mesh) {
	std::vector< CVertexPosUV > vtxs;
	vtxs.resize(4);
	CVertexPosUV *v = &vtxs[0];
	v->Pos = XMFLOAT3(-0.5f, -0.5f, 0.f); v->UV = XMFLOAT2(0, 0); ++v;
	v->Pos = XMFLOAT3(0.5f, -0.5f, 0.f); v->UV = XMFLOAT2(1, 0); ++v;
	v->Pos = XMFLOAT3(-0.5f, 0.5f, 0.f); v->UV = XMFLOAT2(0, 1); ++v;
	v->Pos = XMFLOAT3(0.5f, 0.5f, 0.f); v->UV = XMFLOAT2(1, 1); ++v;
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::TRIANGLE_STRIP);
}

// -----------------------------------------------------
bool createLine(CMesh& mesh) {

	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(2);
	CVertexPosColor *v = &vtxs[0];

	v->Pos = XMFLOAT3(0, 0, 0.f);
	v->Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	++v;
	v->Pos = XMFLOAT3(0, 0, 1.f);
	v->Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	++v;

	return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createWiredCube(CMesh& mesh) {

  std::vector< CVertexPosColor > vtxs;
  vtxs.resize(8);
  CVertexPosColor *v = &vtxs[0];

  // Set the color to white
  for (auto& it : vtxs)
    it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

  // Axis X
  v->Pos = XMFLOAT3(-1.f, -1.f, 0.f); ++v;
  v->Pos = XMFLOAT3( 1.f, -1.f, 0.f); ++v;
  v->Pos = XMFLOAT3(-1.f,  1.f, 0.f); ++v;
  v->Pos = XMFLOAT3( 1.f,  1.f, 0.f); ++v;
  v->Pos = XMFLOAT3(-1.f, -1.f, 1.f); ++v;
  v->Pos = XMFLOAT3( 1.f, -1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(-1.f,  1.f, 1.f); ++v;
  v->Pos = XMFLOAT3( 1.f,  1.f, 1.f); ++v;

  const CMesh::TIndex idxs[] = {
    0, 1, 2, 3, 4, 5, 6, 7
    , 0, 2, 1, 3, 4, 6, 5, 7
    , 0, 4, 1, 5, 2, 6, 3, 7
  };
  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 24, idxs, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createIcosahedron(CMesh& mesh) {
#define X (.525731112119133606f*1.2584085f)
#define Z (.850650808352039932f*1.2584085f)
	static const float vs[12][3] = {
		{ -X, 0.0, Z }, { X, 0.0, Z }, { -X, 0.0, -Z }, { X, 0.0, -Z },
		{ 0.0, Z, X }, { 0.0, Z, -X }, { 0.0, -Z, X }, { 0.0, -Z, -X },
		{ Z, X, 0.0 }, { -Z, X, 0.0 }, { Z, -X, 0.0 }, { -Z, -X, 0.0 }
	};
#undef X
#undef Y
	static const CMesh::TIndex idxs[20][3] = {
		{ 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
		{ 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
		{ 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
		{ 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 } };
	return mesh.create(12, (const CVertexPos*)vs, 20 * 3, &idxs[0][0], CMesh::TRIANGLE_LIST);
}

// -----------------------------------------------------
bool createUnitWiredCube(CMesh& mesh, XMFLOAT4 color) {

	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(8);
	CVertexPosColor *v = &vtxs[0];

	// Set the color to white
	for (auto& it : vtxs)
		it.Color = color;

	// Axis X
	v->Pos = XMFLOAT3(-.5f, -.5f, -.5f); ++v;
	v->Pos = XMFLOAT3(.5f, -.5f, -.5f); ++v;
	v->Pos = XMFLOAT3(-.5f, .5f, -.5f); ++v;
	v->Pos = XMFLOAT3(.5f, .5f, -.5f); ++v;
	v->Pos = XMFLOAT3(-.5f, -.5f, .5f); ++v;
	v->Pos = XMFLOAT3(.5f, -.5f, .5f); ++v;
	v->Pos = XMFLOAT3(-.5f, .5f, .5f); ++v;
	v->Pos = XMFLOAT3(.5f, .5f, .5f); ++v;

	const CMesh::TIndex idxs[] = {
		0, 1, 2, 3, 4, 5, 6, 7
		, 0, 2, 1, 3, 4, 6, 5, 7
		, 0, 4, 1, 5, 2, 6, 3, 7
	};
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], 24, idxs, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createCube(CMesh& mesh, float size) {

	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(8);
	CVertexPosColor *v = &vtxs[0];

	// Top
	v->Pos = XMFLOAT3(-1.0f * size, 1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, 1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, 1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-1.0f * size, 1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;

	// Bottom
	v->Pos = XMFLOAT3(-1.0f * size, -1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, -1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, -1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-1.0f * size, -1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;

	// Indices
	CMesh::TIndex indices[] =
	{
		3, 0, 1,
		2, 3, 1,

		0, 4, 5,
		1, 0, 5,

		3, 7, 4,
		0, 3, 4,

		1, 5, 6,
		2, 1, 6,

		2, 6, 7,
		3, 2, 7,

		6, 5, 4,
		7, 6, 4,
	};

	return mesh.create((unsigned)vtxs.size(), &vtxs[0], ARRAYSIZE(indices), indices, CMesh::TRIANGLE_LIST);
}

// -----------------------------------------------------
bool createCamera(CMesh& mesh)
{
	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(16);
	CVertexPosColor *v = &vtxs[0];

	// Set the color to white
	for (auto& it : vtxs)
		it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	// Top
	v->Pos = XMFLOAT3(-.25f, .25f, -1.0f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, .25f, -1.0f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, .25f, .7f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.25f, .25f, .7f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;

	// Bottom
	v->Pos = XMFLOAT3(-.25f, -0.25f, -1.0f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, -0.25f, -1.0f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, -0.25f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.25f, -0.25f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;

	// Camera front
	v->Pos = XMFLOAT3(.2f, 0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.2f, 0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.2f, -0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.2f, -0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;

	v->Pos = XMFLOAT3(.5f, .5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.5f, .5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.5f, -0.5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.5f, -0.5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;


	const CMesh::TIndex idxs[] = {
		3, 0, 1,
		2, 3, 1,

		0, 4, 5,
		1, 0, 5,

		3, 7, 4,
		0, 3, 4,

		1, 5, 6,
		2, 1, 6,

		6, 5, 4,
		7, 6, 4,

		9, 7, 3,
		9, 11, 7,

		2, 8, 9,
		2, 9, 3,

		2, 6, 10,
		2, 10, 8,

		6, 7, 10,
		10, 7, 11,

		8, 10, 14,
		8, 14, 12,

		8, 12, 9,
		9, 12, 13,

		9, 13, 11,
		11, 13, 15,

		14, 10, 15,
		15, 10, 11,

		12, 14, 13,
		13, 14, 15

	};
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], ARRAYSIZE(idxs), idxs, CMesh::TRIANGLE_LIST);
}

// -----------------------------------------------------
bool createString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension)
{
	float dist = XMVectorGetX(XMVector3Length(initialPos - finalPos));

	const int epsilon = 25;
	CVertexPosColor ropeVertices[epsilon];
	float y = 0;
	float pow_r = 2;	// Suavidad de la onda
	int velocity = 10;	// Velocidad de movimiento
	float wave_freq = 0.9f;	// frecuencia del ruido de la onda
	float amplitude = 0.05f;	// amplitud del ruido de la onda
	float elapsed = 1 / 60.0f;

	for (int i = 0; i < epsilon; i++)
	{
		XMVECTOR midPos = XMVectorLerp(initialPos, finalPos, (float)i / (epsilon - 1));

		if (i < epsilon / 2)
			y = (float) (pow(epsilon - i, pow_r) / pow(epsilon, pow_r));
		else
			y = (float) (pow(i, pow_r) / pow(epsilon, pow_r));

		float noise = (sin(elapsed*velocity + i*wave_freq) + 1) * amplitude;
		if (i != 0 && i != epsilon - 1)
			y += noise * noise;

		y -= 1;

		y *= tension;

		ropeVertices[i].Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		ropeVertices[i].Pos = XMFLOAT3(XMVectorGetX(midPos), XMVectorGetY(midPos) + y, XMVectorGetZ(midPos));
	}

	ropeVertices[0].Pos = XMFLOAT3(XMVectorGetX(initialPos), XMVectorGetY(initialPos), XMVectorGetZ(initialPos));;
	ropeVertices[epsilon - 1].Pos = XMFLOAT3(XMVectorGetX(finalPos), XMVectorGetY(finalPos), XMVectorGetZ(finalPos));;

	XMFLOAT3 a = ropeVertices[epsilon - 1].Pos;	

	return mesh.create(epsilon, ropeVertices, 0, NULL, CMesh::LINE_LIST_ADJ, &vdcl_position_color);
}

bool createFullString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension, float width)
{
	float dist = XMVectorGetX(XMVector3Length(initialPos - finalPos));

	const int epsilon = 15;
	const int sizes = 4;
	//float width = 0.1;
	XMFLOAT3 ropeReferences[epsilon];
	CVertexPosUVNormal ropeVertices[epsilon * sizes];
	CMesh::TIndex ropeIndices[(epsilon - 1) * sizes * 6];
	float y = 0;
	float pow_r = 2;	// Suavidad de la onda
	int velocity = 10;	// Velocidad de movimiento
	float wave_freq = 0.9f;	// frecuencia del ruido de la onda
	float amplitude = 0.05f;	// amplitud del ruido de la onda
	float elapsed = 1 / 60.0f;

	for (int i = 0; i < epsilon; i++)
	{
		XMVECTOR midPos = XMVectorLerp(initialPos, finalPos, (float)i / (epsilon - 1));

		if (i < epsilon / 2)
			y = (float) (pow(epsilon - i, pow_r) / pow(epsilon, pow_r));
		else
			y = (float) (pow(i, pow_r) / pow(epsilon, pow_r));

		float noise = (sin(elapsed*velocity + i*wave_freq) + 1) * amplitude;
		if (i != 0 && i != epsilon - 1)
			y += noise * noise;

		y -= 1;

		y *= tension;
		// Guardar la referencia de posición de la cuerda
		ropeReferences[i] = XMFLOAT3(XMVectorGetX(midPos), XMVectorGetY(midPos) + y, XMVectorGetZ(midPos));
	}

	ropeReferences[0] = XMFLOAT3(XMVectorGetX(initialPos), XMVectorGetY(initialPos), XMVectorGetZ(initialPos));;
	ropeReferences[epsilon - 1] = XMFLOAT3(XMVectorGetX(finalPos), XMVectorGetY(finalPos), XMVectorGetZ(finalPos));;

	// Ángulo que hay entre cada lado
	const float angle = deg2rad( 360 / sizes);
	XMVECTOR normal;
	XMVECTOR dir;
	for (int i = 0; i < epsilon; i++)
	{
		// Calcular la normal del punto de referencia
		// Caso inicial: la normal es el vector ortogonal a la dirección desde el segundo al primer punto de referencia
		if (i == 0) {
			dir = XMVectorSet(
				ropeReferences[i + 1].x
				, ropeReferences[i + 1].y
				, ropeReferences[i + 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir = XMVector3Normalize(dir);
			normal = XMVector3Orthogonal(dir);
		}
		// Caso final: la normal es el vector ortogonal a la dirección desde el penúltimo al último punto de referencia
		else if (i == epsilon - 1) {
			dir = XMVectorSet(
				ropeReferences[i - 1].x
				, ropeReferences[i - 1].y
				, ropeReferences[i - 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir = XMVector3Normalize(dir);
			normal = XMVector3Orthogonal(dir);
		}
		// Resto de casos: la normal es la media del anterior y el siguiente
		else
		{
			XMVECTOR dir1 = XMVectorSet(
				ropeReferences[i - 1].x
				, ropeReferences[i - 1].y
				, ropeReferences[i - 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir1 = XMVector3Normalize(dir1);
			XMVECTOR normal1 = XMVector3Cross(dir1, XMVectorSet(0, 1, 0, 0));
			normal1 = XMVector3Orthogonal(dir1);
			XMVECTOR dir2 = XMVectorSet(
				ropeReferences[i - 1].x
				, ropeReferences[i - 1].y
				, ropeReferences[i - 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir2 = XMVector3Normalize(dir2);
			XMVECTOR normal2 = XMVector3Cross(dir2, XMVectorSet(0, 1, 0, 0));
			normal2 = XMVector3Orthogonal(dir1);
			dir = (dir1 + dir2) / 2;
			normal = (normal1 + normal2) / 2;
		}
		// Crear los vértices extra, en forma de hexágono
		for (int j = 0; j < sizes; j++) {
			// Posiciones extra, siguiendo un círculo
			// Se rota la normal en el eje de dirección
			XMVECTOR quat = XMQuaternionRotationAxis(dir, angle * j);
			XMVECTOR pos = normal * width;
			pos = XMVector3Rotate(pos, quat);

			ropeVertices[i * sizes + j].Pos = XMFLOAT3(ropeReferences[i].x + XMVectorGetX(pos), ropeReferences[i].y + XMVectorGetY(pos), ropeReferences[i].z + XMVectorGetZ(pos));
			float uvy = i % 2 == 0.f ? 0.f : 1.f;
			float uvx = (1.0f / sizes) * j;
			ropeVertices[i * sizes + j].UV = XMFLOAT2(uvx, uvy);
			XMVECTOR realNormal = XMVector3Normalize(pos);
			ropeVertices[i * sizes + j].Normal = XMFLOAT3(XMVectorGetX(realNormal), XMVectorGetY(realNormal), XMVectorGetZ(realNormal));
		}		
	}

	// Índices
	int offset = 0;
	int inner_offset = 0;
	for (int i = 0; i < epsilon - 1; i++) {
		for (int j = 0; j < sizes; j++) {
			offset = i * sizes * 6 + j * 6;
			inner_offset = i * sizes;
			// Último caso (cierre)
			if (j == sizes - 1) {
				ropeIndices[offset + 0] = inner_offset + j;
				ropeIndices[offset + 1] = inner_offset + j + (sizes + 1) - sizes;
				ropeIndices[offset + 2] = inner_offset + j + sizes;

				ropeIndices[offset + 3] = inner_offset + j;
				ropeIndices[offset + 4] = inner_offset + j + 1 - sizes;
				ropeIndices[offset + 5] = inner_offset + j + (sizes + 1) - sizes;
			}
			else {
				ropeIndices[offset + 0] = inner_offset + j;
				ropeIndices[offset + 1] = inner_offset + j + (sizes + 1);
				ropeIndices[offset + 2] = inner_offset + j + sizes;

				ropeIndices[offset + 3] = inner_offset + j;
				ropeIndices[offset + 4] = inner_offset + j + 1;
				ropeIndices[offset + 5] = inner_offset + j + (sizes + 1);
			}
		}
	}

	XMFLOAT3 a = ropeVertices[epsilon - 1].Pos;

	return mesh.create(epsilon * sizes, ropeVertices, (epsilon - 1) * sizes * 6, ropeIndices, CMesh::TRIANGLE_LIST, &vdcl_position_uv_normal);
	//return mesh.create(epsilon * sizes, ropeVertices, 0, NULL, CMesh::LINE_LIST_ADJ, &vdcl_position_color);
}

// -----------------------------------------------------
void drawViewVolume(const CCamera& camera) {
  XMVECTOR det;
  for (int i = 0; i < 10; ++i) {
    float ratio = (i + 1) / 10.0f;
    XMMATRIX scale = XMMatrixScaling(1.f, 1.f, ratio);
    XMMATRIX inv_view_proj = XMMatrixInverse(&det, camera.getViewProjection());
    ctes_object.get()->World = scale * inv_view_proj;
    ctes_object.uploadToGPU();
    wire_cube.activateAndRender();
  }
}

void drawLine(XMVECTOR src, XMVECTOR target) {
	XMMATRIX mtx = XMMatrixIdentity();
	XMVECTOR delta = target - src;

	XMVectorSetW(delta, 0.f);
	XMVectorSetW(src, 1.f);
	mtx.r[2] = delta;
	mtx.r[3] = src;
	ctes_object.get()->World = mtx;
	ctes_object.uploadToGPU();
	mesh_line.activateAndRender();
}

void setWorldMatrix(XMMATRIX world) {
  ctes_object.get()->World = world;
  ctes_object.uploadToGPU();
}

void setTint(XMVECTOR tint) {
	ctes_object.get()->Tint = tint;
	ctes_object.uploadToGPU();
}

// -----------------------------------------------
void drawTexture2D(int x0, int y0, int w, int h, const CTexture* texture, const char *tech_name) {

	if (tech_name == nullptr)
		tech_name = "textured";

	render_techniques_manager.getByName(tech_name)->activate();

	// Activate the texture
	texture->activate(0);

	// Activate a ortho camera view projection matrix
	XMMATRIX prev_view_proj = ctes_camera.get()->ViewProjection;
	ctes_camera.get()->ViewProjection = XMMatrixOrthographicOffCenterRH(
		0, render.xres,
		render.yres, 0.f,
		-1.f, 1.f);
	ctes_camera.uploadToGPU();

	// Update the world matrix to match the params
	ctes_object.get()->World = XMMatrixScaling(w, h, 1) * XMMatrixTranslation(x0, y0, 0);
	ctes_object.uploadToGPU();

	mesh_textured_quad_xy.activateAndRender();

	// Restore old view proj
	ctes_camera.get()->ViewProjection = prev_view_proj;
	ctes_camera.uploadToGPU();
}

void drawTexture3D(CCamera& camera, XMVECTOR world_p3d, int w, int h, const CTexture* texture, const char* tech_name){

	float x, y;
	if (camera.getScreenCoords(world_p3d, &x, &y)) {

		if (tech_name == nullptr)
			tech_name = "textured";

		render_techniques_manager.getByName(tech_name)->activate();

		// Activate the texture
		texture->activate(0);

		// Activate a ortho camera view projection matrix
		XMMATRIX prev_view_proj = ctes_camera.get()->ViewProjection;
		ctes_camera.get()->ViewProjection = XMMatrixOrthographicOffCenterRH(
			0, render.xres,
			render.yres, 0.f,
			-1.f, 1.f);
		ctes_camera.uploadToGPU();

		// Update the world matrix to match the params
		ctes_object.get()->World = XMMatrixScaling(w, h, 1) * XMMatrixTranslation(x, y, 0);
		ctes_object.uploadToGPU();

		mesh_textured_quad_xy.activateAndRender();

		// Restore old view proj
		ctes_camera.get()->ViewProjection = prev_view_proj;
		ctes_camera.uploadToGPU();
	}
}

void drawTexture3DDynamic(CCamera& camera, XMVECTOR world_p3d, int w, int h, const CTexture* texture, const char* tech_name){

	float dist = V3DISTANCE(camera.getPosition(), world_p3d);
	dist = 1 / dist;

	float x, y;
	if (camera.getScreenCoords(world_p3d, &x, &y)) {

		if (tech_name == nullptr)
			tech_name = "textured";

		render_techniques_manager.getByName(tech_name)->activate();

		// Activate the texture
		texture->activate(0);

		// Activate a ortho camera view projection matrix
		XMMATRIX prev_view_proj = ctes_camera.get()->ViewProjection;
		ctes_camera.get()->ViewProjection = XMMatrixOrthographicOffCenterRH(
			0, render.xres,
			render.yres, 0.f,
			-1.f, 1.f);
		ctes_camera.uploadToGPU();

		// Update the world matrix to match the params
		ctes_object.get()->World = XMMatrixScaling(w * dist, h * dist, 1) * XMMatrixTranslation(x, y, 0);
		ctes_object.uploadToGPU();

		mesh_textured_quad_xy.activateAndRender();

		// Restore old view proj
		ctes_camera.get()->ViewProjection = prev_view_proj;
		ctes_camera.uploadToGPU();
	}
}