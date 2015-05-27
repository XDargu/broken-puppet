#include "mcv_platform.h"
#include "particle.h"

TParticle::TParticle(XMFLOAT3 the_position, float the_age, float the_lifespan, XMVECTOR the_color, float the_size) {
	position = the_position;
	age = the_age;
	lifespan = the_lifespan;
	XMStoreFloat3(&color, the_color);
	size = the_size;
}

TParticle::TParticle() {
	position = XMFLOAT3(0, 0, -1);
	age = 0;
	lifespan = 0;
	color = XMFLOAT3(0, 0, 0);
	size = -1;
}

void TParticle::update(float elapsed) {
	age += elapsed;
}
