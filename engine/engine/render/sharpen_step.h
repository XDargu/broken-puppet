#ifndef _SHARPEN_STEP_H
#define _SHARPEN_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TSharpenStep {
	CRenderToTexture *rt_sharpen;
	char name[64];
	int  factor;
	int  xres;
	int  yres;

	float amount;
	float water_level;

	bool create(const char* name, int axres, int ayres, int afactor);

	void apply(CTexture* in);

	CTexture* getOutput();
};

#endif