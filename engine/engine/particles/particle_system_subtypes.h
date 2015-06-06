#ifndef INC_PARTICLE_SYSTEM_SUBTYPES_H_
#define INC_PARTICLE_SYSTEM_SUBTYPES_H_

#include "utils.h"
#include "handle\handle.h"
#include "particle.h"
#include "physics_manager.h"

struct TParticleSystem;

typedef std::vector<TParticle> VParticles;

// --------------------------------- EMITTERS ------------------------------

enum TParticleEmitterShape {
	SPHERE = 0,
	SEMISPHERE = 1,
	CONE = 2,
	RING = 3,
	BOX = 4,
};

enum TParticleRenderType {
	BILLBOARD = 0,
	V_BILLBOARD = 1,
	H_BILLBOARD = 2,
	H_DIR_BILLBOARD = 3,
	STRETCHED_BILLBOARD = 4,
};

enum TParticleCurve {
	LINEAL = 0,
	LOGARITHM = 1,
	EXPONENTIAL = 2
};

float getCurveVal(TParticleCurve curve, float curve_val, float min, float max, float t);
XMFLOAT3 getCurveVal(TParticleCurve curve, float curve_val, XMFLOAT3 min, XMFLOAT3 max, float t);

struct TParticleEmitterGeneration {
protected:
	float rate_counter;
	float delay_counter;
public:
	int emitter_counter;

	TParticleSystem* ps;

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
	// Delay before start emitting
	float delay;
	// Check if the emitter must loop
	bool loop;
	// Random rotation
	bool random_rotation;

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
	TParticleEmitterGeneration(TParticleSystem* the_ps, TParticleEmitterShape the_shape, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius_or_box_size, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount, float the_delay, bool the_loop, bool the_random_rotation);
	// Cone
	TParticleEmitterGeneration(TParticleSystem* the_ps, TParticleEmitterShape the_shape, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, float the_angle_or_inner_radius, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount, float the_delay, bool the_loop, bool the_random_rotation);

	void fillInitial();

	virtual void update(float elapsed);

	void addParticle();

	std::string getXMLDefinition();

	void restart();
};

// --------------------------------- RENDERER ------------------------------

struct TParticleRenderer {
	VParticles* particles;
	char texture[64];
	TParticleRenderType render_type;
	bool additive;
	int n_anim_x;
	int n_anim_y;
	float stretch;
	int particle_animation_mode;
	int stretch_mode;

	TParticleRenderer(VParticles* the_particles, const char* the_texture, bool is_aditive, TParticleRenderType the_render_type, int the_n_anim_x, int the_n_anim_y, float the_stretch, int the_particle_animation_mode, int the_stretch_mode);

	void update(TParticle* particle, float elapsed);
	void render();

	std::string getXMLDefinition();
};

// --------------------------------- UPDATERS ------------------------------

struct TParticleUpdaterMovement {
	float speed;

	TParticleUpdaterMovement() { speed = 1; }

	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

struct TParticleUpdaterPhysx {
	TParticleSystem* ps;

	TParticleUpdaterPhysx(TParticleSystem* the_ps) : ps(the_ps) {}

	void update(TParticle* particle, float elapsed);
};

struct TParticleUpdaterRotation {

	float angular_speed;

	TParticleUpdaterRotation(float the_angular_speed) : angular_speed(the_angular_speed) {}

	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};
struct TParticleUpdaterGravity {
	float gravity;
	bool constant;

	TParticleUpdaterGravity(float the_gravity, bool is_constant) : gravity(the_gravity) , constant(is_constant) {}
	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

struct TParticleUpdaterLifeTime {
	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

struct TParticleUpdaterSize {
	float initial_size;
	float final_size;

	TParticleUpdaterSize(float the_initial_size, float the_final_size);

	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

struct TParticleUpdaterColor {
	XMFLOAT3 initial_color;
	XMFLOAT3 final_color;
	TParticleCurve curve;
	float curve_val;

	TParticleUpdaterColor();
	TParticleUpdaterColor(XMVECTOR the_initial_color, XMVECTOR the_final_color, TParticleCurve the_curve, float the_curve_val);

	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

struct TParticleUpdaterNoise {
	XMFLOAT3 min_noise;
	XMFLOAT3 max_noise;

	TParticleUpdaterNoise(XMVECTOR the_min_noise, XMVECTOR the_max_noise) { XMStoreFloat3(&min_noise, the_min_noise); XMStoreFloat3(&max_noise, the_max_noise); };
	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

struct TParticleSubemitter {
public:
	char death_emitter[64];
	
	TParticleSubemitter() { death_emitter[0] = 0; }
	void onParticleDeath(TParticle* particle);
	void update(TParticle* particle, float elapsed);
	std::string getXMLDefinition();
};

#endif