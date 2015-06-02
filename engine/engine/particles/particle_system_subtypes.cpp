#include "mcv_platform.h"
#include "particle_system_subtypes.h"
#include "components\comp_transform.h"
#include "physics_manager.h"

TParticleEmitterGenerationSphere::TParticleEmitterGenerationSphere(
	VParticles* the_particles 
	, CHandle the_transform
	, float the_rate
	, float the_min_life_time, float the_max_life_time
	, float the_radius
	, bool the_fill_initial
	, int the_limit
	, float the_burst_time
	, int the_burst_amount
	) {

	radius = the_radius;
	h_transform = the_transform;
	min_life_time = the_min_life_time;
	max_life_time = the_max_life_time;
	rate = the_rate;
	rate_counter = 0;
	fill_initial = the_fill_initial;
	limit = the_limit;
	particles = the_particles;
	burst_time = the_burst_time;
	burst_amount = the_burst_amount;
	burst_counter = 0;

	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
	}
}
void TParticleEmitterGenerationSphere::update(float elapsed) {
	rate_counter += elapsed;
	burst_counter += elapsed;

	if (burst_time > 0 && burst_counter > burst_time) {
		int amount = min(burst_amount, (limit - particles->size()));
		burst_counter = 0;
		for (int i = 0; i < amount; ++i) {
			addParticle();
		}
	}

	// If we have to make a new particle
	if (burst_time == 0 && rate != 0 && rate_counter > rate && particles->size() < limit) {
		int num_new_particles = max(elapsed / rate, 1);
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}
	}
}

void TParticleEmitterGenerationSphere::addParticle() {
	if (particles->size() >= limit) { return; }
	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
	bool insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
	while (!insideSphere) {
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
	}
	XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
	float life_time = getRandomNumber(min_life_time, max_life_time);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&pos) - m_transform->position));
	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		);
	//particles->erase(particles->begin());
	particles->push_back(n_particle);
	rate_counter = 0;
}

//----------------------------------------------------------------------------------------------

TParticleEmitterGenerationPhysX::TParticleEmitterGenerationPhysX(
	VParticles* the_particles
	, CHandle the_transform
	, float the_rate
	, float the_min_life_time
	, float the_max_life_time
	, float the_size
	, bool the_fill_initial
	, int the_limit
	, float the_burst_time
	, int the_burst_amount
	) {
	size = the_size;
	h_transform = the_transform;
	min_life_time = the_min_life_time;
	max_life_time = the_max_life_time;
	rate = the_rate;
	rate_counter = 0;
	fill_initial = the_fill_initial;
	limit = the_limit;
	particles = the_particles;
	burst_time = the_burst_time;
	burst_amount = the_burst_amount;
	burst_counter = 0;
	index = 0;

	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
		//physXParticles.createParticles(1);
	}
}
void TParticleEmitterGenerationPhysX::update(float elapsed) {
	rate_counter += elapsed;
	burst_counter += elapsed;

	if (burst_time > 0 && burst_counter > burst_time) {
		int amount = min(burst_amount, (limit - particles->size()));
		burst_counter = 0;
		for (int i = 0; i < amount; ++i) {
			addParticle();
		}
	}

	// If we have to make a new particle
	if (burst_time == 0 && rate != 0 && rate_counter > rate && particles->size() < limit){
		int num_new_particles = max(elapsed / rate, 1);
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}
	}
}

void TParticleEmitterGenerationPhysX::addParticle() {
	if (particles->size() >= limit) { return; }
	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, m_transform->position + getRandomVector3(-size, size));
	float life_time = getRandomNumber(min_life_time, max_life_time);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, m_transform->getFront());
	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		);
	//particles->erase(particles->begin());

	//Añadir particulas!!!
	index++;
	PxU32 indexToAdd[1] = { index };
	PxVec3 positionToAdd[1] = { PxVec3(pos.x, pos.y, pos.z) };
	PxVec3 velocityToAdd[1] = { PxVec3(direction.x, direction.y, direction.z) };
	physXParticles.addParticles(1, indexToAdd, positionToAdd, velocityToAdd);
	//Necesitamos de alguna manera los indices para que los vectores esten alineados

	particles->push_back(n_particle);
	rate_counter = 0;
}


//----------------------------------------------------------------------------------------------


