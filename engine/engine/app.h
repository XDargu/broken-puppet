#ifndef INC_APP_H_
#define INC_APP_H_

#include "entity_inspector.h"

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
	CEntityInspector entity_inspector;
	CEntityLister	 entity_lister;
	CEntityActioner	 entity_actioner;

	CApp();
	void loadConfig();
	bool create();
	void doFrame();
	void destroy();

	static CApp& get();
};

#endif
