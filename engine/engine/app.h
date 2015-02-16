#ifndef INC_APP_H_
#define INC_APP_H_

class CEntity;

class CApp {

  void update(float elapsed);
  void fixedUpdate(float elapsed); // Physx update
  void render();
  void renderEntities();
  void renderDebugEntities(bool draw_names);
  void CApp::renderEntityDebugList();

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
