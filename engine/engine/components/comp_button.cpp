#include "mcv_platform.h"
#include "comp_button.h"
#include "comp_render.h"
#include "io/iostatus.h"
#include "physics_manager.h"
#include "comp_camera.h"
#include "render\render_manager.h"
#include "ai\logic_manager.h"
#include "entity_manager.h"

void TCompButton::loadFromAtts(const std::string& elem, MKeyValue& atts) {
	if (elem == "actionChangeScene") {
		strcpy(scene_change_name, atts.getString("name", "noscene").c_str());
	}
	if (elem == "actionExit") {
		exit_action = true;
	}
	if (elem == "actionStartGame") {
		start_game_action = true;
	}
	hovering = false;
}

void TCompButton::init() {
	CIOStatus& io = CIOStatus::get();

	io.setMousePointer(false);
	while (ShowCursor(TRUE) <= 0);
}

void TCompButton::update(float elapsed) {
	CIOStatus& io = CIOStatus::get();

	TCompCamera* camera = render_manager.activeCamera;
	XMVECTOR mouse_pos = camera->getWorldCoords(io.getMouse().normalized_x, io.getMouse().normalized_y);

	/*CEntity* l_e = CEntityManager::get().getByName("libro_b_35.0");
	TCompTransform* t = l_e->get<TCompTransform>();
	t->position = mouse_pos;*/
	
		
	PxRaycastBuffer hit;		

	XMVECTOR dir = XMVector3Normalize(mouse_pos - camera->getPosition());
	Physics.raycast(mouse_pos, dir, 1000, hit);

	//t->position = mouse_pos + dir * 1;
	CHandle m_entity = CHandle(this).getOwner();
	TCompTransform* t = ((CEntity*)m_entity)->get<TCompTransform>();
	TCompRender* r = ((CEntity*)m_entity)->get<TCompRender>();

	bool now_hovering = false;

	if (hit.hasBlock) {
		CHandle target_entity(hit.block.actor->userData);
		
		if (target_entity.isValid()) {

			if (m_entity == target_entity) {
				
				now_hovering = true;

				if (CIOStatus::get().becomesReleased(CIOStatus::MOUSE_LEFT)){
					CSoundManager::get().playEvent("BUTTON_CLICK");
					// Actions
					// Change scene
					if (!char_equal(scene_change_name, "")) {
						CLogicManager::get().loadScene(scene_change_name);
						CApp::get().game_state = CApp::TGameState::GAMEPLAY;
					}

					// Exit
					if (exit_action) {
						CLogicManager::get().exitGame();
					}

					// Start game
					if (start_game_action) {
#ifdef _DEBUG
						CLogicManager::get().loadScene("data/scenes/scene_1.xml");
#else
						CApp::get().playInitialVideo();
#endif
					}
				}

			}
		}
	}

	if (hovering != now_hovering) {
		r->swap_textures = now_hovering;
		if (now_hovering) {
			CSoundManager::get().playEvent("BUTTON_HOVER");
		}
	}

	hovering = now_hovering;
}
