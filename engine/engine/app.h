#ifndef INC_APP_H_
#define INC_APP_H_

#include "entity_inspector.h"

class CEntity;

class CApp {

  void update(float elapsed);
  void fixedUpdate(float elapsed); // Physx update
  void render();
  void renderEntities();
  void renderDebugEntities();
  void activateInspectorMode(bool active);

public:

	bool renderAABB;
	bool renderAxis;
	bool renderGrid;
	bool renderNames;

	int  xres;
	int  yres;
	HWND hWnd;
	CEntityInspector entity_inspector;
	CEntityLister	 entity_lister;
	CEntityActioner	 entity_actioner;
	CDebugOptioner	 debug_optioner;

	CApp();
	void loadConfig();
	bool create();
	void doFrame();
	void destroy();

	static CApp& get();
};

#endif
