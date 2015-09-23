#include "mcv_platform.h"
#include "texture.h"
#include "DDSTextureLoader.h"

CTextureManager texture_manager;

void CTexture::activate(int slot) const {
	assert(resource_view);
	::render.ctx->PSSetShaderResources(slot, 1, &resource_view);
}

// ------------------------------------------
bool CTexture::load(const char* name) {

	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s.dds", "data/textures", name);

	// Convert the byte string to wchar string
	wchar_t wname[MAX_PATH];
	mbstowcs(wname, full_name, MAX_PATH);

	CDBGTimer tm;

	// Load the Texture
	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		render.device,
		wname,
		(ID3D11Resource**)&resource,
		&resource_view,
		0, nullptr
		);
	if (FAILED(hr))
		return false;
	
	setDbgName(resource, full_name);
	dbg("Load texture %s took %g seconds. Seconds since scene load: %g\n", name, tm.seconds(), CApp::get().load_timer.seconds());
	CApp::get().load_text_time += tm.seconds();

	return true;
}

void CTexture::destroy() {
	SAFE_RELEASE(resource);
	SAFE_RELEASE(resource_view);
}