TParticleEmitterGenerationSemiSphere::TParticleEmitterGenerationSemiSphere(
	VParticles* the_particles
		, CHandle the_transform
		, float the_rate
		, float the_min_life_time
		, float the_max_life_time
		, float the_radius
		, bool the_fill_initial
		, int the_limit
		, float the_burst_time
		, int the_burst_amount
	) {
	radius = the_radius;
	h_transform = the_transform;
	min_life_time = the_min_life_time;
	max_life_time = the_max_life_time;
	rate = the_rate;
	rate_counter = 0;
	fill_initial = the_fill_initial;
	limit = the_limit;
	particles = the_particles;
	burst_time = the_burst_time;
	burst_amount = the_burst_amount;
	burst_counter = 0;

	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
	}
}
void TParticleEmitterGenerationSemiSphere::update(float elapsed) {
	rate_counter += elapsed;
	burst_counter += elapsed;

	if (burst_time > 0 && burst_counter > burst_time) {
		int amount = min(burst_amount, (limit - particles->size()));
		burst_counter = 0;
		for (int i = 0; i < amount; ++i) {
			addParticle();
		}
	}

	// If we have to make a new particle
	if (burst_time == 0 && rate != 0 && rate_counter > rate && particles->size() < limit){
		int num_new_particles = max(elapsed / rate, 1);
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}
	}
}

void TParticleEmitterGenerationSemiSphere::addParticle() {
	if (particles->size() >= limit) { return; }
	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
	bool insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
	insideSphere &= m_transform->isInFront(XMLoadFloat3(&pos) + m_transform->position);

	while (!insideSphere) {
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));	
		insideSphere &= m_transform->isInFront(XMLoadFloat3(&pos) + m_transform->position);
	}
	XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
	float life_time = getRandomNumber(min_life_time, max_life_time);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&pos) - m_transform->position));
	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		);
	//particles->erase(particles->begin());
	particles->push_back(n_particle);
	rate_counter = 0;
}

TParticleEmitterGenerationCone::TParticleEmitterGenerationCone(
		VParticles* the_particles
		, CHandle the_transform
		, float the_rate
		, float the_min_life_time
		, float the_max_life_time
		, float the_radius
		, float the_angle
		, bool the_fill_initial
		, int the_limit
		, float the_burst_time
		, int the_burst_amount
	) {
	radius = the_radius;
	angle = the_angle;
	h_transform = the_transform;
	min_life_time = the_min_life_time;
	max_life_time = the_max_life_time;
	rate = the_rate;
	rate_counter = 0;
	fill_initial = the_fill_initial;
	limit = the_limit;
	particles = the_particles;
	burst_time = the_burst_time;
	burst_amount = the_burst_amount;
	burst_counter = 0;

	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
	}
}
void TParticleEmitterGenerationCone::update(float elapsed) {
	rate_counter += elapsed;
	burst_counter += elapsed;

	if (burst_time > 0 && burst_counter > burst_time) {
		int amount = min(burst_amount, (limit - particles->size()));
		burst_counter = 0;
		for (int i = 0; i < amount; ++i) {
			addParticle();
		}
	}

	// If we have to make a new particle
	if (burst_time == 0 && rate != 0 && rate_counter > rate && particles->size() < limit){
		int num_new_particles = max(elapsed / rate, 1);
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}
	}
}

void TParticleEmitterGenerationCone::addParticle() {
	if (particles->size() >= limit) { return; }
	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
	bool insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
	insideSphere &= m_transform->isInFov(XMLoadFloat3(&pos) + m_transform->position, angle);

	while (!insideSphere) {
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		insideSphere = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		insideSphere &= m_transform->isInFov(XMLoadFloat3(&pos) + m_transform->position, angle);
	}
	XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
	float life_time = getRandomNumber(min_life_time, max_life_time);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&pos) - m_transform->position));
	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		);
	//particles->erase(particles->begin());
	particles->push_back(n_particle);
	rate_counter = 0;
}

TParticleEmitterGenerationRing::TParticleEmitterGenerationRing(
	VParticles* the_particles
	, CHandle the_transform
	, float the_rate
	, float the_min_life_time
	, float the_max_life_time
	, float the_inner_radius
	, float the_outer_radius
	, bool the_fill_initial
	, int the_limit
	, float the_burst_time
	, int the_burst_amount
	) {
	inner_radius = the_inner_radius;
	outer_radius = the_outer_radius;
	h_transform = the_transform;
	min_life_time = the_min_life_time;
	max_life_time = the_max_life_time;
	rate = the_rate;
	rate_counter = 0;
	fill_initial = the_fill_initial;
	limit = the_limit;
	particles = the_particles;
	burst_time = the_burst_time;
	burst_amount = the_burst_amount;
	burst_counter = 0;

	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
	}
}
void TParticleEmitterGenerationRing::update(float elapsed) {
	rate_counter += elapsed;
	burst_counter += elapsed;

	if (burst_time > 0 && burst_counter > burst_time) {
		int amount = min(burst_amount, (limit - particles->size()));
		burst_counter = 0;
		for (int i = 0; i < amount; ++i) {
			addParticle();
		}
	}

	// If we have to make a new particle
	if (burst_time == 0 && rate != 0 && rate_counter > rate && particles->size() < limit){
		int num_new_particles = max(elapsed / rate, 1);
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}
	}
}

