#include "mcv_platform.h"
#include "render_manager.h"
#include "render_utils.h"
#include "camera.h"
#include "transform.h"
#include "aabb.h"
#include "font\font.h"
#include "handle/handle.h"
#include "components/comp_transform.h"
#include "components/comp_transform.h"
#include "components/comp_skeleton.h"
#include "components\comp_render.h"
#include "components\comp_aabb.h"

CRenderManager render_manager;

bool CRenderManager::sort_by_material_then_mesh(const CRenderManager::TKey& k1, const CRenderManager::TKey& k2) {
	if (k1.material != k2.material) {
		// sort, first the solid, then the transparent
		if (k1.material->isSolid() != k2.material->isSolid())
			return k1.material->isSolid();
		return k1.material->getName() < k2.material->getName();
	}
	if (k1.mesh != k2.mesh)
		return k1.mesh < k2.mesh;
	return k1.mesh_id < k2.mesh_id;
}

void CRenderManager::addKey(const CMesh*      mesh
	, const CMaterial*  material
	, int  mesh_id
	, CHandle owner
	, bool* active
	) {

	SET_ERROR_CONTEXT("Adding a render key", "")
	TKey k = { material, mesh, mesh_id, owner };

	// Pasar de comp_render a entity
	CEntity* e = owner.getOwner();
	k.transform = e->get< TCompTransform >();
	XASSERT(k.transform.isValid(), "Transform from entity %s not valid", e->getName());

	k.active = active;

	keys.push_back(k);
	sort_required = true;

	// 
	if (material->castsShadows()) {
		TShadowCasterKey ck = { material, mesh, owner };
		ck.transform = k.transform;
		shadow_casters_keys.push_back(ck);
	}
}

void CRenderManager::renderAll(const CCamera* camera, bool solids) {
	renderAll(camera, &TTransform(), solids);
}

void CRenderManager::renderAll(const CCamera* camera, TTransform* camera_transform, bool solids) {
	SET_ERROR_CONTEXT("Rendering entities", "")

	if (sort_required) {
		std::sort(keys.begin(), keys.end(), sort_by_material_then_mesh);
		sort_required = false;
	}
	
	planes.create(camera->getViewProjection());

	const CRenderTechnique* curr_tech = nullptr;
	activateCamera(*camera, 1);

	bool uploading_bones = false;
	
	auto first_transparent = std::lower_bound(keys.begin(), keys.end(), false
		, [](const CRenderManager::TKey&k1, bool is_solid) {
		return k1.material->isSolid() != is_solid;
	}
	);

	auto first_it = solids ? keys.begin() : first_transparent;
	auto last_it = solids ? first_transparent : keys.end();

	bool is_first = true;
	auto prev_it = keys.begin();
	auto it = keys.begin();
	
	bool culling = true;
	int render_count = 0;

	while (it != keys.end()) {
		CErrorContext ce2("Rendering key with material", it->material->getName().c_str());
		
		TCompTransform* tmx = it->transform;
		XASSERT(tmx, "Invalid transform");

		
		TCompAABB* m_aabb = ((CEntity*)CHandle(tmx).getOwner())->get<TCompAABB>();
		XASSERT(m_aabb, "Invalid AABB");
		culling = planes.isVisible(m_aabb);

		culling &= it->material->isSolid() == solids;
				
		//culling = true;
		if (*it->active && culling)
		{
			render_count++;
			if (it->material != prev_it->material || is_first) {
				
				// La tech
				if (it->material->getTech() != curr_tech) {
					curr_tech = it->material->getTech();
					curr_tech->activate();
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
			std::string name = ((CEntity*)it->transform.getOwner())->getName();
			// Activar la world del obj
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
	VShadowCasterKeys shadow_keys_to_remove;

	for (auto& it : keys) {
		if (it.owner == owner)
			keys_to_remove.push_back(it);
	}

	for (auto& it : shadow_casters_keys) {
		if (it.owner == owner)
			shadow_keys_to_remove.push_back(it);
	}

	for (auto& it : keys_to_remove) {
		auto it2 = std::remove(keys.begin(), keys.end(), it);
		keys.erase(it2, keys.end());
	}

	for (auto& it : shadow_keys_to_remove) {
		auto it2 = std::remove(shadow_casters_keys.begin(), shadow_casters_keys.end(), it);
		shadow_casters_keys.erase(it2, shadow_casters_keys.end());
	}
}

void CRenderManager::destroyAllKeys() {
	keys.clear();
	shadow_casters_keys.clear();
}

// ---------------------------------------------------------------
void CRenderManager::renderShadowsCasters() {

	bool uploading_bones = false;
	for (auto k : shadow_casters_keys) {
		
		if (!k.material->isSolid())
			continue;

		// Activar la world del obj
		TCompTransform* tmx = k.transform;
		assert(tmx);
		setWorldMatrix(tmx->getWorld());

		uploading_bones = k.material->getTech()->usesBones();

		if (uploading_bones) {
			render_techniques_manager.getByName("gen_shadows_skel")->activate();

			const TCompSkeleton* skel = k.owner;
			XASSERT(skel, "Invalid skeleton");
			skel->uploadBonesToGPU();
		}
		else {
			render_techniques_manager.getByName("gen_shadows")->activate();
		}


		// Pintar la mesh:submesh del it
		k.mesh->activateAndRender();
	}

}