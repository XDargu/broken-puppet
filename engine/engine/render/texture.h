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
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const char* getName() const { return name.c_str(); }
	void destroy();
	void activate(int slot) const;
};

typedef CItemsByName< CTexture > CTextureManager;
extern CTextureManager texture_manager;

#endif
