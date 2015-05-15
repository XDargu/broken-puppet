#include "mcv_platform.h"
#include "render_instances.h"
#include "vertex_declarations.h"
#include "render_utils.h"
using namespace DirectX;

float urandom() {
	return (float)rand() / (float)RAND_MAX;
}

float random(float vmin, float vmax) {
	return vmin + urandom() * (vmax - vmin);
}

bool CRenderInstances::create(size_t n, const CMesh* instanced) {

	instanced_mesh = instanced;
	global_time = 0.f;

	particles.resize(n);
	int idx = 0;
	for (auto& p : particles) {
		p.pos = XMFLOAT3(random(-5, 5), 0, random(-5, 5));
		//p.pos = XMFLOAT3(idx, 0, 0);
		p.nframe = random(0, 15);
		++idx;
	}

	// This mesh has not been registered in the mesh manager
	instances_data = new CMesh;
	bool is_ok = instances_data->create(n, &particles[0]
		, 0, nullptr        // No indices
		, CMesh::POINTS     // We are not using this
		, &vdcl_particle_data    // Type of vertex
		, true              // the buffer IS dynamic
		);


	return is_ok;
}

void CRenderInstances::destroy() {
	if (instances_data) {
		instances_data->destroy();
		delete instances_data;
	}
}


void CRenderInstances::render() {

	setWorldMatrix(XMMatrixIdentity());
	CTraceScoped t0("instances");
	render_techniques_manager.getByName("particles")->activate();
	texture_manager.getByName("fire")->activate(0);
	activateBlendConfig(BLEND_CFG_ADDITIVE_BY_SRC_ALPHA);
	activateZConfig(ZCFG_TEST_BUT_NO_WRITE);
	instanced_mesh->renderInstanced(*instances_data, particles.size());
	activateZConfig(ZCFG_DEFAULT);
	activateBlendConfig(BLEND_CFG_DEFAULT);
}

void CRenderInstances::update(float elapsed) {

	// Update particles using some cpu code
	global_time += elapsed;
	int idx = 1;
	for (auto& p : particles) {
		p.nframe += elapsed;
		//p.pos.y -= random(1.f, 3.f) * elapsed* 2;
		//if (p.pos.y < 0)
		//  p.pos.y += 50.f;
		++idx;
	}

	instances_data->updateFromCPU(&particles[0]);

}