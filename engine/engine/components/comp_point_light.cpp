#include "mcv_platform.h"
#include "render/render_utils.h"
#include "comp_point_light.h"
#include "handle/handle.h"
#include "components/comp_transform.h"
#include "aabb.h"
#include "render\render_manager.h"

using namespace DirectX;

void TCompPointLight::draw() {


	// Get the transform of the entity
	CEntity* owner = CHandle(this).getOwner();	
	TCompTransform* t = owner->get<TCompTransform>();

	XMVECTOR bounds = XMVectorSet(radius, radius, radius, 0);
	AABB aabb = AABB(t->position - bounds, t->position + bounds);

	bool culling = render_manager.planes_active_camera.isVisible(&aabb);

	if (culling) {

		XMMATRIX scale = XMMatrixScaling(radius, radius, radius);
		XMMATRIX trans = XMMatrixTranslationFromVector(t->position);
		setWorldMatrix(scale * trans);

		// Activate color, radius and intensity
		activatePointLight(this, t->position, 5);

		mesh_icosahedron.activateAndRender();
	}
}