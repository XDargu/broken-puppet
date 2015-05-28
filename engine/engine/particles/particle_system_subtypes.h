#ifndef INC_PARTICLE_SYSTEM_SUBTYPES_H_
#define INC_PARTICLE_SYSTEM_SUBTYPES_H_

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

	virtual void update(float elapsed) = 0;
};

struct TParticleEmitterGenerationSphere : TParticleEmitterGeneration {

	float radius;

	TParticleEmitterGenerationSphere(VParticles* the_particles, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount);

	void update(float elapsed);

	void addParticle();
};

struct TParticleEmitterGenerationSemiSphere : TParticleEmitterGeneration {

	float radius;

	TParticleEmitterGenerationSemiSphere(VParticles* the_particles, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount);

	void update(float elapsed);

	void addParticle();
};

struct TParticleEmitterGenerationCone : TParticleEmitterGeneration {

	float radius;
	float angle;

	TParticleEmitterGenerationCone(VParticles* the_particles, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, float the_angle, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount);

	void update(float elapsed);

	void addParticle();
};

struct TParticleEmitterGenerationBox : TParticleEmitterGeneration {

	float size;

	TParticleEmitterGenerationBox(VParticles* the_particles, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_size, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount);

	void update(float elapsed);

	void addParticle();
};

struct TParticleRenderer {
	VParticles* particles;
	char texture[64];
	bool additive;

	TParticleRenderer(VParticles* the_particles, const char* the_texture, bool is_aditive);

	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterMovement {
	float speed;

	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterPhysx {
};

struct TParticleUpdaterGravity {
	float gravity;

	TParticleUpdaterGravity::TParticleUpdaterGravity(float the_gravity) : gravity(the_gravity) {}
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

	void update(TParticle* particle, float elapsed);
};

#endif