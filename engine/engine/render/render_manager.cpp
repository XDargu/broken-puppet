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
		TShadowCasterKey ck = { mesh, owner };
		ck.transform = k.transform;
		shadow_casters_keys.push_back(ck);
	}
}

void CRenderManager::renderAll(const CCamera* camera) {
	renderAll(camera, &TTransform());
}

void CRenderManager::renderAll(const CCamera* camera, TTransform* camera_transform) {
	SET_ERROR_CONTEXT("Rendering entities", "")

	if (sort_required) {
		std::sort(keys.begin(), keys.end(), sort_by_material_then_mesh);
		sort_required = false;
	}
	
	const CRenderTechnique* curr_tech = nullptr;
	activateCamera(*camera, 1);

	bool uploading_bones = false;
	
	bool is_first = true;
	auto prev_it = keys.begin();
	auto it = keys.begin();


	// Frustrum points
	float yFac = tanf(camera->getFov() * 3.14159f / 360.0f);
	float xFac = yFac*camera->getAspectRatio();

	XMVECTOR Forward, Right, Up; //Simply the three columns from your transformation matrix (or the inverse of your view matrix)

	XMVECTOR Position = camera->getPosition();
	Forward = camera->getFront();
	Up = XMVectorSet(0, 1, 0, 0);
	Right = XMVector3Cross(camera->getFront(), Up);
	float zfar = camera->getZFar();
	float znear = camera->getZNear();

	XMFLOAT3 frustum[8];

	XMStoreFloat3(&frustum[0], Position + Forward*zfar - zfar*Right*xFac*zfar + Up*yFac*zfar);
	XMStoreFloat3(&frustum[1], Position + Forward*zfar + zfar*Right*xFac*zfar + Up*yFac*zfar);
	XMStoreFloat3(&frustum[2], Position + Forward*zfar - zfar*Right*xFac*zfar - Up*yFac*zfar);
	XMStoreFloat3(&frustum[3], Position + Forward*zfar + zfar*Right*xFac*zfar - Up*yFac*zfar);
	// and so on for the near plane 
	XMStoreFloat3(&frustum[4], Position + Forward*znear - zfar*Right*xFac*znear + Up*yFac*znear);
	XMStoreFloat3(&frustum[5], Position + Forward*znear + zfar*Right*xFac*znear + Up*yFac*znear);
	XMStoreFloat3(&frustum[6], Position + Forward*znear - zfar*Right*xFac*znear - Up*yFac*znear);
	XMStoreFloat3(&frustum[7], Position + Forward*znear + zfar*Right*xFac*znear - Up*yFac*znear);

	XMFLOAT3 v_min = XMFLOAT3(1000000, 1000000, 1000000);
	XMFLOAT3 v_max = XMFLOAT3(-1000000, -1000000, -1000000);

	for (int i = 0; i < 8; ++i) {

		v_min.x = min(v_min.x, frustum[i].x);
		v_min.y = min(v_min.y, frustum[i].y);
		v_min.z = min(v_min.z, frustum[i].z);

		v_max.x = max(v_max.x, frustum[i].x);
		v_max.y = max(v_max.y, frustum[i].y);
		v_max.z = max(v_max.z, frustum[i].z);
	}

	AABB camera_aabb = AABB(XMLoadFloat3(&v_min), XMLoadFloat3(&v_max));
	
	bool culling = true;
	int render_count = 0;

	while (it != keys.end()) {
		CErrorContext ce2("Rendering key with material", it->material->getName().c_str());
		
		TCompTransform* tmx = it->transform;
		XASSERT(tmx, "Invalid transform");

		
		TCompAABB* m_aabb = ((CEntity*)CHandle(tmx).getOwner())->get<TCompAABB>();
		if (m_aabb)
			culling = camera_aabb.intersects(m_aabb);
		else
			culling = true;

		/*bool is_in_front = XMVectorGetX(XMVector3Dot(camera->getFront(), tmx->position - camera->getPosition())) > 0.f;
		culling = is_in_front;*/
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

			// Activar la world del obj
			setWorldMatrix(tmx->getWorld());

			// Pintar la mesh:submesh del it
			it->mesh->renderGroup(it->mesh_id);

			prev_it = it;
			is_first = false;
		}
		++it;		
	}

	font.printf(200, 50, "Dibujando %i keys de %i", render_count, (int)keys.size());
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
	shadow_casters_keys.clear();
}

// ---------------------------------------------------------------
void CRenderManager::renderShadowsCasters() {

	for (auto k : shadow_casters_keys) {

		// Activar la world del obj
		TCompTransform* tmx = k.transform;
		assert(tmx);
		setWorldMatrix(tmx->getWorld());


		// Pintar la mesh:submesh del it
		k.mesh->activateAndRender();
	}

}