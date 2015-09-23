#ifndef _SSAO_STEP_H
#define _SSAO_STEP_H

#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TSSAOStep {
	CRenderToTexture *rt_ssao;
	char name[64];
	int  factor;
	int  xres;
	int  yres;

	float radius;

	bool create(const char* name, int axres, int ayres, int afactor);
	void destroy();

	void apply(CTexture* in);

	TSSAOStep() :rt_ssao(nullptr) {} 

	CTexture* getOutput();
};

#endif