#include "mcv_platform.h"
#include "render_manager.h"
#include "render_utils.h"
#include "camera.h"
#include "transform.h"
#include "aabb.h"
#include "font\font.h"
#include "handle/handle.h"
#include "components/comp_transform.h"
#include "components/comp_name.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"
#include "components/comp_skeleton.h"
#include "components\comp_render.h"
#include "components\comp_aabb.h"
#include "entity_manager.h"

CRenderManager render_manager;

void CRenderManager::init() {
	technique_gen_shadows = render_techniques_manager.getByName("gen_shadows");
	technique_gen_shadows_skel = render_techniques_manager.getByName("gen_shadows_skel");
}

bool CRenderManager::sort_by_material_then_mesh(const CRenderManager::TKey& k1, const CRenderManager::TKey& k2) {
	// Sort first by blend mode
	if (k1.material->getBlendMode() != k2.material->getBlendMode()) {
		return k1.material->getBlendMode() < k2.material->getBlendMode();
	}
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
	, XMVECTOR color
	) {

	SET_ERROR_CONTEXT("Adding a render key", "")
		TKey k = { material, mesh, mesh_id, owner };

	// Pasar de comp_render a entity
	CEntity* e = owner.getOwner();
	TCompName* c_name = e->get< TCompName >();

	if (c_name) {
		std::string lightmap = "/lightmaps/" + CApp::get().current_scene_name + "/" + std::string(c_name->name) + "_lighting";
		const CTexture* light = texture_manager.getByName(lightmap.c_str());
		if (light != nullptr)
			k.lightmap = light;
		else
			k.lightmap = texture_manager.getByName("black");
	}

	k.transform = e->get< TCompTransform >();
	k.aabb = e->get< TCompAABB >();
	k.color = color;
	XASSERT(k.transform.isValid(), "Transform from entity %s not valid", e->getName());
	//XASSERT(k.aabb.isValid(), "AABB from entity %s not valid", e->getName());

	k.active = active;

	keys.push_back(k);
	sort_required = true;

	// 
	if (material->castsShadows()) {
		TShadowCasterKey ck = { material, mesh, owner };
		ck.aabb = k.aabb;
		ck.transform = k.transform;
		shadow_casters_keys.push_back(ck);
	}
}

void CRenderManager::renderAll(const CCamera* camera, bool solids, bool double_sided) {
	renderAll(camera, &TTransform(), solids, double_sided);
}

