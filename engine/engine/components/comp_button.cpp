#include "mcv_platform.h"
#include "comp_button.h"
#include "io/iostatus.h"
#include "physics_manager.h"
#include "comp_camera.h"
#include "render\render_manager.h"
#include "ai\logic_manager.h"
#include "entity_manager.h"

void TCompButton::loadFromAtts(const std::string& elem, MKeyValue& atts) {
	if (elem == "actionChangeScene") {
		scene_change_name = atts.getString("name", "noscene");
	}
	if (elem == "actionExit") {
		exit_action = true;
	}
}

void TCompButton::init() {
	CIOStatus& io = CIOStatus::get();

	io.setMousePointer(false);
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


	if (hit.hasBlock) {
		CHandle target_entity(hit.block.actor->userData);
		t->scale = XMVectorSet(1, 1, 1, 0);

		if (target_entity.isValid()) {

			if (m_entity == target_entity) {
				
				t->scale = XMVectorSet(1.1f, 1.1f, 1.1f, 0);

				if (CIOStatus::get().becomesReleased(CIOStatus::MOUSE_LEFT)){
					// Actions
					// Change scene
					if (scene_change_name != "") {
						CLogicManager::get().loadScene(scene_change_name);
					}

					// Exit
					if (exit_action) {
						exit(0);
					}
				}

			}
		}
	}
}
