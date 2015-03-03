#ifndef INC_IO_STATUS_H_
#define INC_IO_STATUS_H_

class CIOStatus {

public:

	static CIOStatus& get();

	struct TButton {
		bool  is_pressed;
		bool  was_pressed;
		float time_pressed;
		float time_released;
		int   key;
		TButton() : is_pressed(false)
			, was_pressed(false)
			, time_pressed(0.f)
			, time_released(0.f)
		{ }
		void setPressed(bool how, float elapsed);
	};

	enum TButtonID {
		CROSS
		, CIRCLE
		, TRIANGLE
		, SQUARE
		, DIGITAL_LEFT
		, DIGITAL_RIGHT
		, DIGITAL_UP
		, DIGITAL_DOWN
		, START
		, MOUSE_LEFT
		, MOUSE_RIGHT
		, MOUSE_MIDDLE
		, MOUSE_WHEEL_UP
		, MOUSE_WHEEL_DOWN
		, INSPECTOR_MODE
		, DEBUG_MODE
		, THROW_STRING
		, TENSE_STRING
		, CANCEL_STRING
		, EXTRA
		, EXIT
		, BUTTONS_COUNT
	};

	bool isPressed(TButtonID button_id) const { return buttons[button_id].is_pressed; }
	bool isReleased(TButtonID button_id) const { return !buttons[button_id].is_pressed; }
	bool becomesPressed(TButtonID button_id) const {
		return buttons[button_id].is_pressed && !buttons[button_id].was_pressed;
	}
	bool becomesReleased(TButtonID button_id) const {
		return !buttons[button_id].is_pressed && buttons[button_id].was_pressed;
	}
	float getTimePressed(TButtonID button_id) const { return buttons[button_id].time_pressed; }
	float getTimeReleased(TButtonID button_id) const { return buttons[button_id].time_released; }

	TButton* getButtons(){ return buttons; }

  struct TMouse {
    int   screen_x;     
    int   screen_y;
    float normalized_x;       
    float normalized_y;
    int   dx;
    int   dy;
    int   prev_x;
    int   prev_y;

	void update(float elapsed, bool centered);
  };
  const TMouse &getMouse() const { return mouse; }

  void setMouseCoords( int x, int y );

  CIOStatus();
  void update( float elapsed );
  void setMousePointer(bool mode);
  bool getMousePointer();

protected:
  TButton   buttons[BUTTONS_COUNT];
  TMouse    mouse;

private:
	bool mouse_pointer;
};

#endif