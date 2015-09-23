#include "mcv_platform.h"

// The global render object
CRender render;

// Ctor just clear everything
CRender::CRender()
	: device(nullptr)
	, ctx(nullptr)
	, swap_chain(nullptr)
	, render_target_view(nullptr)
	, depth_stencil(nullptr)
	, depth_stencil_view(nullptr)
{ }

// --------------------------------------
bool CRender::createDevice() {

	HRESULT hr = S_OK;
	CApp& app = CApp::get();

	RECT rc;
	GetClientRect(app.hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
	#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		//D3D_FEATURE_LEVEL_10_1,
		//D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = app.hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		D3D_DRIVER_TYPE g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &swap_chain, &device, &featureLevel, &ctx);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return false;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return false;

	hr = device->CreateRenderTargetView(pBackBuffer, NULL, &render_target_view);
	pBackBuffer->Release();
	if (FAILED(hr))
		return false;

	// ------------------------------------
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = device->CreateTexture2D(&descDepth, NULL, &depth_stencil);
	if (FAILED(hr))
		return false;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depth_stencil, &descDSV, &depth_stencil_view);
	if (FAILED(hr))
		return false;

	xres = width;
	yres = height;

	activateBackbuffer();

	return true;
}

// --------------------------------------
void CRender::activateBackbuffer() {
	ctx->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)xres;
	vp.Height = (FLOAT)yres;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ctx->RSSetViewports(1, &vp);
}

// --------------------------------------
void CRender::destroyDevice() {
	if (ctx) ctx->ClearState();
	SAFE_RELEASE(depth_stencil_view);
	SAFE_RELEASE(depth_stencil);
	SAFE_RELEASE(render_target_view);
	SAFE_RELEASE(swap_chain);
	SAFE_RELEASE(ctx);

	ID3D11Debug *d3dDebug = nullptr;
	::render.device->QueryInterface(__uuidof(ID3D11Debug), (void**)(&d3dDebug));
	d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);

	SAFE_RELEASE(device);
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
bool CRender::compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// To avoid having to transpose the matrix in the ctes buffers when
	// uploading matrix to the gpu, and to keep multiplying vector * matrix
	// in the shaders instead of matrix * vector (as in gles20)
	dwShaderFlags |= D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;

	wchar_t wFilename[MAX_PATH];
	mbstowcs(wFilename, szFileName, MAX_PATH);

	while (true) {

		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(wFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

		if (pErrorBlob) {
			dbg("Compiling %s: %s", szFileName, pErrorBlob->GetBufferPointer());
		}

		if (FAILED(hr))
		{
			if (pErrorBlob != NULL) {
				MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "Compiler Error", MB_OK);
			}
			else {
				MessageBox(NULL, "Unknown error. Probably missing file\n", "Compiler Error", MB_OK);
			}
			if (pErrorBlob) pErrorBlob->Release();
			continue;
		}
		if (pErrorBlob) pErrorBlob->Release();
		break;
	}

	return true;
}
