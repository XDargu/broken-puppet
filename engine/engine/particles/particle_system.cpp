#include "mcv_platform.h"
#include "particle_system.h"
#include "render\render_utils.h"
#include "components\comp_transform.h"
#include "render\ctes\shader_ctes.h"
#include "render\render_utils.h"

extern CShaderCte<TCtesParticleSystem> ctes_particle_system;

TParticleSystem::TParticleSystem() : updater_lifetime(nullptr)
, updater_size(nullptr)
, updater_color(nullptr)
, updater_movement(nullptr)
, updater_gravity(nullptr)
, updater_rotation(nullptr)
, updater_noise(nullptr)
, updater_physx(nullptr)

, emitter_generation(nullptr)

, renderer(nullptr)
, psx(nullptr)

, h_transform(CHandle())
, use_physx(false)

, instanced_mesh(nullptr)
, instances_data(nullptr)
{	
}

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
		if (updater_type == "rotation") {
			float angular_speed = atts.getFloat("angular_speed", 0.5f);
			updater_rotation = new TParticleUpdaterRotation(angular_speed);
		}
		if (updater_type == "gravity") {
			float gravity = atts.getFloat("gravity", 1);
			bool constant = atts.getBool("constant", false);
			updater_gravity = new TParticleUpdaterGravity(gravity, constant);
		}
		if (updater_type == "noise") {
			XMVECTOR min_noise = atts.getPoint("minNoise");
			XMVECTOR max_noise = atts.getPoint("maxNoise");
			updater_noise = new TParticleUpdaterNoise(min_noise, max_noise);
		}
		if (updater_type == "physx") {
			bool gravity = atts.getBool("gravity", true);
			updater_physx = new TParticleUpdaterPhysx(this);
			psx->setParticlesNoGravity(!gravity);			
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
		bool loop = atts.getBool("loop", true);
		bool random_rotation = atts.getBool("randomRotation", true);
		float delay = atts.getFloat("delay", 0);
		particles.reserve(limit);

		// Physx
		use_physx = atts.getBool("use_physx", false);
		psx = new CPhysicsParticleSystem();
		psx->createParticles(limit);
		
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
			emitter_generation = new TParticleEmitterGeneration(this, TParticleEmitterShape::SPHERE, h_transform, rate, min_life_time, max_life_time, radius, fill_initial, limit, burst_time, burst_amount, delay, loop, random_rotation);
		}

		if (emitter_type == "semiSphere") {
			float radius = atts.getFloat("radius", 1);
			emitter_generation = new TParticleEmitterGeneration(this, TParticleEmitterShape::SEMISPHERE, h_transform, rate, min_life_time, max_life_time, radius, fill_initial, limit, burst_time, burst_amount, delay, loop, random_rotation);
		}

		if (emitter_type == "cone") {
			float radius = atts.getFloat("radius", 1);
			float angle = deg2rad(atts.getFloat("angle", 30));
			emitter_generation = new TParticleEmitterGeneration(this, TParticleEmitterShape::CONE, h_transform, rate, min_life_time, max_life_time, radius, angle, fill_initial, limit, burst_time, burst_amount, delay, loop, random_rotation);
		}

		if (emitter_type == "ring") {
			float inner_radius = atts.getFloat("innerRadius", 0.5);
			float outer_radius = atts.getFloat("outerRadius", 1);
			emitter_generation = new TParticleEmitterGeneration(this, TParticleEmitterShape::RING, h_transform, rate, min_life_time, max_life_time, outer_radius, inner_radius, fill_initial, limit, burst_time, burst_amount, delay, loop, random_rotation);
		}

		if (emitter_type == "box") {
			float size = atts.getFloat("size", 1);			
			emitter_generation = new TParticleEmitterGeneration(this, TParticleEmitterShape::BOX, h_transform, rate, min_life_time, max_life_time, size, fill_initial, limit, burst_time, burst_amount, delay, loop, random_rotation);
		}

	}

	if (elem == "renderer") {
		std::string texture_name = atts.getString("texture", "unknown");
		bool is_aditive = atts.getBool("aditive", true);
		int n_anim_x = atts.getInt("n_anim_x", 1);
		int n_anim_y = atts.getInt("n_anim_y", 1);
		int animation_mode = atts.getInt("animationMode", 0);
		float stretch = atts.getFloat("stretch", 2);
		int stretch_mode = atts.getInt("stretchMode", 0);
		std::string str_mode = atts.getString("render_mode", "billboard");
		TParticleRenderType type = TParticleRenderType::BILLBOARD;
		if (str_mode == "billboard") { type = TParticleRenderType::BILLBOARD; }
		if (str_mode == "h_billboard") { type = TParticleRenderType::H_BILLBOARD; }
		if (str_mode == "h_dir_billboard") { type = TParticleRenderType::H_DIR_BILLBOARD; }
		if (str_mode == "v_billboard") { type = TParticleRenderType::V_BILLBOARD; }
		if (str_mode == "stretched_billboard") { type = TParticleRenderType::STRETCHED_BILLBOARD; }

		renderer = new TParticleRenderer(&particles, texture_name.c_str(), is_aditive, type, n_anim_x, n_anim_y, stretch, animation_mode, stretch_mode);
	}
	
}

