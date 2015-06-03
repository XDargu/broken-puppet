#include "mcv_platform.h"
#include "comp_particle_group.h"
#include "comp_transform.h"
#include "physics_manager.h"
#include "render\render_utils.h"
#include "particles\importer_particle_groups.h"

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

void TCompParticleGroup::update(float elapsed) {
	for (auto& ps : particle_systems) {
		ps.update(elapsed);
	}
}

void TCompParticleGroup::render() {
	for (auto& ps : particle_systems) {
		ps.render();
	}
	
}

void TCompParticleGroup::renderDebug3D() const {
	for (auto& ps : particle_systems) {
		ps.renderDebug3D();
	}
}

void TCompParticleGroup::removeParticleSystem(TParticleSystem* ps) {
	std::vector<TParticleSystem>::iterator it = particle_systems.begin();

	while (it != particle_systems.end()) {
		if (&it->emitter_generation == &ps->emitter_generation) {
			it = particle_systems.erase(it);
		}
		else ++it;
	}
}

void TCompParticleGroup::clearParticleSystems() {
	for (int i = 0; i < particle_systems.size(); ++i) {
		particle_systems[i].destroy();
	}
	particle_systems.clear();
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