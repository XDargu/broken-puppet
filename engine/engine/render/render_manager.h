#ifndef INC_RENDER_MANAGER_H_
#define INC_RENDER_MANAGER_H_

#include "handle/handle.h"
#include "culling.h"

class CMaterial;
class CMesh;
class CCamera;
struct TTransform;

class CRenderManager {

	const CRenderTechnique* technique_gen_shadows;
	const CRenderTechnique* technique_gen_shadows_skel;

	struct TKey {
		const CMaterial*   material;
		const CMesh*       mesh;
		int                mesh_id;
		CHandle            owner;
		CHandle            transform;
		CHandle            aabb;
		bool*			   active;

		bool operator==(const TKey &key) const {
			return key.owner == owner && key.mesh_id == mesh_id && key.material == material && key.mesh == mesh;
		}
	};

	typedef std::vector< TKey > VKeys;
	VKeys keys;

	struct TShadowCasterKey {
		const CMaterial*   material;
		const CMesh*       mesh;
		CHandle            owner;
		CHandle            transform;
		CHandle            aabb;

		bool operator==(const TShadowCasterKey &key) const {
			return key.owner == owner && key.material == material && key.mesh == mesh;
		}
	};

	// In case we want to save only those keys of objects which 
	// generates shadows.
	typedef std::vector< TShadowCasterKey > VShadowCasterKeys;
	VShadowCasterKeys shadow_casters_keys;

	bool  sort_required;

	static bool sort_by_material_then_mesh(const CRenderManager::TKey& k1, const CRenderManager::TKey& k2);

public:

	VPlanes planes_active_camera;
	VPlanes planes2;
	CHandle		  activeCamera;

	void addKey(const CMesh*      mesh
		, const CMaterial*  material
		, int  mesh_id
		, CHandle owner
		, bool* active
		);

	void init();

	void removeKeysFromOwner(CHandle owner);

	void renderAll(const CCamera* camera, bool solids, bool double_sided);
	void renderAll(const CCamera* camera, TTransform* camera_transform, bool solids, bool double_sided);
	
	void renderShadowsCasters(const CCamera* camera);

	void destroyAllKeys();

	void cullActiveCamera();

};

extern CRenderManager render_manager;


#endif