void TParticleEmitterGenerationRing::addParticle() {
	if (particles->size() >= limit) { return; }
	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, getRandomVector3(-outer_radius, 0, -outer_radius, outer_radius, 0, outer_radius));
	bool insideRing = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (outer_radius*outer_radius));
	insideRing &= ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) > (inner_radius*inner_radius));

	while (!insideRing) {
		XMStoreFloat3(&pos, getRandomVector3(-outer_radius, 0, -outer_radius, outer_radius, 0, outer_radius));
		insideRing = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (outer_radius*outer_radius));
		insideRing &= ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) > (inner_radius*inner_radius));
	}
	XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
	float life_time = getRandomNumber(min_life_time, max_life_time);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&pos) - m_transform->position));
	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		);
	//particles->erase(particles->begin());
	particles->push_back(n_particle);
	rate_counter = 0;
}


TParticleEmitterGenerationBox::TParticleEmitterGenerationBox(
		VParticles* the_particles
		, CHandle the_transform
		, float the_rate
		, float the_min_life_time
		, float the_max_life_time
		, float the_size
		, bool the_fill_initial
		, int the_limit
		, float the_burst_time
		, int the_burst_amount
	) {
	size = the_size;
	h_transform = the_transform;
	min_life_time = the_min_life_time;
	max_life_time = the_max_life_time;
	rate = the_rate;
	rate_counter = 0;
	fill_initial = the_fill_initial;
	limit = the_limit;
	particles = the_particles;
	burst_time = the_burst_time;
	burst_amount = the_burst_amount;
	burst_counter = 0;

	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
	}
}
void TParticleEmitterGenerationBox::update(float elapsed) {
	rate_counter += elapsed;
	burst_counter += elapsed;

	if (burst_time > 0 && burst_counter > burst_time) {
		int amount = min(burst_amount, (limit - particles->size()));
		burst_counter = 0;
		for (int i = 0; i < amount; ++i) {
			addParticle();
		}
	}

	// If we have to make a new particle
	if (burst_time == 0 && rate != 0 && rate_counter > rate && particles->size() < limit){
		int num_new_particles = max(elapsed / rate, 1);
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}
	}
}

void TParticleEmitterGenerationBox::addParticle() {
	if (particles->size() >= limit) { return; }
	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, m_transform->position + getRandomVector3(-size, size));
	float life_time = getRandomNumber(min_life_time, max_life_time);
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, m_transform->getFront());
	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		);
	//particles->erase(particles->begin());
	particles->push_back(n_particle);
	rate_counter = 0;
}


VParticles* particles;
char texture[64];
bool additive;

TParticleRenderer::TParticleRenderer(VParticles* the_particles, const char* the_texture, bool is_aditive) {
	particles = the_particles;
	strcpy(texture, the_texture);
	additive = is_aditive;

}

void TParticleRenderer::update(TParticle* particle, float elapsed) {
	particle->age += elapsed;
}

void TParticleUpdaterMovement::update(TParticle* particle, float elapsed) {
	XMStoreFloat3(&particle->position, XMLoadFloat3(&particle->position) + XMLoadFloat3(&particle->direction) * speed * elapsed);
}

void TParticleUpdaterGravity::update(TParticle* particle, float elapsed) {
	XMStoreFloat3(&particle->speed, XMLoadFloat3(&particle->speed) + Physics.PxVec3ToXMVECTOR(Physics.gScene->getGravity()) * elapsed * gravity);
}

void TParticleUpdaterLifeTime::update(TParticle* particle, float elapsed) {
	particle->age += elapsed;
}

TParticleUpdaterSize::TParticleUpdaterSize(float the_initial_size, float the_final_size) {
	initial_size = the_initial_size;
	final_size = the_final_size;
}

void TParticleUpdaterSize::update(TParticle* particle, float elapsed) {
	particle->size = lerp(initial_size, final_size, particle->age / particle->lifespan);
}

void TParticleUpdaterColor::update(TParticle* particle, float elapsed) {
	XMStoreFloat3(&particle->color, XMVectorLerp(initial_color, final_color, particle->age / particle->lifespan));
}

void TParticleUpdaterNoise::update(TParticle* particle, float elapsed) {	
	XMStoreFloat3(&particle->speed, XMLoadFloat3(&particle->speed) + getRandomVector3(min_noise, max_noise) * elapsed);
}