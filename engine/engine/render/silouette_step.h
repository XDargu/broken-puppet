#ifndef _SILOUETTE_STEP_H
#define _SILOUETTE_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TSilouetteStep {
	CRenderToTexture *rt_final;
	CRenderToTexture *rt_sobel;
	CRenderToTexture *rt_dynamic;
	char name[64];
	int  factor;
	int  xres;
	int  yres;

	float amount;
	float water_level;

	bool create(const char* name, int axres, int ayres, int afactor);
	void destroy();

	void apply(CTexture* in);

	CTexture* getOutput();
};

#endif