void TParticleSystem::init() {
	if (particles.capacity() < emitter_generation->limit) {
		particles.reserve(emitter_generation->limit - particles.capacity());
	}	
}

void TParticleSystem::update(float elapsed) {
	if (emitter_generation != nullptr) {
		emitter_generation->ps = this;
		emitter_generation->update(elapsed);
	}
	if (updater_physx != nullptr) {
		updater_physx->ps = this;
	}

	VParticles::iterator it = particles.begin();

	int delete_counter = 0;
	while (it != particles.end()) {

		if (it->size != -1) {
			// Update the position, no updater needed
			it->position.x += it->speed.x * elapsed;
			it->position.y += it->speed.y * elapsed;
			it->position.z += it->speed.z * elapsed;

			if (updater_rotation != nullptr) {
				updater_rotation->update(&(*it), elapsed);
			}
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
			if (updater_noise != nullptr) {
				updater_noise->update(&(*it), elapsed);
			}
			if (updater_physx != nullptr) {
				// Unlock
				updater_physx->update(&(*it), elapsed);
				// Lock
			}
			if (it->lifespan != 0 && it->age > it->lifespan) {
				if (use_physx) {
					PxU32 indicesToRelease[1];
					indicesToRelease[0] = it->index;
					psx->releaseParticles(1, indicesToRelease);
				}
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
		instances_data->updateFromCPU(&particles[0], particles.size() * sizeof(TParticle));
	}
}

void TParticleSystem::render() {
	if (instances_data != nullptr) {
		TCtesParticleSystem* ps_ctes = ctes_particle_system.get();
		ps_ctes->n_imgs_x = renderer->n_anim_x;
		ps_ctes->n_imgs_y = renderer->n_anim_y;
		ps_ctes->stretch = renderer->stretch;
		ps_ctes->render_mode = renderer->render_type;
		ps_ctes->stretch_mode = renderer->stretch_mode;
		ps_ctes->animation_mode = renderer->particle_animation_mode;
		ctes_particle_system.uploadToGPU();
		ctes_particle_system.activateInVS(5);

		setWorldMatrix(XMMatrixIdentity());
		CTraceScoped t0("Particle system");
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

void TParticleSystem::changeLimit(int the_limit) {
	// Set the new limit
	emitter_generation->limit = the_limit;

	// Remove extra particles, if needed
	int amount_to_erase = max(0, particles.size() - the_limit);

	for (int i = 0; i < amount_to_erase; ++i) {
		particles.pop_back();
	}

	// If the capacity is less than the new limit, reserve more memory
	if (particles.capacity() < the_limit) {
		particles.reserve(the_limit);
	}

	// Change the buffer mesh
	if (instances_data) {
		instances_data->destroy();
		delete instances_data;
	}

	void *data = nullptr;
	if (particles.empty()) {
		particles.resize(1);
		data = &particles[0];
		particles.clear();
	}
	else {
		data = &particles[0];
	}

	instances_data = new CMesh;
	bool is_ok = instances_data->create(the_limit, data
		, 0, nullptr        // No indices
		, CMesh::POINTS     // We are not using this
		, &vdcl_particle_data    // Type of vertex
		, true              // the buffer IS dynamic
		);
}

int TParticleSystem::getLimit() {
	return emitter_generation->limit;
}

void TParticleSystem::loadDefaultPS() {

	TCompTransform* m_transform = h_transform;

	emitter_generation = new TParticleEmitterGeneration(
		this, TParticleEmitterShape::BOX, m_transform, 0.05f, 1, 2, 1, false, 100, 0, 0, 0, false, true);
	renderer = new TParticleRenderer(&particles, "smoke", false, TParticleRenderType::BILLBOARD, 4, 4, 1, 0, 0);

	updater_lifetime = new TParticleUpdaterLifeTime();
	updater_movement = new TParticleUpdaterMovement();

	// Instancing
	instanced_mesh = &mesh_textured_quad_xy_centered;

	// This mesh has not been registered in the mesh manager
	instances_data = new CMesh;
	bool is_ok = instances_data->create(100, &particles
		, 0, nullptr        // No indices
		, CMesh::POINTS     // We are not using this
		, &vdcl_particle_data    // Type of vertex
		, true              // the buffer IS dynamic
		);

	particles.reserve(100);
}

void TParticleSystem::restart() {
	particles.clear();
	emitter_generation->restart();
}

std::string TParticleSystem::getXMLDefinition() {

	std::string def = "";

	def += "<particleSystem>";

	def += emitter_generation->getXMLDefinition();

	if (updater_movement != nullptr) {
		def += updater_movement->getXMLDefinition();
	}
	if (updater_rotation != nullptr) {
		def += updater_rotation->getXMLDefinition();
	}
	if (updater_gravity != nullptr) {
		def += updater_gravity->getXMLDefinition();
	}
	if (updater_color != nullptr) {
		def += updater_color->getXMLDefinition();
	}
	if (updater_lifetime != nullptr) {
		def += updater_lifetime->getXMLDefinition();
	}
	if (updater_size != nullptr) {
		def += updater_size->getXMLDefinition();
	}
	if (updater_noise != nullptr) {
		def += updater_noise->getXMLDefinition();
	}

	def += renderer->getXMLDefinition();

	def += "</particleSystem>";

	return def;
}