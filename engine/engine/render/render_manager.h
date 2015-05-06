#ifndef INC_RENDER_MANAGER_H_
#define INC_RENDER_MANAGER_H_

#include "handle/handle.h"
#include "culling.h"

class CMaterial;
class CMesh;
class CCamera;
struct TTransform;

class CRenderManager {

	VPlanes planes;

	struct TKey {
		const CMaterial*   material;
		const CMesh*       mesh;
		int                mesh_id;
		CHandle            owner;
		CHandle            transform;
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

	void addKey(const CMesh*      mesh
		, const CMaterial*  material
		, int  mesh_id
		, CHandle owner
		, bool* active
		);

	void removeKeysFromOwner(CHandle owner);

	void renderAll(const CCamera* camera);
	void renderAll(const CCamera* camera, TTransform* camera_transform);
	
	void renderShadowsCasters();

	void destroyAllKeys();

};

extern CRenderManager render_manager;


#endif
