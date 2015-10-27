#ifndef _LENS_FLARE_STEP_H
#define _LENS_FLARE_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TLensFlareStep {
	CRenderToTexture *rt_lens;
	char name[64];
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