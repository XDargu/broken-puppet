#ifndef _CHROMATIC_ABERRATION_STEP_H
#define _CHROMATIC_ABERRATION_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TChromaticAberrationStep {
	CRenderToTexture *rt_ca;
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