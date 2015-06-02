#include "mcv_platform.h"
#include "particle.h"

TParticle::TParticle(XMFLOAT3 the_position, XMFLOAT3 the_direction, float the_age, float the_lifespan, XMVECTOR the_color, float the_size, int the_index) {
	position = the_position;
	direction = the_direction;
	speed = XMFLOAT3(0, 0, 0);
	age = the_age;
	lifespan = the_lifespan;
	XMStoreFloat3(&color, the_color);
	size = the_size;
	index = the_index;
}

TParticle::TParticle() {
	position = XMFLOAT3(0, 0, -1);
	direction = XMFLOAT3(0, 0, -1);
	speed = XMFLOAT3(0, 0, 0);
	age = 0;
	lifespan = 0;
	color = XMFLOAT3(0, 0, 0);
	size = -1;
	index = -1;
}

void TParticle::update(float elapsed) {
	age += elapsed;
}
