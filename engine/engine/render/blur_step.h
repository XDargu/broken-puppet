#include "render.h"
#include "render_to_texture.h"

// --------------------------------
struct TBlurStep {
	CRenderToTexture *rt_down_by_x;
	CRenderToTexture *rt_down_by_y;
	char name_by_x[64];
	char name_by_y[64];
	int  factor;
	int  xres;
	int  yres;

	bool create(const char* name, int axres, int ayres, int afactor);

	void apply(CTexture* in);

	CTexture* getOutput();
};