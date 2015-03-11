#ifndef INC_RENDER_H_
#define INC_RENDER_H_

#include <d3d11.h>
#include <d3dcompiler.h>

class CRender {
public:
  ID3D11Device*          device;
  ID3D11DeviceContext*   ctx;
  IDXGISwapChain*        swap_chain;
  ID3D11RenderTargetView* render_target_view;
  ID3D11Texture2D*        depth_stencil;
  ID3D11DepthStencilView* depth_stencil_view;

  // Methods
  CRender();
  bool createDevice();
  void destroyDevice();

  bool compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);

};

extern CRender render;

#define SAFE_RELEASE(x) if( x ) x->Release(), x = nullptr

#include "shaders.h"
#include "mesh.h"
#include "material.h"

#endif
