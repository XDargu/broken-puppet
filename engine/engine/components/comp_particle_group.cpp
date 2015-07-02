#include "mcv_platform.h"
#include "comp_particle_group.h"
#include "comp_transform.h"
#include "physics_manager.h"
#include "render\render_utils.h"
#include "particles\importer_particle_groups.h"
#include "entity_manager.h"
#include "comp_aabb.h"
#include "render\render_manager.h"

TCompParticleGroup::~TCompParticleGroup() {
	for (int i = 0; i < particle_systems.size(); ++i) {
		particle_systems[i].destroy();
	}
}

void TCompParticleGroup::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	CHandle h_transform = assertRequiredComponent<TCompTransform>(this);
	TCompTransform* m_transform = h_transform;

	if (elem == "particleGroup") {
		def_name = atts.getString("name", "unnamed");
	}

	if (elem == "particleSystem") {
		TParticleSystem ps = TParticleSystem();
		ps.h_transform = h_transform;
		ps.h_pg = CHandle(this);
		particle_systems.push_back(ps);		
	}
	else if (particle_systems.size() > 0) {
		particle_systems.back().loadFromAtts(elem, atts);
	}
	
}

void TCompParticleGroup::init() {
	if (def_name != "unnamed") {
		CEntity* m_entity = CHandle(this).getOwner();
		particle_groups_manager.addParticleGroupToEntity(m_entity, def_name);
	}

	for (auto& ps : particle_systems) {
		ps.init();
	}
}

void TCompParticleGroup::fixedUpdate(float elapsed) {
	for (auto& ps : particle_systems) {
		ps.fixedUpdate(elapsed);
	}
}

void TCompParticleGroup::update(float elapsed) {

	/*if (destroy_on_death && particle_systems.size() == 0) {
		CEntityManager::get().remove(CHandle(this).getOwner());
	}*/

	bool all_dirty = true;
	for (auto& ps : particle_systems) {
		ps.update(elapsed);
		all_dirty &= ps.dirty_destroy_group;
	}

	if (destroy_on_death && all_dirty) {
		clearParticleSystems();

		/*CEntity* e = CHandle(this).getOwner();
		CEntityManager::get().remove(e);*/
	}

	for (auto& it : particle_systems_to_destroy) {

		auto it2 = std::find(particle_systems.begin(), particle_systems.end(), it);
		
		if (it2 == particle_systems.end()) { fatal("Trying to destroy not registered particle system"); }
		it2->destroy();
		particle_systems.erase(it2);
		
	};

	particle_systems_to_destroy.clear();

	if (destroy_on_death && all_dirty) {

		CEntity* e = CHandle(this).getOwner();
		CEntityManager::get().remove(e);
	}
}

void TCompParticleGroup::render() {
	TCompAABB* aabb = getSibling<TCompAABB>(this);
	if (render_manager.planes_active_camera.isVisible(aabb)) {
		for (auto& ps : particle_systems) {
			ps.render(false);
		}
	}
}

void TCompParticleGroup::renderDistorsion() {
	for (auto& ps : particle_systems) {
		ps.render(true);
	}
}

void TCompParticleGroup::renderDebug3D() const {
	for (auto& ps : particle_systems) {
		ps.renderDebug3D();
	}
}

void TCompParticleGroup::removeParticleSystem(TParticleSystem* ps) {
	for (auto& ps_2 : particle_systems) {
		if (ps_2.emitter_generation == ps->emitter_generation) {
			ps_2.dirty_destroy_group = true;
			particle_systems_to_destroy.push_back(ps_2);
		}
	}
}

void TCompParticleGroup::clearParticleSystems() {
	for (int i = 0; i < particle_systems.size(); ++i) {
		particle_systems[i].destroy();
	}
	particle_systems.clear();
}

void TCompParticleGroup::restart() {
	for (auto& ps : particle_systems) {
		ps.restart();
	}
}

std::string TCompParticleGroup::getXMLDefinition() {
	std::string def = "";

	def += "<particleGroup>";

	for (auto& ps : particle_systems) {
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

	for (auto& ps : particle_systems) {
		def += ps.getXMLDefinition();
	}

	def += "</particleGroup>";

	return def;
}