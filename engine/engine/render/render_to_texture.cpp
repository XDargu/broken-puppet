#include "mcv_platform.h"
#include "render_to_texture.h"

CRenderToTexture::CRenderToTexture()
	: name(nullptr)
	, render_target_view(nullptr)
	, depth_stencil_view(nullptr)
{ }

// ------------------------------------------------
bool CRenderToTexture::create(
	const char* aname
	, int axres
	, int ayres
	, DXGI_FORMAT acolor_fmt
	, DXGI_FORMAT adepth_fmt
	, TZBufferType zbuffer_type
	, bool mipmaps
	) {

	// Save the params
	name = aname;
	xres = axres;
	yres = ayres;
	color_fmt = acolor_fmt;
	depth_fmt = adepth_fmt;

	// Use the name for the debug name

	if (color_fmt != DXGI_FORMAT_UNKNOWN)
		if (!createColorBuffer(mipmaps))
			return false;

	// Create ZBuffer
	if (zbuffer_type == USE_OWN_ZBUFFER) {
		if (depth_fmt != DXGI_FORMAT_UNKNOWN)
			if (!createDepthBuffer())
				return false;
	}
	else if (zbuffer_type == NO_ZBUFFER) {
		depth_stencil_view = nullptr;

	}
	else {
		assert(zbuffer_type == USE_BACK_ZBUFFER);
		depth_stencil_view = render.depth_stencil_view;
		depth_stencil_view->AddRef();
	}
	
	// Registrarlo en el texturemanager
	return texture_manager.registerNew(name, this);

	//return true;
}

// ------------------------------------------------
bool CRenderToTexture::createColorBuffer(bool mipmaps) {
	// Create a color surface
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = xres;
	desc.Height = yres;
	desc.MipLevels = mipmaps ? 0 : 1;
	desc.ArraySize = 1;
	desc.Format = color_fmt;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = mipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	HRESULT hr = render.device->CreateTexture2D(&desc, NULL, &resource);
	if (FAILED(hr))
		return false;

	// Create the render target view
	hr = render.device->CreateRenderTargetView(resource, NULL, &render_target_view);
	if (FAILED(hr))
		return false;
	setDbgName(render_target_view, name);
	// Create a resource view so we can use it in the shaders as input
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	memset(&SRVDesc, 0, sizeof(SRVDesc));
	SRVDesc.Format = color_fmt;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = mipmaps ? -1 : desc.MipLevels;
	hr = render.device->CreateShaderResourceView(resource, &SRVDesc, &resource_view);
	if (FAILED(hr))
		return false;

	
	return true;
}

// ------------------------------------------------
bool CRenderToTexture::createDepthBuffer() {
	ID3D11Device* device = render.device;
	//assert(ztexture.resource_view == NULL);
	assert(depth_fmt != DXGI_FORMAT_UNKNOWN);
	assert(depth_fmt == DXGI_FORMAT_R32_TYPELESS
		|| depth_fmt == DXGI_FORMAT_R24G8_TYPELESS
		|| depth_fmt == DXGI_FORMAT_R16_TYPELESS
		|| depth_fmt == DXGI_FORMAT_D24_UNORM_S8_UINT
		|| depth_fmt == DXGI_FORMAT_R8_TYPELESS);

	D3D11_TEXTURE2D_DESC          depthBufferDesc;

	// Init depth and stencil buffer
	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = xres;
	depthBufferDesc.Height = yres;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = depth_fmt;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	//  if (bind_shader_resource)
	depthBufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
	DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;       // Stencil format
	DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;       // Depth format

	switch (depth_fmt) {
	case DXGI_FORMAT_R32_TYPELESS:
		SRVfmt = DXGI_FORMAT_R32_FLOAT;
		DSVfmt = DXGI_FORMAT_D32_FLOAT;
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		SRVfmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		DSVfmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case DXGI_FORMAT_R16_TYPELESS:
		SRVfmt = DXGI_FORMAT_R16_UNORM;
		DSVfmt = DXGI_FORMAT_D16_UNORM;
		break;
	case DXGI_FORMAT_R8_TYPELESS:
		SRVfmt = DXGI_FORMAT_R8_UNORM;
		DSVfmt = DXGI_FORMAT_R8_UNORM;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		SRVfmt = depthBufferDesc.Format;
		DSVfmt = depthBufferDesc.Format;
		break;
	}

	// Create the texture for the de  pth buffer using the filled out description.
	ID3D11Texture2D* ztexture2d;
	HRESULT hr = device->CreateTexture2D(&depthBufferDesc, NULL, &ztexture2d);
	if (FAILED(hr))
		return false;

	// Initialize the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DSVfmt;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	hr = device->CreateDepthStencilView(ztexture2d, &depthStencilViewDesc, &depth_stencil_view);
	if (FAILED(hr))
		return false;
	std::string m_name = "ZTextureDSV_" + std::string(name);
	setDbgName(depth_stencil_view, m_name.c_str());

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = SRVfmt;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = depthBufferDesc.MipLevels;

	// Create the shader resource view.
	ID3D11ShaderResourceView* depth_resource_view = nullptr;
	hr = device->CreateShaderResourceView(ztexture2d, &shaderResourceViewDesc, &depth_resource_view);
	if (FAILED(hr))
		return false;

	ztexture = new CTexture();
	ztexture->resource_view = depth_resource_view;
	ztexture->setName(("Z" + std::string(name)).c_str());
	setDbgName(ztexture->resource_view, ztexture->getName());
	texture_manager.registerNew(ztexture->getName(), ztexture);
	SAFE_RELEASE(ztexture2d);
	return true;
}


void CRenderToTexture::activate() {

	// OJO!! con el ZBUFFER del backbuffer! la resolucion tiene que ser la misma!!
	render.ctx->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

	activateViewport();
}

void CRenderToTexture::activateViewport() {
	// activate the associated viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)xres;
	vp.Height = (FLOAT)yres;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	render.ctx->RSSetViewports(1, &vp);
}

void CRenderToTexture::clearColorBuffer(const FLOAT ColorRGBA[4]) {
	assert(render_target_view);
	::render.ctx->ClearRenderTargetView(render_target_view, ColorRGBA);
}

void CRenderToTexture::clearDepthBuffer() {
	assert(depth_stencil_view);
	::render.ctx->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CRenderToTexture::destroy() {
	//texture_manager.unregister(name);
	SAFE_RELEASE(render_target_view);
	SAFE_RELEASE(depth_stencil_view);
	/*if (ztexture != nullptr)
		ztexture->destroy(), ztexture = nullptr;*/
	CTexture::destroy();
	name = nullptr;
}