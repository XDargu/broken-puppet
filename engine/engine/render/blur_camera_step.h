#ifndef _BLUR_CAMERA_STEP_H
#define _BLUR_CAMERA_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TBlurCameraStep {
	CRenderToTexture *rt_blur;
	char name[64];
	int  factor;
	int  xres;
	int  yres;

	float amount;

	bool create(const char* name, int axres, int ayres, int afactor);

	void apply(CTexture* in);

	CTexture* getOutput();
};

#endif