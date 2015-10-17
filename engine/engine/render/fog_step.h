#ifndef _FOG_STEP_H
#define _FOG_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TFogStep {
	CRenderToTexture *rt_fog;
	char name[64];
	int  factor;
	int  xres;
	int  yres;

	XMVECTOR fog_color;
	float fog_level;
	float fog_distance;

	float amount;

	bool create(const char* name, int axres, int ayres, int afactor);
	void destroy();

	void apply(CTexture* in);

	CTexture* getOutput();
};

#endif