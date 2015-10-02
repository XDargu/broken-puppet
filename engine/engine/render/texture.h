#ifndef INC_TEXTURE_MANAGER_H_
#define INC_TEXTURE_MANAGER_H_

#include "mcv_platform.h"

class CTexture {
protected:
	ID3D11ShaderResourceView* resource_view;
	ID3D11Texture2D*          resource;
	std::string               name;

	friend class CRenderToTexture;

public:
	// DirectX... pointers
	// extra info 
	// name

	CTexture() : resource_view(nullptr), resource(nullptr) {}
	virtual ~CTexture() { }
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const char* getName() const { return name.c_str(); }
	virtual void destroy();
	void activate(int slot) const;
	bool reload() { return load(name.c_str()); }

	ID3D11ShaderResourceView* getResourceView() { return resource_view; }

	void setResource(ID3D11Texture2D* the_resource) { resource = the_resource; }
	void setResourceView(ID3D11ShaderResourceView* rw) { resource_view = rw; }
	
	virtual bool isRenderTexture() const { return false; }
};

class CTextureManager : public  CItemsByName < CTexture > {
public:
	void destroyAllTextures();
};
extern CTextureManager texture_manager;

#endif
