#include "mcv_platform.h"
#include "particle_system.h"
#include "render\render_utils.h"
#include "components\comp_transform.h"

void TParticleSystem::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	TCompTransform* m_transform = h_transform;

	if (elem == "updater") {
		std::string updater_type = atts.getString("type", "");
		if (updater_type == "lifeTime") {
			updater_lifetime = new TParticleUpdaterLifeTime();
		}
		if (updater_type == "size") {
			float initial_size = atts.getFloat("initialSize", 1);
			float final_size = atts.getFloat("finalSize", 1);
			updater_size = new TParticleUpdaterSize(initial_size, final_size);
		}
		if (updater_type == "color") {
			XMVECTOR initial_color = atts.getPoint("initialColor");
			XMVECTOR final_color = atts.getPoint("finalColor");
			updater_color = new TParticleUpdaterColor();
			updater_color->initial_color = initial_color;
			updater_color->final_color = final_color;
		}
		if (updater_type == "movement") {
			updater_movement = new TParticleUpdaterMovement();
			updater_movement->speed = atts.getFloat("speed", 1);
		}
		if (updater_type == "gravity") {
			float gravity = atts.getFloat("gravity", 1);
			updater_gravity = new TParticleUpdaterGravity(gravity);
		}
	}

	if (elem == "emitter") {
		XASSERT(emitter_generation == nullptr, "Emitter already declared, a particle system can't have two different emitters");
		std::string emitter_type = atts.getString("type", "");

		float rate = atts.getFloat("rate", 0.1);
		float min_life_time = atts.getFloat("minLifeTime", 5);
		float max_life_time = atts.getFloat("maxLifeTime", 5);
		bool fill_initial = atts.getBool("fillInitial", false);
		int limit = atts.getInt("limit", 1000);
		float burst_time = atts.getFloat("burstTime", 0);
		int burst_amount = atts.getInt("burstAmount", 100);
		particles.reserve(limit);
		
		// Instancing
		instanced_mesh = &mesh_textured_quad_xy_centered;

		// This mesh has not been registered in the mesh manager
		instances_data = new CMesh;
		bool is_ok = instances_data->create(limit, &particles
			, 0, nullptr        // No indices
			, CMesh::POINTS     // We are not using this
			, &vdcl_particle_data    // Type of vertex
			, true              // the buffer IS dynamic
			);

		if (emitter_type == "sphere") {
			float radius = atts.getFloat("radius", 1);			
			emitter_generation = new TParticleEmitterGenerationSphere(&particles, h_transform, rate, min_life_time, max_life_time, radius, fill_initial, limit, burst_time, burst_amount);
		}

		if (emitter_type == "semiSphere") {
			float radius = atts.getFloat("radius", 1);
			emitter_generation = new TParticleEmitterGenerationSemiSphere(&particles, h_transform, rate, min_life_time, max_life_time, radius, fill_initial, limit, burst_time, burst_amount);
		}

		if (emitter_type == "cone") {
			float radius = atts.getFloat("radius", 1);
			float angle = deg2rad(atts.getFloat("angle", 30));
			emitter_generation = new TParticleEmitterGenerationCone(&particles, h_transform, rate, min_life_time, max_life_time, radius, angle, fill_initial, limit, burst_time, burst_amount);
		}

		if (emitter_type == "ring") {
			float inner_radius = atts.getFloat("innerRadius", 0.5);
			float outer_radius = atts.getFloat("outerRadius", 1);
			emitter_generation = new TParticleEmitterGenerationRing(&particles, h_transform, rate, min_life_time, max_life_time, inner_radius, outer_radius, fill_initial, limit, burst_time, burst_amount);
		}

		if (emitter_type == "box") {
			float size = atts.getFloat("size", 1);			
			emitter_generation = new TParticleEmitterGenerationBox(&particles, h_transform, rate, min_life_time, max_life_time, size, fill_initial, limit, burst_time, burst_amount);
		}
	}

	if (elem == "renderer") {
		std::string texture_name = atts.getString("texture", "unknown");
		bool is_aditive = atts.getBool("aditive", true);
		renderer = new TParticleRenderer(&particles, texture_name.c_str(), is_aditive);
		
	}
	
}

void TParticleSystem::init() {

}

void TParticleSystem::update(float elapsed) {
	if (emitter_generation != nullptr) {
		emitter_generation->particles = &particles;
		emitter_generation->update(elapsed);
	}

	VParticles::iterator it = particles.begin();

	int delete_counter = 0;
	while (it != particles.end()) {

		if (it->size != -1) {

			if (updater_movement != nullptr) {
				updater_movement->update(&(*it), elapsed);
			}
			if (updater_gravity != nullptr) {
				updater_gravity->update(&(*it), elapsed);
			}
			if (updater_color != nullptr) {
				updater_color->update(&(*it), elapsed);
			}
			if (updater_lifetime != nullptr) {
				updater_lifetime->update(&(*it), elapsed);
			}
			if (updater_size != nullptr) {
				updater_size->update(&(*it), elapsed);
			}

			if (it->lifespan != 0 && it->age > it->lifespan) {
				it = particles.erase(it);
				delete_counter++;
			}
			else ++it;
		}
		else {
			++it;
		}
		//++it;
	}

	/*for (int i = 0; i < delete_counter; ++i) {
		particles.push_front(TParticle());		
	}*/


	
	// Update particles using some cpu code
	//int idx = 1;
	/*for (auto& p : particles) {
	//p.nframe += elapsed;
	//p.pos.y -= random(1.f, 3.f) * elapsed* 2;
	//if (p.pos.y < 0)
	//  p.pos.y += 50.f;
	++idx;
	}*/

	if (instances_data != nullptr && particles.size() > 0) {
		instances_data->updateFromCPU(&particles[0]);
	}
}

void TParticleSystem::render() {
	if (instances_data != nullptr) {
		setWorldMatrix(XMMatrixIdentity());
		CTraceScoped t0("instances");
		render_techniques_manager.getByName("particles")->activate();		
		texture_manager.getByName(renderer->texture)->activate(0);
		if (renderer->additive)
			activateBlendConfig(BLEND_CFG_ADDITIVE_BY_SRC_ALPHA);
		else
			activateBlendConfig(BLEND_CFG_COMBINATIVE);
		activateZConfig(ZCFG_TEST_BUT_NO_WRITE);
		instanced_mesh->renderInstanced(*instances_data, particles.size());
		activateZConfig(ZCFG_DEFAULT);
		activateBlendConfig(BLEND_CFG_DEFAULT);
	}
}

void TParticleSystem::renderDebug3D() const {
	render_techniques_manager.getByName("basic")->activate();
	return;
	for (auto& particle : particles) {
		XMVECTOR pos = XMLoadFloat3(&particle.position);
		XMVECTOR rot = XMVectorSet(0, 0, 0, 1);
		XMVECTOR scale = XMVectorSet(0.1, 0.1, 0.1, 0.1);
		XMVECTOR zero = XMVectorSet(0, 0, 0, 0);
		setWorldMatrix(XMMatrixAffineTransformation(scale, zero, rot, pos));
		axis.activateAndRender();
	}
}