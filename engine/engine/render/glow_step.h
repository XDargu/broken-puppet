#ifndef _GLOW_STEP_H
#define _GLOW_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TGlowStep {
	CRenderToTexture *rt_lights;
	CRenderToTexture *rt_glow;
	char name_lights[64];
	char name_glow[64];
	int  factor;
	int  xres;
	int  yres;

	float amount;

	bool create(const char* name, int axres, int ayres, int afactor);
	void destroy();

	void apply(CTexture* in);

	CTexture* getOutput();
};

#endif