#include "mcv_platform.h"
#include "font.h"
#include "../handle/handle.h"
#include "../components/comp_camera.h"

#pragma comment(lib, "font/FW1FontWrapper.lib") 

CFont::CFont()
  : font(nullptr)
  , FW1Factory(nullptr)
  , color(0xffffffff)   // Text color default to white: 0xAaBbGgRr
  , size(16.f)
  , camera(nullptr)
{}

bool CFont::create() {
  assert(render.device);
  assert(font == nullptr);
  HRESULT hResult = FW1CreateFactory(FW1_VERSION, &FW1Factory);
  hResult = FW1Factory->CreateFontWrapper(render.device, L"Lucida Console", &font);
  return !FAILED(hResult);
}

void CFont::destroy() {
  if (font)
    font->Release();
  font = nullptr;
  if (FW1Factory)
    FW1Factory->Release();
  FW1Factory = nullptr;
}

float CFont::print(float x, float y, const char *text) const {
  if (!font)
    return 0.f;
  assert(font);
  WCHAR utf16[512];
  memset(utf16, 0x80, 512 * 2);
  size_t n = mbstowcs(utf16, text, strlen(text));
  utf16[n] = 0x00;
  font->DrawString(
    render.ctx,
    utf16,
    size,
    x, y,
    color,
    FW1_RESTORESTATE// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
    );
  return size;
}

float CFont::printf(float x, float y, const char *fmt, ...) const {
  va_list args;
  va_start(args, fmt);

  char buf[ 2048 ];
  int n = vsnprintf( buf, sizeof( buf )-1, fmt, args );

  // Confirm the msg fits in the given buffer
  if( n < 0 )
    buf[ sizeof( buf )-1 ] = 0x00;

  return print( x, y, buf );
}

float CFont::print3D(XMVECTOR world_p3d, const char *text) const {
  assert( camera );
  float x,y;
  if( camera->getScreenCoords( world_p3d, &x, &y ) )
    return print( x, y, text );
  return 0.f;
}
