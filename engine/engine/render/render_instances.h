#ifndef INC_RENDER_INSTANCES_H_
#define INC_RENDER_INSTANCES_H_

#include "mesh.h"

class CRenderInstances {

	const CMesh* instanced_mesh;      // The single tree, particle
	CMesh* instances_data;      // The positions of each instance
	float  global_time;

	struct TParticleData {
		XMFLOAT3 pos;
		float    nframe;
		//    float    scale;
		//    XMFLOAT4 color;
	};
	std::vector<TParticleData> particles;

public:

	CRenderInstances()
		: instanced_mesh(nullptr)
		, instances_data(nullptr)
	{}

	bool create(size_t n, const CMesh* instanced);
	void destroy();
	void render();
	void update(float elapsed);
};

#endif
