#ifndef INC_COMP_PARTICLE_SYSTEM_H_
#define INC_COMP_PARTICLE_SYSTEM_H_

#include "base_component.h"
#include "render\render_instances.h"

struct TParticle {
	XMFLOAT3 position;
	float age;
	float lifespan;
	float size;
	//XMVECTOR color;
	

	TParticle(XMFLOAT3 the_position, float the_age, float the_lifespan, XMVECTOR the_color, float the_size) {
		position = the_position;
		age = the_age;
		lifespan = the_lifespan;
		//color = the_color;
		size = the_size;
	}

	TParticle() {
		position = XMFLOAT3(0, 0, -1);
		age = 0;
		lifespan = 0;
		//color = XMVectorSet(0, 0, 0, -1);
		size = -1;
	}

	void update(float elapsed) {
		age += elapsed;
	}
};

typedef std::deque<TParticle> VParticles;

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
	float life_time;

	virtual void update(float elapsed) = 0;
};

struct TParticleEmitterGenerationSphere : TParticleEmitterGeneration {

	float radius;

	TParticleEmitterGenerationSphere(XMVECTOR the_position, float the_rate, float the_life_time, float the_radius) {
		radius = the_radius;
		position = the_position;
		life_time = the_life_time;
		rate = the_rate;
		rate_counter = 0;
	}
	void update(float elapsed) {
		rate_counter += elapsed;
		
		// If we have to make a new particle
		if (rate_counter > rate){
			XMFLOAT3 pos;
			XMStoreFloat3(&pos, position + getRandomVector3(-radius, radius));
			TParticle n_particle = TParticle(
				pos
				, 0
				, life_time
				, XMVectorSet(1, 1, 1, 1)
				, 1
				);
			particles->pop_front();
			particles->push_back(n_particle);
			rate_counter = 0;
		}
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


struct TCompParticleSystem : TBaseComponent {

	VParticles particles;
	TParticleUpdaterLifeTime* updater_lifetime;
	TParticleUpdaterSize* updater_size;
	TParticleUpdaterColor* updater_color;
	TParticleUpdaterMovement* updater_movement;

	TParticleEmitterGeneration* emitter_generation;

	CHandle h_transform;

	const CMesh* instanced_mesh;      // The single tree, particle
	CMesh* instances_data;      // The positions of each instance

public:

	TCompParticleSystem()
		: updater_lifetime(nullptr)
		, updater_size(nullptr)
		, updater_color(nullptr)
		, updater_movement(nullptr)
		
		, emitter_generation(nullptr)

		, h_transform(CHandle())

		, instanced_mesh(nullptr)
		, instances_data(nullptr)
	{};

	~TCompParticleSystem() {
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