void CRenderManager::renderAll(const CCamera* camera, TTransform* camera_transform, bool solids, bool double_sided) {
	SET_ERROR_CONTEXT("Rendering entities", "")

	if (sort_required) {
		std::sort(keys.begin(), keys.end(), sort_by_material_then_mesh);
		sort_required = false;
	}	

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

	// ----------------- TEST OCCLUSION CULLING ----------------------

	/*CEntity* occlusion_plane = CEntityManager::get().getByName("occlusion_plane1");
	//CEntity* playerCamera = CEntityManager::get().getByName("esc1_arco_pilar_bottom_a_166.0");
	CEntity* playerCamera = CEntityManager::get().getByName("PlayerCamera");
	TCompTransform* p_t = occlusion_plane->get<TCompTransform>();
	TCompTransform* c_t = playerCamera->get<TCompTransform>();
	
	frustrum_occlusion.create(*p_t, 8, 8);
	frustrum_occlusion.update(camera->getPosition());*/

	for (auto& o_plane : occlusionPlanes) {
		o_plane.update(camera->getPosition());
	}

	// ---------------------------------------------------------------
	
	TCompTransform* tmx = nullptr;
	TCompRender* render = nullptr;

	while (it != keys.end()) {
		if (!it->material->isDoubleSided() && double_sided) {
			it++;
			continue; 			
		}

		CErrorContext ce2("Rendering key with material", it->material->getName().c_str());
		
		tmx = it->transform;
		if (it->owner.isTypeOf<TCompRender>()) {
			render = it->owner;
		}
		XASSERT(tmx, "Invalid transform");

		if (!it->aabb.isValid()) {
			it->aabb = ((CEntity*)it->transform.getOwner())->get<TCompAABB>();
		}
		
		TCompAABB* m_aabb = it->aabb;
		//TCompName* m_name = ((CEntity*)it->transform.getOwner())->get<TCompName>();
		XASSERT(m_aabb, "Invalid AABB");
		culling = planes_active_camera.isVisible(m_aabb);
		
		if (culling) {
			for (auto& o_plane : occlusionPlanes) {
				culling &= !o_plane.isInside(m_aabb);
				if (!culling)
				{
					break;
				}
			}
			culling &= it->material->isSolid() == solids;
		}		
				
		//culling = true;
		if (*it->active && culling)
		{			
			CTraceScoped scope(((TCompName*)((CEntity*)it->transform.getOwner())->get<TCompName>())->name);
			render_count++;

			if (it->material->getBlendMode() != prev_it->material->getBlendMode() || is_first) {
				activateBlendConfig(it->material->getBlendMode());
			}

			it->lightmap->activate(9);
			/*std::string lightmap = "/lightmaps/" + std::string(m_name->name) + "_lighting";
			const CTexture* light = texture_manager.getByName(lightmap.c_str());
			if (light != nullptr)
				light->activate(9);
			else
				texture_manager.getByName("black")->activate(9);*/

			if (it->material != prev_it->material || is_first) {
				
				// La tech
				if (it->material->getTech() != curr_tech) {
					curr_tech = it->material->getTech();
					curr_tech->activate();
					activateWorldMatrix(0);

					uploading_bones = it->material->getTech()->usesBones();
				}
				
				// Activar shader y material de it
				if (it->owner.isTypeOf<TCompRender>()) 
					it->material->activateTextures(render->emissive_on); 
				else
					it->material->activateTextures(true);
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
			if (((CEntity*)it->transform.getOwner())->hasTag("player"))
				setTransformType(0.2f);
			else
				setTransformType(tmx->getType() / 100.0f);

			setWorldMatrix(tmx->getWorld());
			setTint(it->color);

			// Highligh border -- DOESN'T WORK WITH MULTIPLE SUBMESHES
			/*if (tmx->getType() >= 80 && tmx->getType() <= 90) {
				render_techniques_manager.getByName("highligh")->activate();
				it->mesh->renderGroup(it->mesh_id);
				curr_tech->activate();
			}*/

			// Pintar la mesh:submesh del it
			it->mesh->renderGroup(it->mesh_id);
			
			prev_it = it;
			is_first = false;
		}
		++it;		
	}
	activateBlendConfig(BLEND_CFG_DEFAULT);
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
void CRenderManager::renderShadowsCasters(const CCamera* camera) {
	planes2.create(camera->getViewProjection());
	bool culling = true;
	bool uploading_bones = false;

	for (auto k : shadow_casters_keys) {
		
		if (!k.material->isSolid())
			continue;
		
		if (!k.aabb.isValid()) {
			k.aabb = ((CEntity*)k.transform.getOwner())->get<TCompAABB>();
		}

		TCompAABB* m_aabb = k.aabb;

		XASSERT(k.aabb.isValid(), "Invalid AABB");
		culling = planes_active_camera.isVisible(m_aabb);
		if (culling)
			culling &= planes2.isVisible(m_aabb);

		if (culling) {

			// Activar la world del obj
			TCompTransform* tmx = k.transform;
			assert(tmx);
			setWorldMatrix(tmx->getWorld());

			uploading_bones = k.material->getTech()->usesBones();

			if (uploading_bones) {
				technique_gen_shadows_skel->activate();

				const TCompSkeleton* skel = k.owner;
				XASSERT(skel, "Invalid skeleton");
				skel->uploadBonesToGPU();
			}
			else {
				technique_gen_shadows->activate();
			}


			// Pintar la mesh:submesh del it
			k.mesh->activateAndRender();
		}
	}

}

void CRenderManager::cullActiveCamera() {
	planes_active_camera.create(((TCompCamera*)activeCamera)->getViewProjection());

}

void CRenderManager::addOcclusionPlane(TTransform* a_plane, float the_width, float the_height) {
	OcclusionPlane o_plane;
	o_plane.create(*a_plane, the_width, the_height);
	occlusionPlanes.push_back(o_plane);
}

void CRenderManager::clearOcclusionPlanes() {
	occlusionPlanes.clear();
}