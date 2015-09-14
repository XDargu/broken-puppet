#ifndef INC_FONT_H_
#define INC_FONT_H_

#include "FW1FontWrapper.h"
#include "handle/handle.h"

class CCamera;

struct CFont {
  IFW1FontWrapper* font;
  IFW1Factory*     FW1Factory;
  unsigned         color;
  float            size;
  CHandle		   camera;
  CFont();
  bool create();
  bool create(LPCWSTR name);
  void destroy();
  XMVECTOR print(float x, float y, const char *text) const;
  XMVECTOR printCentered(float x, float y, const char *text) const;
  float printf(float x, float y, const char *fmt, ... ) const;
  XMVECTOR print3D(XMVECTOR p3d, const char *fmt) const;

  XMVECTOR measureString(const char *text) const;
};

extern CFont font;

#endif
