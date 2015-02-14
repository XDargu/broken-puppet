#include "mcv_platform.h"
#include "iostatus.h"
#include "utils.h"

CIOStatus io;
CApp& App = CApp::get();

CIOStatus::CIOStatus( ) {
}

// ---------
void CIOStatus::update( float elapsed ) {
  mouse.update();
}

void CIOStatus::TMouse::update( ) {

  POINT mid_screen = { App.xres / 2, App.yres / 2 };
  ::ClientToScreen( App.hWnd, &mid_screen );

  POINT cursor_screen;
  ::GetCursorPos( &cursor_screen );

  dx = cursor_screen.x - mid_screen.x;
  dy = cursor_screen.y - mid_screen.y;

  ::SetCursorPos( mid_screen.x, mid_screen.y );
}

