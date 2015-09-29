#include "mcv_platform.h"
#include "render/render_utils.h"
#include "comp_point_light.h"
#include "comp_render.h"
#include "handle/handle.h"
#include "components/comp_transform.h"
#include "aabb.h"
#include "render\render_manager.h"
#include "ai\logic_manager.h"

using namespace DirectX;

void TCompPointLight::draw() {

	int current_player_zone = CLogicManager::get().getPlayerZoneID();
	// Get the transform of the entity
	CEntity* owner = CHandle(this).getOwner();	
	TCompTransform* t = owner->get<TCompTransform>();

	XMVECTOR bounds = XMVectorSet(radius, radius, radius, 0);
	AABB aabb = AABB(t->position - bounds, t->position + bounds);
	
	bool culling = abs(current_player_zone - t->room_id) <= 1;

	culling &= render_manager.planes_active_camera.isVisible(&aabb);
	bool parent_emissive_on = true;

	if (t->hasParent()) {
		CEntity* parent = t->getParent().getOwner();
		if (parent) {
			TCompRender* render = parent->get<TCompRender>();
			if (render) {
				parent_emissive_on = render->emissive_on;
			}
		}
	}

	if (culling && parent_emissive_on) {

		XMMATRIX scale = XMMatrixScaling(radius, radius, radius);
		XMMATRIX trans = XMMatrixTranslationFromVector(t->position);
		setWorldMatrix(scale * trans);

		// Activate color, radius and intensity
		activatePointLight(this, t->position, 5);

		mesh_icosahedron.activateAndRender();
	}
}