#ifndef INC_PARTICLE_H_
#define INC_PARTICLE_H_

#include "utils.h"
#include "handle\handle.h"

struct TParticle {
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT3 speed;
	float age;
	float lifespan;
	float size;
	XMFLOAT3 color;
	int index;
	float rotation;

	TParticle(XMFLOAT3 the_position, XMFLOAT3 the_direction, float the_age, float the_lifespan, XMVECTOR the_color, float the_size, int the_index);

	TParticle();

	void update(float elapsed);
};

#endif