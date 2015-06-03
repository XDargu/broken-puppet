#ifndef INC_PARTICLE_SYSTEM_SUBTYPES_H_
#define INC_PARTICLE_SYSTEM_SUBTYPES_H_

#include "utils.h"
#include "handle\handle.h"
#include "particle.h"

typedef std::vector<TParticle> VParticles;

// --------------------------------- EMITTERS ------------------------------

enum TParticleEmitterShape {
	SPHERE = 0,
	SEMISPHERE = 1,
	CONE = 2,
	RING = 3,
	BOX = 4,
	COUNT
};

struct TParticleEmitterGeneration {
protected:
	float rate_counter;
public:
	VParticles* particles;
	// Emitter shape
	TParticleEmitterShape shape;
	// Emission rate (particles / second)
	float rate;
	// Transform of the emitter parent entity
	CHandle h_transform;
	// Lifetime of the particles
	float min_life_time;
	float max_life_time;
	// Should the particle system generate the particles at the begining?
	bool fill_initial;
	// Max amount of particles
	int limit;

	// Time between bursts
	float burst_time;
	float burst_counter;
	int burst_amount;

	// Values per shape
	float radius;
	float inner_radius;
	float angle;
	float box_size;

	// Sphere / Semisphere
	TParticleEmitterGeneration(VParticles* the_particles, TParticleEmitterShape the_shape, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius_or_box_size, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount);
	// Cone
	TParticleEmitterGeneration(VParticles* the_particles, TParticleEmitterShape the_shape, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, float the_angle_or_inner_radius, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount);

	void fillInitial();

	virtual void update(float elapsed);

	void addParticle();
};

// --------------------------------- RENDERER ------------------------------

struct TParticleRenderer {
	VParticles* particles;
	char texture[64];
	bool additive;

	TParticleRenderer(VParticles* the_particles, const char* the_texture, bool is_aditive);

	void update(TParticle* particle, float elapsed);
};

// --------------------------------- UPDATERS ------------------------------

struct TParticleUpdaterMovement {
	float speed;

	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterPhysx {

};

struct TParticleUpdaterGravity {
	float gravity;

	TParticleUpdaterGravity(float the_gravity) : gravity(the_gravity) {}
	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterLifeTime {
	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterSize {
	float initial_size;
	float final_size;

	TParticleUpdaterSize(float the_initial_size, float the_final_size);

	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterColor {
	XMVECTOR initial_color;
	XMVECTOR final_color;

	TParticleUpdaterColor();
	TParticleUpdaterColor(XMVECTOR the_initial_color, XMVECTOR the_final_color);

	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterNoise {
	XMFLOAT3 min_noise;
	XMFLOAT3 max_noise;

	TParticleUpdaterNoise(XMVECTOR the_min_noise, XMVECTOR the_max_noise) { XMStoreFloat3(&min_noise, the_min_noise); XMStoreFloat3(&max_noise, the_max_noise); };
	void update(TParticle* particle, float elapsed);
};

#endif