#ifndef INC_SHADERS_H_
#define INC_SHADERS_H_

#include "render.h"
#include "vertex_declarations.h"
#include "XMLParser.h"
#include "items_by_name.h"

// ----------------------------------------
class CShaderCteBase {
protected:
	ID3D11Buffer*  buffer;
	bool           dirty;

public:
	CShaderCteBase() : buffer(nullptr), dirty(false) {}
	void destroy() {
		SAFE_RELEASE(buffer);
	}

	void activateInVS(unsigned index_slot) const {
		assert(buffer);
		assert(!dirty);
		render.ctx->VSSetConstantBuffers(index_slot, 1, &buffer);
	}
	void activateInPS(unsigned index_slot) const {
		assert(buffer);
		assert(!dirty);
		render.ctx->PSSetConstantBuffers(index_slot, 1, &buffer);
	}
};

template< class TParams >
class CShaderCte : public CShaderCteBase {
	TParams params;
public:
	bool create() {
		assert(buffer == nullptr);
		// Create the constant buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(TParams);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr = ::render.device->CreateBuffer(&bd, NULL, &buffer);
		if (FAILED(hr))
			return false;
		return true;
	}

	TParams* get() {
		dirty = true;
		return &params;
	}

	//void updateFrom(const TParams& new_data) {
	//  assert(buffer);
	//  render.ctx->UpdateSubresource(buffer, 0, NULL, &new_data, 0, 0);
	//}
	void uploadToGPU() {
		assert(buffer);
		dirty = false;
		const TParams *obj = &params;
		render.ctx->UpdateSubresource(buffer, 0, NULL, obj, 0, 0);
	}

};

// ----------------------------------------
class CVertexShader {
	ID3D11VertexShader*   vs;
	ID3D11InputLayout*    vtx_layout;
public:
	CVertexShader();
	bool compile(const char* szFileName
		, const char* szEntryPoint
		, const CVertexDecl &decl);
	void destroy();
	void activate() const;
	void setCte(unsigned index_slot, const CShaderCteBase& cte);
};

// ----------------------------------------
class CPixelShader {
	ID3D11PixelShader*   ps;
public:
	CPixelShader();
	bool compile(const char* szFileName, const char* szEntryPoint);
	void destroy();
	void activate() const;
};

// ----------------------------------------
class CRenderTechnique : public CXMLParser {
	CVertexShader    vs;
	CPixelShader     ps;
	std::string      name;
	void onStartElement(const std::string &elem, MKeyValue &atts);
public:
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const std::string& getName() const { return name; }
	void activate() const;
	void destroy();
};

typedef CItemsByName< CRenderTechnique > CRenderTechniquesManager;
extern CRenderTechniquesManager render_techniques_manager;


#endif
