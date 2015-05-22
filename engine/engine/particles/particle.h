#ifndef INC_PARTICLE_H_
#define INC_PARTICLE_H_

#include "utils.h"
#include "handle\handle.h"

struct TParticle {
	XMFLOAT3 position;
	float age;
	float lifespan;
	float size;
	//XMVECTOR color;


	TParticle(XMFLOAT3 the_position, float the_age, float the_lifespan, XMVECTOR the_color, float the_size);

	TParticle();

	void update(float elapsed);
};

#endif