#include "mcv_platform.h"
#include "render_manager.h"
#include "render_utils.h"
#include "camera.h"
#include "handle/handle.h"
#include "components/comp_transform.h"
#include "components/comp_transform.h"
#include "components/comp_skeleton.h"
#include "components\comp_render.h"

CRenderManager render_manager;

bool CRenderManager::sort_by_material_then_mesh(const CRenderManager::TKey& k1, const CRenderManager::TKey& k2) {
	if (k1.material != k2.material)
		return k1.material->getName() < k2.material->getName();
	if (k1.mesh != k2.mesh)
		return k1.mesh < k2.mesh;
	return k1.mesh_id < k2.mesh_id;
}

void CRenderManager::addKey(const CMesh*      mesh
	, const CMaterial*  material
	, int  mesh_id
	, CHandle owner
	) {

	SET_ERROR_CONTEXT("Adding a render key", "")
	TKey k = { material, mesh, mesh_id, owner };

	// Pasar de comp_render a entity
	CEntity* e = owner.getOwner();
	k.transform = e->get< TCompTransform >();
	XASSERT(k.transform.isValid(), "Transform from entity %s not valid", e->getName());

	keys.push_back(k);
	sort_required = true;
}

void CRenderManager::renderAll(const CCamera* camera) {
	SET_ERROR_CONTEXT("Rendering entities", "")

	if (sort_required) {
		std::sort(keys.begin(), keys.end(), sort_by_material_then_mesh);
		sort_required = false;
	}

	const CRenderTechnique* curr_tech = nullptr;

	bool uploading_bones = false;

	bool is_first = true;
	auto prev_it = keys.begin();
	auto it = keys.begin();
	while (it != keys.end()) {
		CErrorContext ce2("Rendering key with material", it->material->getName().c_str());

		if ((it->owner.isTypeOf<TCompRender>() && ((TCompRender*)it->owner)->active)
			|| (it->owner.isTypeOf<TCompSkeleton>() && ((TCompSkeleton*)it->owner)->active)
			)
		{

			if (it->material != prev_it->material || is_first) {

				// La tech
				if (it->material->getTech() != curr_tech) {
					curr_tech = it->material->getTech();
					curr_tech->activate();
					activateCamera(camera, 1);
					activateWorldMatrix(0);

					uploading_bones = it->material->getTech()->usesBones();
				}

				// Activar shader y material de it
				it->material->activateTextures();
			}

			if (it->mesh != prev_it->mesh || is_first) {
				it->mesh->activate();
			}

			if (uploading_bones) {
				const TCompSkeleton* skel = it->owner;
				XASSERT(skel, "Invalid skeleton");
				skel->uploadBonesToGPU();
			}

			// Activar la world del obj
			TCompTransform* tmx = it->transform;
			XASSERT(tmx, "Invalid transform");
			setWorldMatrix(tmx->getWorld());

			// Pintar la mesh:submesh del it
			it->mesh->renderGroup(it->mesh_id);

			prev_it = it;
			is_first = false;
		}
		++it;		
	}


}

void CRenderManager::removeKeysFromOwner(CHandle owner) {
	VKeys keys_to_remove;

	for (auto& it : keys) {
		if (it.owner == owner)
			keys_to_remove.push_back(it);
	}

	for (auto& it : keys_to_remove) {
		auto it2 = std::remove(keys.begin(), keys.end(), it);
		keys.erase(it2, keys.end());
	}
}

void CRenderManager::destroyAllKeys() {
	keys.clear();
}