#ifndef _SSRR_STEP_H
#define _SSRR_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TSSRRStep {
	CRenderToTexture *rt_ssrr;
	char name[64];
	int  factor;
	int  xres;
	int  yres;

	float amount;
	float quality;

	bool create(const char* name, int axres, int ayres, int afactor);
	void destroy();

	void apply(CTexture* in);

	CTexture* getOutput();
};

#endif