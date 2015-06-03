#include "mcv_platform.h"
#include "particle_system_subtypes.h"
#include "components\comp_transform.h"
#include "physics_manager.h"

void TParticleEmitterGeneration::update(float elapsed) {
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
		rate_counter = 0;
		for (int i = 0; i < num_new_particles; ++i) {
			addParticle();
		}		
	}
}

void TParticleEmitterGeneration::addParticle() {
	if (particles->size() >= limit) { return; }

	TCompTransform* m_transform = h_transform;
	XMFLOAT3 pos;
	XMFLOAT3 direction;

	bool inside_condition;

	switch (shape)
	{
	case TParticleEmitterShape::SPHERE:
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		while (!inside_condition) {
			XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
			inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		}
		XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
		break;

	case TParticleEmitterShape::SEMISPHERE:
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		inside_condition &= m_transform->isInFront(XMLoadFloat3(&pos) + m_transform->position);

		while (!inside_condition) {
			XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
			inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
			inside_condition &= m_transform->isInFront(XMLoadFloat3(&pos) + m_transform->position);
		}
		XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
		break;

	case TParticleEmitterShape::CONE:
		XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
		inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		inside_condition &= m_transform->isInFov(XMLoadFloat3(&pos) + m_transform->position, angle);

		while (!inside_condition) {
			XMStoreFloat3(&pos, getRandomVector3(-radius, radius));
			inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
			inside_condition &= m_transform->isInFov(XMLoadFloat3(&pos) + m_transform->position, angle);
		}
		XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
		break;

	case TParticleEmitterShape::RING:
		XMStoreFloat3(&pos, m_transform->getUp() * getRandomNumber(-radius, radius) + m_transform->getLeft() * getRandomNumber(-radius, radius));
		inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
		inside_condition &= ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) > (inner_radius*inner_radius));

		while (!inside_condition) {
			XMStoreFloat3(&pos, m_transform->getUp() * getRandomNumber(-radius, radius) + m_transform->getLeft() * getRandomNumber(-radius, radius));
			inside_condition = ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) < (radius*radius));
			inside_condition &= ((pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) > (inner_radius*inner_radius));
		}
		XMStoreFloat3(&pos, m_transform->position + XMLoadFloat3(&pos));
		break;

	case TParticleEmitterShape::BOX:
		XMStoreFloat3(&pos, m_transform->position + getRandomVector3(-box_size, box_size));
		break;
	}	

	if (shape == TParticleEmitterShape::BOX) {
		XMStoreFloat3(&direction, m_transform->getFront());
	}
	else
	{
		XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&pos) - m_transform->position));
	}

	float life_time = getRandomNumber(min_life_time, max_life_time);

	TParticle n_particle = TParticle(
		pos
		, direction
		, 0
		, life_time
		, XMVectorSet(1, 1, 1, 1)
		, 1
		, (int)particles->size()
		);
	//particles->erase(particles->begin());
	particles->push_back(n_particle);
}

// Sphere / Semisphere / Box
TParticleEmitterGeneration::TParticleEmitterGeneration(VParticles* the_particles, TParticleEmitterShape the_shape, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius_or_box_size, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount) {
	shape = the_shape;
	radius = the_radius_or_box_size;
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

	inner_radius = 0.5;
	box_size = the_radius_or_box_size;
	angle = deg2rad(30);

	fillInitial();
}
// Cone / Ring
TParticleEmitterGeneration::TParticleEmitterGeneration(VParticles* the_particles, TParticleEmitterShape the_shape, CHandle the_transform, float the_rate, float the_min_life_time, float the_max_life_time, float the_radius, float the_angle_or_inner_radius, bool the_fill_initial, int the_limit, float the_burst_time, int the_burst_amount) {
	shape = the_shape;
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

	inner_radius = the_angle_or_inner_radius;
	box_size = 1;
	angle = the_angle_or_inner_radius;

	fillInitial();
}

void TParticleEmitterGeneration::fillInitial() {
	if (fill_initial) {
		// Make the initial particles
		for (int i = 0; i < limit; ++i) {
			addParticle();
		}
	}
}

std::string TParticleEmitterGeneration::getXMLDefinition() {
	std::string def = "";
	def += "<emitter ";

	// Sepecific atts
	def += "type=\"";
	switch (shape)
	{
		case SPHERE: 
			def += "sphere"; 
			def += "\" ";

			def += "radius=\"";
			def += std::to_string(radius) + "\" ";
			break;
		case SEMISPHERE: 
			def += "semisphere"; 
			def += "\" ";

			def += "radius=\"";
			def += std::to_string(radius) + "\" ";
			break;
		case CONE: 
			def += "cone"; 
			def += "\" ";

			def += "radius=\"";
			def += std::to_string(radius) + "\" ";
			
			def += "angle=\"";
			def += std::to_string(rad2deg(angle)) + "\" ";
			break;
		case BOX: 
			def += "box"; 
			def += "\" ";

			def += "size=\"";
			def += std::to_string(box_size) + "\" ";
			break;
		case RING: 
			def += "ring"; 
			def += "\" ";

			def += "innerRadius=\"";
			def += std::to_string(inner_radius) + "\" ";

			def += "outerRadius=\"";
			def += std::to_string(radius) + "\" ";
			break;
	}

	// General atts
	def += "rate=\"";
	def += std::to_string(rate) + "\" ";

	def += "minLifeTime=\"";
	def += std::to_string(min_life_time) + "\" ";

	def += "maxLifeTime=\"";
	def += std::to_string(max_life_time) + "\" ";

	def += "fillInitial=\"";
	def += std::to_string(fill_initial) + "\" ";

	def += "limit=\"";
	def += std::to_string(limit) + "\" ";

	def += "burstTime=\"";
	def += std::to_string(burst_time) + "\" ";

	def += "burstAmount=\"";
	def += std::to_string(burst_amount) + "\" ";

	def += "/>";

	return def;
}

