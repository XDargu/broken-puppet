#ifndef INC_APP_H_
#define INC_APP_H_

class CApp {

  void update(float elapsed);
  void render();
  void renderEntities();

public:
	int  xres;
	int  yres;
  HWND hWnd;

	CApp();
	void loadConfig();
	bool create();
	void doFrame();
	void destroy();

	static CApp& get();
};

#endif
