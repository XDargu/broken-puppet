#include "mcv_platform.h"
#include "shaders.h"

CRenderTechniquesManager render_techniques_manager;

// ----------------------------------------------------
CVertexShader::CVertexShader() : vs(nullptr), vtx_layout(nullptr) {}

bool CVertexShader::compile(
	const char* szFileName
	, const char* szEntryPoint
	, const CVertexDecl &decl) {

	// A blob is a representation of a buffer in dx, a void pointer + size
	ID3DBlob* pVSBlob = NULL;
	bool is_ok = render.compileShaderFromFile(szFileName, szEntryPoint
		, "vs_4_0", &pVSBlob);
	if (!is_ok)
		return false;

	// Create the vertex shader
	HRESULT hr;
	hr = render.device->CreateVertexShader(
		pVSBlob->GetBufferPointer()
		, pVSBlob->GetBufferSize()
		, NULL
		, &vs);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}

	// Create the input layout
	hr = render.device->CreateInputLayout(decl.elems, decl.nelems, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &vtx_layout);
	pVSBlob->Release();
	if (FAILED(hr))
		return false;

	return true;
}

void CVertexShader::destroy() {
	SAFE_RELEASE(vs);
	SAFE_RELEASE(vtx_layout);
}

void CVertexShader::activate() const {
	assert(vs);
	render.ctx->VSSetShader(vs, NULL, 0);

	// Set the input layout
	render.ctx->IASetInputLayout(vtx_layout);
}


// ----------------------------------------------------
CPixelShader::CPixelShader() : ps(nullptr) {}

bool CPixelShader::compile(const char* szFileName, const char* szEntryPoint) {

	ID3DBlob* pVSBlob = NULL;
	bool is_ok = render.compileShaderFromFile(szFileName, szEntryPoint, "ps_4_0", &pVSBlob);
	if (!is_ok)
		return false;

	// Create the vertex shader
	HRESULT hr;
	hr = render.device->CreatePixelShader(
		pVSBlob->GetBufferPointer()
		, pVSBlob->GetBufferSize()
		, NULL
		, &ps);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}

	pVSBlob->Release();
	return true;
}

void CPixelShader::destroy() {
	SAFE_RELEASE(ps);
}


void CPixelShader::activate() const {
	// ps might be null, for example when generating shadows
	render.ctx->PSSetShader(ps, NULL, 0);
}

// --------------------------------------
bool CRenderTechnique::load(const char* name) {
	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s.xml", "data/techniques", name);
	return xmlParseFile(full_name);
}

void CRenderTechnique::activate() const {
	vs.activate();
	ps.activate();
}

void CRenderTechnique::destroy() {
	vs.destroy();
	ps.destroy();
}


// --------------------------------------
void CRenderTechnique::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "technique") {
		std::string vs_name = atts["vs"];
		std::string vs_decl_name = atts["vs_decl"];

		// Pasar la decl de texto a objeto de c++
		const CVertexDecl* decl = nullptr;
		if (vs_decl_name == "vdcl_position_color")
			decl = &vdcl_position_color;
		else if (vs_decl_name == "vdcl_position_uv")
			decl = &vdcl_position_uv;
		else if (vs_decl_name == "vdcl_position_uv_normal")
			decl = &vdcl_position_uv_normal;
		else if (vs_decl_name == "vdcl_position_uv_normal_skin")
			decl = &vdcl_position_uv_normal_skin;
		else if (vs_decl_name == "vdcl_position_uv_normal_tangent")
			decl = &vdcl_position_uv_normal_tangent;
		else if (vs_decl_name == "vdcl_position_uv_normal_skin_tangent")
			decl = &vdcl_position_uv_normal_skin_tangent;
		else if (vs_decl_name == "vdcl_instanced_position_uv")
			decl = &vdcl_instanced_position_uv;
		else
			fatal("Unsupported vdcl %s", vs_decl_name.c_str());
		assert(decl != nullptr);

		// 
		std::string ps_name = atts["ps"];
		std::string fx_name = atts["fx"];
		if (fx_name.empty())
			fx_name = "deferred_gbuffer.fx";

		bool is_ok = vs.compile(fx_name.c_str(), vs_name.c_str(), *decl);

		if (!ps_name.empty())
			is_ok &= ps.compile(fx_name.c_str(), ps_name.c_str());

		uses_bones = atts.getBool("uses_bones", false);

		assert(is_ok);
	}
}

