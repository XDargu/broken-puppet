#ifndef INC_RENDER_MANAGER_H_
#define INC_RENDER_MANAGER_H_

#include "handle/handle.h"

class CMaterial;
class CMesh;
class CCamera;

class CRenderManager {

	struct TKey {
		const CMaterial*   material;
		const CMesh*       mesh;
		int                mesh_id;
		CHandle            owner;
		CHandle            transform;

		bool operator==(const TKey &key) const {
			return key.owner == owner && key.mesh_id == mesh_id && key.material == material && key.mesh == mesh;
		}
	};

	typedef std::vector< TKey > VKeys;
	VKeys keys;
	bool  sort_required;

	static bool sort_by_material_then_mesh(const CRenderManager::TKey& k1, const CRenderManager::TKey& k2);

public:

	void addKey(const CMesh*      mesh
		, const CMaterial*  material
		, int  mesh_id
		, CHandle owner
		);

	void removeKeysFromEntity(CHandle owner);

	void renderAll(const CCamera* camera);

	void destroyAllKeys();

};

extern CRenderManager render_manager;


#endif
