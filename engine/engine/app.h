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
  void activateDebugMode(bool active);

public:

	float delta_time;
	double total_time;

	bool renderAABB;
	bool renderAxis;
	bool renderGrid;
	bool renderNames;

	//Variables usadas para limitar el número de tramas de hilo disponibles
	static const unsigned int max_num_string = 4;

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
	unsigned int numStrings();


	static CApp& get();
};

#endif
