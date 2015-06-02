#ifndef INC_PARTICLE_SYSTEM_H_
#define INC_PARTICLE_SYSTEM_H_

#include "utils.h"
#include "handle\handle.h"
#include "particle.h"
#include "particle_system_subtypes.h"

typedef std::vector<TParticle> VParticles;

// Particle system
struct TParticleSystem {
	// Vector of particles
	VParticles particles;

	// Updaters
	TParticleUpdaterLifeTime* updater_lifetime;
	TParticleUpdaterSize* updater_size;
	TParticleUpdaterColor* updater_color;
	TParticleUpdaterGravity* updater_gravity;
	TParticleUpdaterMovement* updater_movement;
	TParticleUpdaterNoise* updater_noise;

	// Emitter shape
	TParticleEmitterGeneration* emitter_generation;

	// Renderer
	TParticleRenderer* renderer;

	// Transform reference
	CHandle h_transform;

	const CMesh* instanced_mesh;      // The particle mesh
	CMesh* instances_data;      // The positions of each instance
public:

	TParticleSystem() : updater_lifetime(nullptr)
		, updater_size(nullptr)
		, updater_color(nullptr)
		, updater_movement(nullptr)
		, updater_gravity(nullptr)
		, updater_noise(nullptr)

		, emitter_generation(nullptr)

		, renderer(nullptr)

		, h_transform(CHandle())

		, instanced_mesh(nullptr)
		, instances_data(nullptr)

	{
		
	}

	void destroy() {
		SAFE_DELETE(updater_lifetime);
		SAFE_DELETE(updater_size);
		SAFE_DELETE(updater_color);
		SAFE_DELETE(updater_movement);
		SAFE_DELETE(updater_gravity);
		SAFE_DELETE(updater_noise);

		SAFE_DELETE(emitter_generation);

		SAFE_DELETE(renderer);

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

	void changeLimit(int the_limit);

	int getLimit();
};

#endif