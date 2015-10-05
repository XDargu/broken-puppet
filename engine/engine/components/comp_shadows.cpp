#include "mcv_platform.h"
#include "render/render_utils.h"
#include "handle/handle.h"
#include "comp_shadows.h"
#include "ai/logic_manager.h"

using namespace DirectX;

void TCompShadows::init() {

	CEntity* e = CHandle(this).getOwner();
	assert(e);

	// Create a give ownership to texture mananger
	rt = new CRenderToTexture;
	bool is_ok = rt->create(e->getName(), resolution, resolution
		, DXGI_FORMAT_UNKNOWN     // I do NOT want COLOR Buffer
		, DXGI_FORMAT_R32_TYPELESS
		);
	assert(is_ok);
}

void TCompShadows::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	resolution = atts.getInt("res", 256);
	color = atts.getPoint("color");
	intensity = atts.getFloat("intensity", 1.f);
	character_camera = atts.getBool("characterLight", false);

	// TODO: Para que funcione desde max, es necesario rotarlas 90 grados, cambiar el exportador de max más adelante
	if (atts.has("correction")) {
		TCompTransform* m_trans = getSibling<TCompTransform>(this);
		m_trans->lookAt(m_trans->position - m_trans->getUp(), m_trans->getUp());
	}

}

// This generates the depth map from the ligth source
void TCompShadows::generate()  {
	// NO shadows
	if (resolution == 1) { 
		rt->clearDepthBuffer(); 
		return; 
	}

	CTraceScoped s("shadows");

	// Activate the camera component which is the source of the light
	CEntity* e = CHandle(this).getOwner();
	TCompCamera* camera = e->get<TCompCamera>();
	assert(camera || fatal("TCompShadows requieres a TCompCamera component"));

	activateRSConfig(RSCFG_SHADOWS);

	// Start rendering in the rt of the depth buffer
	setSlotNull(6);
	rt->activate();
	rt->clearDepthBuffer();

	activateCamera(*camera, 1);

	// 
	
	render_techniques_manager.getByName("gen_shadows")->activate();
	render_manager.renderShadowsCasters(camera, character_camera);	

	activateRSConfig(RSCFG_DEFAULT);

}

// This renders the light volume into the light accumulation buffer
void TCompShadows::draw() {
	// Culling
	int current_player_zone = CLogicManager::get().getPlayerZoneID();

	// Use the inv view projection as world matrix, so our unit cube mesh
	// becomes the fustrum in world space
	CEntity* owner = CHandle(this).getOwner();
	TCompCamera* c = owner->get<TCompCamera>();
	TCompTransform* t = owner->get<TCompTransform>();

	bool culling = true;// planes_active_camera.isVisible(m_aabb);

	// Draw the light only if it is in the same zone as the player
	culling &= abs(current_player_zone - t->room_id) <= 1;

	if (culling) {

		XMVECTOR det;
		XMMATRIX inv_view_proj = XMMatrixInverse(&det, c->getViewProjection());
		setWorldMatrix(inv_view_proj);

		// Activate the previously generated shadow map
		rt->getZTexture()->activate(6);

		// Activate color, radius and intensity
		activateLight(*c, 4);
		activateDirLight(this, c->getPosition(), t->getFront(), c->getFov(), 5);

		mesh_view_volume.activateAndRender();
	}
}
