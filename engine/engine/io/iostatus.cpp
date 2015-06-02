#include "mcv_platform.h"
#include "iostatus.h"
#include "../utils.h"

CApp& App = CApp::get();

static CIOStatus io;

CIOStatus& CIOStatus::get() {
	return io;
}

CIOStatus::CIOStatus( ) {
	buttons[CROSS].key = ' ';
	//shift
	buttons[CIRCLE].key = 16;

	buttons[MOUSE_LEFT].key = VK_LBUTTON;
	buttons[MOUSE_RIGHT].key = VK_RBUTTON;
	buttons[MOUSE_MIDDLE].key = VK_MBUTTON;	

	buttons[DIGITAL_LEFT].key = 'A';
	buttons[DIGITAL_RIGHT].key = 'D';
	buttons[DIGITAL_UP].key = 'W';
	buttons[DIGITAL_DOWN].key = 'S';

	buttons[THROW_STRING].key = VK_LBUTTON;
	buttons[TENSE_STRING].key = VK_RBUTTON;
	buttons[PULL_STRING].key = VK_RBUTTON;
	buttons[CANCEL_STRING].key = VK_MBUTTON;
	buttons[EXTRA].key = 'K';	
	buttons[INSPECTOR_MODE].key = VK_F2;
	buttons[DEBUG_MODE].key = VK_F3;
	buttons[EXIT].key = VK_ESCAPE;
	buttons[RUN].key = VK_LSHIFT;

	buttons[VIEWER_MOVE_CAM].key = VK_RBUTTON;
	buttons[VIEWER_PAN].key = VK_MBUTTON;

	buttons[F8_KEY].key = VK_F8;
	buttons[F4_KEY].key = VK_F4;
	buttons[ALT].key = VK_MENU;

	buttons[NUM0].key = '0';
	buttons[NUM1].key = '1';
	buttons[NUM2].key = '2';
	buttons[NUM3].key = '3';
	buttons[NUM4].key = '4';
	buttons[NUM5].key = '5';
	buttons[NUM6].key = '6';

	mouse.dx = 0;
	mouse.dy = 0;

	POINT cursor_screen;
	::GetCursorPos(&cursor_screen);
	::ScreenToClient(App.hWnd, &cursor_screen);

	mouse.prev_x = cursor_screen.x;
	mouse.prev_y = cursor_screen.y;

	mouse.screen_x = cursor_screen.x;
	mouse.screen_y = cursor_screen.y;

	mouse.normalized_x = (float)cursor_screen.x / App.xres;
	mouse.normalized_y = (float)cursor_screen.y / App.yres;
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
void CIOStatus::TMouse::update(float elapsed, bool centered){
	// If centered mouse	
	POINT mid_screen = { App.xres / 2, App.yres / 2 };
	::ClientToScreen(App.hWnd, &mid_screen);

	POINT cursor_screen;
	::GetCursorPos(&cursor_screen);
	::ScreenToClient(App.hWnd, &cursor_screen);
	

	dx = cursor_screen.x - prev_x;
	dy = cursor_screen.y - prev_y;

	screen_x = cursor_screen.x;
	screen_y = cursor_screen.y;

	normalized_x = (float)cursor_screen.x / App.xres;
	normalized_y = (float)cursor_screen.y / App.yres;

	prev_x = screen_x;
	prev_y = screen_y;

	if (centered) {
		::SetCursorPos(mid_screen.x, mid_screen.y);
		::ScreenToClient(App.hWnd, &mid_screen);
		prev_x = mid_screen.x;
		prev_y = mid_screen.y;
	}
}

bool CIOStatus::getMousePointer() {
	return mouse_pointer;
}

void CIOStatus::setMousePointer(bool mode){
	mouse_pointer = mode;
	
	// Set delta values if centered (add offset)
	if (mode) {
		POINT mid_screen = { App.xres / 2, App.yres / 2 };
		::ClientToScreen(App.hWnd, &mid_screen);		

		POINT cursor_screen;
		::GetCursorPos(&cursor_screen);
		::ScreenToClient(App.hWnd, &cursor_screen);

		float del_x = (float)cursor_screen.x - mouse.prev_x;
		float del_y = (float)cursor_screen.y - mouse.prev_y;

		::SetCursorPos(mid_screen.x, mid_screen.y);

		mouse.prev_x = (App.xres / 2) + (int)del_x;
		mouse.prev_y = (App.yres / 2) + (int)del_y;
	}
}

void CIOStatus::setMouseCoords(int x, int y){
	::SetCursorPos(x, y);
}

void CIOStatus::update(float elapsed) {

	if (CApp::get().has_focus){
		//mouse	
		mouse.update(elapsed, mouse_pointer);

		// update buttons
		for (int i = 0; i < BUTTONS_COUNT; ++i) {
			TButton &b = buttons[i];

			b.was_pressed = b.is_pressed;

			bool now_is_pressed = false;

			// Is the key right now pressed?
			if (b.key){
				now_is_pressed = (::GetAsyncKeyState(b.key) & 0x8000) != 0;
			}

			b.setPressed(now_is_pressed, elapsed);
		}
	}
}

