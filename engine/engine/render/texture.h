#ifndef INC_TEXTURE_MANAGER_H_
#define INC_TEXTURE_MANAGER_H_

#include "mcv_platform.h"

class CTexture {
  ID3D11ShaderResourceView* resource_view;
  ID3D11Resource*           resource;
  std::string               name;
public:
  // DirectX... pointers
  // extra info 
  // name

  CTexture() : resource_view(nullptr), resource(nullptr) {}
  bool load(const char* name);
  void setName(const char *new_name) {
    name = new_name;
  }
  void destroy();
  void activate(int slot) const;
  bool isValid() const { return resource != nullptr; }
};

typedef CItemsByName< CTexture > CTextureManager;
extern CTextureManager texture_manager;

#endif
