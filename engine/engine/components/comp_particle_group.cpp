#include "mcv_platform.h"
#include "comp_particle_group.h"
#include "comp_transform.h"
#include "physics_manager.h"
#include "render\render_utils.h"
#include "particles\importer_particle_groups.h"
#include "entity_manager.h"
#include "comp_aabb.h"
#include "comp_name.h"
#include "render\render_manager.h"
#include "ai\logic_manager.h"

TCompParticleGroup::~TCompParticleGroup() {
	if (kind == flag::IMPACT)
		CLogicManager::get().p_group_counter--;
	clearParticleSystems();
	SAFE_DELETE(particle_systems);
}

void TCompParticleGroup::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
	TCompTransform* m_transform = h_transform;

	if (elem == "particleGroup") {
		strcpy(def_name, atts.getString("name", "unnamed").c_str());
	}

	if (elem == "particleSystem") {
		TParticleSystem ps = TParticleSystem();
		ps.h_transform = h_transform;
		ps.h_pg = CHandle(this);
		particle_systems->push_back(ps);		
	}
	else if (particle_systems->size() > 0) {
		particle_systems->back().loadFromAtts(elem, atts);
	}
	
}

void TCompParticleGroup::init() {	
	if (!char_equal(def_name, "undefined")) {
		CEntity* m_entity = CHandle(this).getOwner();
		particle_groups_manager.addParticleGroupToEntity(m_entity, std::string(def_name));
	}

	CHandle player = CEntityManager::get().getByName("Player");
	if (player.isValid()) {
		h_player_trans = ((CEntity*)player)->get<TCompTransform>();
	}

	for (auto& ps : *particle_systems) {
		ps.init();
	}
}

void TCompParticleGroup::fixedUpdate(float elapsed) {
	for (auto& ps : *particle_systems) {
		ps.fixedUpdate(elapsed);
	}
}

void TCompParticleGroup::update(float elapsed) {

	/*if (destroy_on_death && particle_systems.size() == 0) {
		CEntityManager::get().remove(CHandle(this).getOwner());
	}*/

	bool all_dirty = true;
	bool near_player = false;
	for (auto& ps : *particle_systems) {
		if (h_player_trans.isValid()) {
			near_player = V3DISTANCE(((TCompTransform*)h_player_trans)->position, ((TCompTransform*)h_transform)->position) < 15;
		}
		ps.visible = render_manager.planes_active_camera.isVisible(&ps.aabb) || near_player;
		ps.update(elapsed);
		all_dirty &= ps.dirty_destroy_group;
	}

	// Check if its on fire, and the particle is underwater, and destroy it
	if (char_equal(def_name, "ps_candle")) {
		TCompTransform* m_transform = h_transform;
		if (CApp::get().water_level > XMVectorGetY(m_transform->position)) {
			destroy_on_death = true;
			all_dirty = true;
		}
	}

	if (destroy_on_death && all_dirty) {
		CEntity* e = CHandle(this).getOwner();
		CEntityManager::get().remove(e);
		clearParticleSystems();
		TCompName* name = e->get<TCompName>();
		//XDEBUG("Destroying name: %s", name);
	}
}

void TCompParticleGroup::render() {
	/*TCompAABB* aabb = getSibling<TCompAABB>(this);
	XMVECTOR minValue = XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, 1);
	XMVECTOR maxValue = -minValue;*/
	CEntity* e = CHandle(this).getOwner();
	TCompName* name = e->get<TCompName>();
	//XDEBUG("Rendering name: %s", name);
	for (auto& ps : *particle_systems) {

		/*if (XMVectorGetX(ps.aabb.min) < XMVectorGetX(minValue))
			minValue = XMVectorSetX(minValue, XMVectorGetX(ps.aabb.min));
		if (XMVectorGetY(ps.aabb.min) < XMVectorGetY(minValue))
			minValue = XMVectorSetY(minValue, XMVectorGetY(ps.aabb.min));
		if (XMVectorGetZ(ps.aabb.min) < XMVectorGetZ(minValue))
			minValue = XMVectorSetZ(minValue, XMVectorGetZ(ps.aabb.min));

		if (XMVectorGetX(ps.aabb.max) > XMVectorGetX(maxValue))
			maxValue = XMVectorSetX(maxValue, XMVectorGetX(ps.aabb.max));
		if (XMVectorGetY(ps.aabb.max) > XMVectorGetY(maxValue))
			maxValue = XMVectorSetY(maxValue, XMVectorGetY(ps.aabb.max));
		if (XMVectorGetZ(ps.aabb.max) > XMVectorGetZ(maxValue))
			maxValue = XMVectorSetZ(maxValue, XMVectorGetZ(ps.aabb.max));*/
		
		ps.render(false);		
	}

	/*aabb->min = minValue;
	aabb->max = maxValue;*/
}


void TCompParticleGroup::renderDistorsion() {
	for (auto& ps : *particle_systems) {
		ps.render(true);
	}
}

void TCompParticleGroup::renderDebug3D() const {
	for (auto& ps : *particle_systems) {
		ps.renderDebug3D();
	}
}

void TCompParticleGroup::removeParticleSystem(TParticleSystem* ps) {
	particle_systems->erase(std::remove(particle_systems->begin(), particle_systems->end(), *ps), particle_systems->end());
	/*for (auto& ps_2 : *particle_systems) {
		if (ps_2.emitter_generation == ps->emitter_generation) {
			ps_2.dirty_destroy_group = true;
		}
	}*/
}

void TCompParticleGroup::clearParticleSystems() {
	for (int i = 0; i < particle_systems->size(); ++i) {
		(*particle_systems)[i].destroy();
	}
	particle_systems->clear();
}

void TCompParticleGroup::restart() {
	for (auto& ps : *particle_systems) {
		ps.restart();
	}
}

std::string TCompParticleGroup::getXMLDefinition() {
	std::string def = "";

	def += "<particleGroup>";

	for (auto& ps : *particle_systems) {
		def += ps.getXMLDefinition();
	}

	def += "</particleGroup>";

	return def;
}

std::string TCompParticleGroup::getXMLDefinitionWithName(std::string name) {
	std::string def = "";

	def += "<particleGroup name=\"";
	def += name;
	def += "\">";

	for (auto& ps : *particle_systems) {
		def += ps.getXMLDefinition();
	}

	def += "</particleGroup>";

	return def;
}