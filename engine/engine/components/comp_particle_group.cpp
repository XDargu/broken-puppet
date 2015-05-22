#include "mcv_platform.h"
#include "comp_particle_group.h"
#include "comp_transform.h"
#include "physics_manager.h"
#include "render\render_utils.h"

void TCompParticleGroup::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	CHandle h_transform = assertRequiredComponent<TCompTransform>(this);
	TCompTransform* m_transform = h_transform;

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