TParticleRenderer::TParticleRenderer(VParticles* the_particles, const char* the_texture, bool is_aditive) {
	particles = the_particles;
	strcpy(texture, the_texture);
	additive = is_aditive;

}

void TParticleRenderer::update(TParticle* particle, float elapsed) {
	particle->age += elapsed;
}

std::string TParticleRenderer::getXMLDefinition() {
	std::string def = "";

	def += "<renderer ";

	def += "texture=\"";
	def += std::string(texture) + "\" ";

	def += "aditive=\"";
	def += (additive ? "true" : "false");
	def += "\" ";

	def += "/>";

	return def;
}

void TParticleUpdaterMovement::update(TParticle* particle, float elapsed) {
	XMStoreFloat3(&particle->position, XMLoadFloat3(&particle->position) + XMLoadFloat3(&particle->direction) * speed * elapsed);
}

std::string TParticleUpdaterMovement::getXMLDefinition() {
	std::string def = "";

	def += "<updater type=\"movement\" ";

	def += "speed=\"";
	def += std::to_string(speed) + "\" ";

	def += "/>";

	return def;
}

void TParticleUpdaterGravity::update(TParticle* particle, float elapsed) {
	XMStoreFloat3(&particle->speed, XMLoadFloat3(&particle->speed) + Physics.PxVec3ToXMVECTOR(Physics.gScene->getGravity()) * elapsed * gravity);
}

std::string TParticleUpdaterGravity::getXMLDefinition() {
	std::string def = "";

	def += "<updater type=\"gravity\" ";

	def += "gravity=\"";
	def += std::to_string(gravity) + "\" ";

	def += "/>";

	return def;
}

void TParticleUpdaterLifeTime::update(TParticle* particle, float elapsed) {
	particle->age += elapsed;
}

std::string TParticleUpdaterLifeTime::getXMLDefinition() {
	std::string def = "";

	def += "<updater type=\"lifeTime\" ";

	def += "/>";

	return def;
}

TParticleUpdaterSize::TParticleUpdaterSize(float the_initial_size, float the_final_size) {
	initial_size = the_initial_size;
	final_size = the_final_size;
}

void TParticleUpdaterSize::update(TParticle* particle, float elapsed) {
	particle->size = lerp(initial_size, final_size, particle->age / particle->lifespan);
}

std::string TParticleUpdaterSize::getXMLDefinition() {
	std::string def = "";

	def += "<updater type=\"size\" ";

	def += "initialSize=\"";
	def += std::to_string(initial_size) + "\" ";

	def += "finalSize=\"";
	def += std::to_string(final_size) + "\" ";

	def += "/>";

	return def;
}

TParticleUpdaterColor::TParticleUpdaterColor() {
	initial_color = XMVectorSet(1, 1, 1, 1);
	final_color = XMVectorSet(1, 1, 1, 1);
}

TParticleUpdaterColor::TParticleUpdaterColor(XMVECTOR the_initial_color, XMVECTOR the_final_color) {
	initial_color = the_initial_color;
	final_color = the_final_color;
}

void TParticleUpdaterColor::update(TParticle* particle, float elapsed) {
	XMStoreFloat3(&particle->color, XMVectorLerp(initial_color, final_color, particle->age / particle->lifespan));
}

std::string TParticleUpdaterColor::getXMLDefinition() {
	std::string def = "";

	def += "<updater type=\"color\" ";

	def += "initialColor=\"";
	def += V3ToString(initial_color) + "\" ";

	def += "finalColor=\"";
	def += V3ToString(final_color) + "\" ";

	def += "/>";

	return def;
}

void TParticleUpdaterNoise::update(TParticle* particle, float elapsed) {	
	XMStoreFloat3(&particle->speed, XMLoadFloat3(&particle->speed) + getRandomVector3(min_noise, max_noise) * elapsed);
}

std::string TParticleUpdaterNoise::getXMLDefinition() {
	std::string def = "";

	def += "<updater type=\"noise\" ";

	def += "minNoise=\"";
	def += V3ToString(XMLoadFloat3(&min_noise)) + "\" ";

	def += "maxNoise=\"";
	def += V3ToString(XMLoadFloat3(&max_noise)) + "\" ";

	def += "/>";

	return def;
}