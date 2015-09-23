#ifndef _UNDERWATER_STEP_H
#define _UNDERWATER_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TUnderwaterEffect {
	CRenderToTexture *rt_underwater;
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