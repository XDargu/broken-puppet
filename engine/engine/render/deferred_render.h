#ifndef INC_DEFERRED_RENDER_H_
#define INC_DEFERRED_RENDER_H_

#include "render_to_texture.h"

class CCamera;

class CDeferredRender {

	
	const CRenderTechnique* technique_deferred_point_lights;
	const CRenderTechnique* technique_deferred_dir_lights;


	void generateGBuffer(const CCamera* camera);
	void generateLightBuffer();
	void resolve(const CCamera* camera, CRenderToTexture& rt_out);

public:

	CRenderToTexture*  rt_lights;
	CRenderToTexture*  rt_albedo;
	CRenderToTexture*  rt_normals;
	CRenderToTexture*  rt_specular;
	CRenderToTexture*  rt_gloss;
	CRenderToTexture*  rt_depth;

	bool create(int xres, int yres);
	void destroy();

	void render(const CCamera* camera, CRenderToTexture& rt_out);

	// CTexture* getOutput();
};

#endif
