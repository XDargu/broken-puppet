#ifndef INC_RENDER_TO_TEXTURE_H_
#define INC_RENDER_TO_TEXTURE_H_

#include "texture.h"

class CRenderToTexture : public CTexture {
	const char* name;
	int         xres, yres;
	DXGI_FORMAT color_fmt;
	DXGI_FORMAT depth_fmt;

	ID3D11RenderTargetView*     render_target_view;

	ID3D11DepthStencilView*     depth_stencil_view;
	CTexture*                   ztexture;

	friend class CDeferredRender;

	bool createColorBuffer();
	bool createDepthBuffer();

public:

	enum TZBufferType { USE_BACK_ZBUFFER, USE_OWN_ZBUFFER, NO_ZBUFFER };

	CRenderToTexture();
	bool create(const char* name, int xres, int yres, DXGI_FORMAT color_fmt, DXGI_FORMAT depth_fmt
		, TZBufferType zbuffer_type = USE_OWN_ZBUFFER);
	void activate();
	void activateViewport();
	void clearColorBuffer(const FLOAT ColorRGBA[4]);
	void clearDepthBuffer();

	CTexture* getZTexture() { return ztexture; }

};

#endif
