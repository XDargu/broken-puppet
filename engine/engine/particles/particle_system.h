#ifndef INC_PARTICLE_SYSTEM_H_
#define INC_PARTICLE_SYSTEM_H_

#include "utils.h"
#include "handle\handle.h"
#include "particle.h"
#include "particle_system_subtypes.h"
#include "aabb.h"

typedef std::vector<TParticle> VParticles;

// Particle system
struct TParticleSystem {
	// Vector of particles
	VParticles particles;

	//Physx Particles
	CPhysicsParticleSystem* psx;
	bool use_physx;
	bool dirty_destroy_group;

	// Updaters
	TParticleUpdaterLifeTime* updater_lifetime;
	TParticleUpdaterSize* updater_size;
	TParticleUpdaterColor* updater_color;
	TParticleUpdaterGravity* updater_gravity;
	TParticleUpdaterRotation* updater_rotation;
	TParticleUpdaterMovement* updater_movement;
	TParticleUpdaterNoise* updater_noise;
	TParticleUpdaterPhysx* updater_physx;

	// Subemitter
	TParticleSubemitter* subemitter;

	// Emitter shape
	TParticleEmitterGeneration* emitter_generation;

	// Renderer
	TParticleRenderer* renderer;

	// Transform reference
	CHandle h_transform;
	// Group reference
	CHandle h_pg;

	AABB aabb;

	const CMesh* instanced_mesh;      // The particle mesh
	CMesh* instances_data;      // The positions of each instance

	bool visible; // Is visible vy the camera
public:

	TParticleSystem();
		

	void destroy() {
		SAFE_DELETE(updater_lifetime);
		SAFE_DELETE(updater_size);
		SAFE_DELETE(updater_color);
		SAFE_DELETE(updater_movement);
		SAFE_DELETE(updater_rotation);
		SAFE_DELETE(updater_gravity);
		SAFE_DELETE(updater_noise);
		SAFE_DELETE(updater_physx);
		
		SAFE_DELETE(subemitter);

		SAFE_DELETE(emitter_generation);

		SAFE_DELETE(renderer);
		if (psx != nullptr) { psx->releaseAllParticles(); }		
		SAFE_DELETE(psx);

		if (instances_data) {
			instances_data->destroy();
			delete instances_data;
		}
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);
	void fixedUpdate(float elapsed);

	void render(bool distorsion);
	void renderDebug3D() const;

	void changeLimit(int the_limit);

	int getLimit();

	void loadDefaultPS();

	void restart();

	std::string getXMLDefinition();

	bool operator==(const TParticleSystem ps) const {
		return ps.emitter_generation == emitter_generation;
	}
};

#endif