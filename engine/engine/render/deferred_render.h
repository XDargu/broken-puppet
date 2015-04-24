#ifndef INC_DEFERRED_RENDER_H_
#define INC_DEFERRED_RENDER_H_

#include "render_to_texture.h"

class CCamera;

class CDeferredRender {

	CRenderToTexture*  rt_lights;
	CRenderToTexture*  rt_albedo;
	CRenderToTexture*  rt_normals;

	void generateGBuffer(const CCamera* camera);
	void generateLightBuffer();

public:

	bool create(int xres, int yres);
	void render(const CCamera* camera);

	// CTexture* getOutput();
};

#endif
