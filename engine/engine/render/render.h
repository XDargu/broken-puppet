#ifndef INC_RENDER_H_
#define INC_RENDER_H_

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d9.h>         // D3DPERF_*

class CRender {
public:
	ID3D11Device*           device;
	ID3D11DeviceContext*    ctx;
	IDXGISwapChain*         swap_chain;
	ID3D11RenderTargetView* render_target_view;
	ID3D11Texture2D*        depth_stencil;
	ID3D11DepthStencilView* depth_stencil_view;
	int                     xres, yres;

	// Methods
	CRender();
	bool createDevice();
	void destroyDevice();

	bool compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);

	void activateBackbuffer();

};

extern CRender render;

#define SAFE_RELEASE(x) if( x ) x->Release(), x = nullptr

#define setDbgName(obj,name) (obj)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT) strlen( name ), name );

struct CDbgTrace {
	wchar_t wname[64];
	CDbgTrace(const char *name) {
		setName(name);
	}
	inline void setName(const char *name) {
		mbstowcs(wname, name, 64);
	}
	inline void begin() {
		D3DPERF_BeginEvent(D3DCOLOR_XRGB(255, 0, 255), wname);
	}
	inline void end() {
		D3DPERF_EndEvent();
	}
};

struct CTraceScoped : public CDbgTrace {
	CTraceScoped(const char *name) : CDbgTrace(name) {
		//dbg("Dibujando: %s", name);
		begin();
	}
	~CTraceScoped() {
		end();
	}
};



#include "shaders.h"
#include "mesh.h"
#include "material.h"

#endif
