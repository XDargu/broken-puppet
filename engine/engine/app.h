#ifndef INC_APP_H_
#define INC_APP_H_

#include "utils.h"

class CEntity;

class CApp {

	enum TGameState {
		INITIAL_VIDEO,
		MAIN_MENU,
		GAMEPLAY,
		FINAL_VIDEO
	};

	TGameState game_state;

	void update(float elapsed);
	void fixedUpdate(float elapsed); // Physx update
	void render();
	void renderEntities();
	void renderDebugEntities();
	void activateDebugMode(bool active);
	float slow_motion_counter;
public:

	std::string current_scene_name;
	std::string first_scene;
	// Timer and stats
	CDBGTimer load_timer;
	CDBGTimer aux_timer;
	double load_mesh_time;
	double load_text_time;
	double load_skel_time;
	double load_ragdoll_time;

	float time_modifier;
	float water_level;
	float delta_time;
	double total_time;

	bool pause;

	bool renderAABB;
	bool renderAxis;
	bool renderGrid;
	bool renderNames;
	bool renderNavMesh;

	int debug_map; // 0: Nothing, 1: Albedo, 2: Normals, 3: Specular, 4: Gloss, 5: Lights, 6: Depth
	bool renderWireframe;
	bool renderWireframeCurrent;

	//Variables usadas para limitar el n�mero de tramas de hilo disponibles
	static const unsigned int max_num_string = 4;

	int  xres;
	int  yres;
	bool fullscreen;
	bool has_focus;
	HWND hWnd;

	CApp();
	void loadConfig();
	bool create();
	void doFrame();
	void destroy();
	unsigned int numStrings();
	void activateVictory();
	
	void loadScene(std::string scene_name);
	void loadPrefab(std::string prefab_name);

	void activateInspectorMode(bool active);
	static CApp& get();

	void slowMotion(float time);
	bool isSlowMotion() { return slow_motion_counter > 0; }

	bool renderVideo();
	void loadVideo(const char* name);
};

#endif
