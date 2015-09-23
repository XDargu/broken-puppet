#ifndef _BLUR_STEP_H
#define _BLUR_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TBlurStep {
	CRenderToTexture *rt_blur_x;
	//CRenderToTexture *rt_blur_y;

	char name_by_x[64];
	char name_by_y[64];
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