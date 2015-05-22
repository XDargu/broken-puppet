#include "mcv_platform.h"
#include "comp_particle_system.h"
#include "comp_transform.h"
#include "physics_manager.h"
#include "render\render_utils.h"

void TCompParticleSystem::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
	TCompTransform* m_transform = h_transform;

	if (elem == "updater") {
		std::string updater_type =  atts.getString("type", "");
		if (updater_type == "lifetime") {
			updater_lifetime = new TParticleUpdaterLifeTime();
		}
		if (updater_type == "size") {
			updater_size = new TParticleUpdaterSize();
		}
		if (updater_type == "color") {
			updater_color = new TParticleUpdaterColor();
		}
		if (updater_type == "movement") {
			updater_movement = new TParticleUpdaterMovement();
			updater_movement->speed = atts.getPoint("speed");
		}
	}

	if (elem == "emitter") {
		std::string emitter_type = atts.getString("type", "");

		XMVECTOR position = m_transform->position;

		if (emitter_type == "sphere") {
			float radius = atts.getFloat("radius", 1);
			float rate = atts.getFloat("rate", 0.1);
			float life_time = atts.getFloat("lifeTime", 5);
			
			emitter_generation = new TParticleEmitterGenerationSphere(position, rate, life_time, radius);
			emitter_generation->particles = &particles;
		}
	}

	if (elem == "renderer") {

	}

	// Instancing
	instanced_mesh = &mesh_textured_quad_xy_centered;

	particles.resize(1000);

	// This mesh has not been registered in the mesh manager
	instances_data = new CMesh;
	bool is_ok = instances_data->create(particles.size(), &particles[0]
		, 0, nullptr        // No indices
		, CMesh::POINTS     // We are not using this
		, &vdcl_particle_data    // Type of vertex
		, true              // the buffer IS dynamic
		);
}

void TCompParticleSystem::init() {

}

void TCompParticleSystem::update(float elapsed) {
	if (emitter_generation != nullptr) {
		TCompTransform* m_transform = h_transform;
		emitter_generation->position = m_transform->position;
		emitter_generation->update(elapsed);
	}

	VParticles::iterator it = particles.begin();

	int delete_counter = 0;
	while (it != particles.end()) {

		if (it->size != -1) {

			if (updater_movement != nullptr) {
				updater_movement->update(&(*it), elapsed);
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
	}

	for (int i = 0; i < delete_counter; ++i) {
		particles.push_front(TParticle());
	}



	// Update particles using some cpu code
	int idx = 1;
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

void TCompParticleSystem::render() {
	return;
	if (instances_data != nullptr) {
		setWorldMatrix(XMMatrixIdentity());
		CTraceScoped t0("instances");
		render_techniques_manager.getByName("particles")->activate();
		texture_manager.getByName("fire")->activate(0);
		activateBlendConfig(BLEND_CFG_ADDITIVE_BY_SRC_ALPHA);
		activateZConfig(ZCFG_TEST_BUT_NO_WRITE);
		instanced_mesh->renderInstanced(*instances_data, particles.size());
		activateZConfig(ZCFG_DEFAULT);
		activateBlendConfig(BLEND_CFG_DEFAULT);
	}
}

void TCompParticleSystem::renderDebug3D() const {
	for (auto& particle : particles) {
		XMVECTOR pos = XMLoadFloat3(&particle.position);
		XMVECTOR rot = XMVectorSet(0, 0, 0, 1);
		XMVECTOR scale = XMVectorSet(1, 1, 1, 1);
		XMVECTOR zero = XMVectorSet(0, 0, 0, 0);
		setWorldMatrix(XMMatrixAffineTransformation(scale, zero, rot, pos));
		axis.activateAndRender();
	}
}