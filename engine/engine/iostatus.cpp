#include "mcv_platform.h"
#include "iostatus.h"
#include "utils.h"

CIOStatus io;
CApp& App = CApp::get();

CIOStatus::CIOStatus( ) {
	buttons[CROSS].key = ' ';
	//shift
	buttons[CIRCLE].key = 16;

	buttons[DIGITAL_LEFT].key = 'A';
	buttons[DIGITAL_RIGHT].key = 'D';
	buttons[DIGITAL_UP].key = 'W';
	buttons[DIGITAL_DOWN].key = 'S';
}

void CIOStatus::TButton::setPressed(bool how, float elapsed) {

	is_pressed = how;

	// The key was and is pressed
	if (was_pressed && is_pressed)
		time_pressed += elapsed;

	// The key was and is NOT pressed anymore -> has been released
	if (was_pressed && !is_pressed)
		time_released = 0.f;

	// ... 
	if (!was_pressed && !is_pressed)
		time_released += elapsed;

	// The key was NOT pressed and now IS pressed -> has been pressed
	if (!was_pressed && is_pressed)
		time_pressed = 0.f;

}

// ---------

void CIOStatus::TMouse::update( ) {

  POINT mid_screen = { App.xres / 2, App.yres / 2 };
  ::ClientToScreen( App.hWnd, &mid_screen );

  POINT cursor_screen;
  ::GetCursorPos( &cursor_screen );

  dx = cursor_screen.x - mid_screen.x;
  dy = cursor_screen.y - mid_screen.y;

  ::SetCursorPos( mid_screen.x, mid_screen.y );
}

void CIOStatus::update(float elapsed) {

	//mouse
	mouse.update();

	// update buttons
	for (int i = 0; i<BUTTONS_COUNT; ++i) {
		TButton &b = buttons[i];

		b.was_pressed = b.is_pressed;

		bool now_is_pressed = false;

		// Is the key right now pressed?
		if (b.key){
			now_is_pressed = (::GetAsyncKeyState(b.key) & 0x8000) != 0;
		}

		if (!(i == MOUSE_LEFT || i == MOUSE_RIGHT || i == MOUSE_MIDDLE)){

			b.setPressed(now_is_pressed, elapsed);
		}
	}
}

