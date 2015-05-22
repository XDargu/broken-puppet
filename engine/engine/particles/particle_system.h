#ifndef INC_PARTICLE_SYSTEM_H_
#define INC_PARTICLE_SYSTEM_H_

#include "utils.h"
#include "handle\handle.h"
#include "particle.h"

typedef std::vector<TParticle> VParticles;

struct TParticleEmitterDirection {
protected:
public:
};

struct TParticleEmitterGeneration {
protected:
	float rate_counter;
public:
	VParticles* particles;
	// Emission rate (particles / second)
	float rate;
	// Position of the emitter (Transform position)
	XMVECTOR position;
	// Lifetime of the particles
	float min_life_time;
	float max_life_time;
	// Should the particle system generate the particles at the begining?
	bool fill_initial;
	// Max amount of particles
	int limit;

	virtual void update(float elapsed) = 0;
};

struct TParticleEmitterGenerationSphere : TParticleEmitterGeneration {

	float radius;

	TParticleEmitterGenerationSphere(VParticles* the_particles, XMVECTOR the_position, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, bool the_fill_initial, int the_limit) {
		radius = the_radius;
		position = the_position;
		min_life_time = the_min_life_time;
		max_life_time = the_max_life_time;
		rate = the_rate;
		rate_counter = 0;
		fill_initial = the_fill_initial;
		limit = the_limit;
		particles = the_particles;

		if (fill_initial) {
			// Make the initial particles
			for (int i = 0; i < limit; ++i) {
				addParticle();
			}
		}
	}
	void update(float elapsed) {
		rate_counter += elapsed;

		// If we have to make a new particle
		if (rate != 0 && rate_counter > rate && particles->size() < limit){
			addParticle();
		}
	}

	void addParticle() {
		XMFLOAT3 pos;
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		bool insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		while (!insideSphere) {
			XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
			insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		}
		XMStoreFloat3(&pos, position + XMLoadFloat3(&pos));
		float life_time = getRandomNumber(min_life_time, max_life_time);
		TParticle n_particle = TParticle(
			pos
			, 0
			, life_time
			, XMVectorSet(1, 1, 1, 1)
			, 1
			);
		particles->push_back(n_particle);
		rate_counter = 0;
	}
};

struct TParticleRenderer {
	VParticles* particles;

	void update(TParticle* particle, float elapsed) {
		particle->age += elapsed;
	}
};

struct TParticleUpdaterMovement {
	XMVECTOR speed;

	void update(TParticle* particle, float elapsed) {
		XMStoreFloat3(&particle->position, XMLoadFloat3(&particle->position) + speed);
	}
};

struct TParticleUpdaterPhysx {
};

struct TParticleUpdaterLifeTime {
	void update(TParticle* particle, float elapsed) {
		particle->age += elapsed;
	}
};

struct TParticleUpdaterSize {
	float initial_size;
	float final_size;

	void update(TParticle* particle, float elapsed) {
		particle->size = lerp(initial_size, final_size, particle->age / particle->lifespan);
	}
};

struct TParticleUpdaterColor {
	XMVECTOR initial_color;
	XMVECTOR final_color;

	void update(TParticle* particle, float elapsed) {
		//particle->color = XMVectorLerp(initial_color, final_color, particle->age / particle->lifespan);
	}
};

// Particle system
struct TParticleSystem {
	// Vector of particles
	VParticles particles;

	// Updaters
	TParticleUpdaterLifeTime* updater_lifetime;
	TParticleUpdaterSize* updater_size;
	TParticleUpdaterColor* updater_color;
	TParticleUpdaterMovement* updater_movement;

	// Emitter shape
	TParticleEmitterGeneration* emitter_generation;

	// Transform reference
	CHandle h_transform;

	const CMesh* instanced_mesh;      // The particle mesh
	CMesh* instances_data;      // The positions of each instance
public:

	TParticleSystem() : updater_lifetime(nullptr)
		, updater_size(nullptr)
		, updater_color(nullptr)
		, updater_movement(nullptr)

		, emitter_generation(nullptr)

		, h_transform(CHandle())

		, instanced_mesh(nullptr)
		, instances_data(nullptr)
	{}

	void destroy() {
		SAFE_DELETE(updater_lifetime);
		SAFE_DELETE(updater_size);
		SAFE_DELETE(updater_color);
		SAFE_DELETE(updater_movement);

		SAFE_DELETE(emitter_generation);

		if (instances_data) {
			instances_data->destroy();
			delete instances_data;
		}
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void render();
	void renderDebug3D() const;
};

#endif