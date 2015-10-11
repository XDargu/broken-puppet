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
		const CTexture*	   lightmap;
		XMVECTOR		   color;

		bool operator==(const TKey &key) const {
			return key.owner == owner && key.mesh_id == mesh_id && key.material == material && key.mesh == mesh;
		}
	};

	typedef std::vector< TKey > VKeys;
	VKeys keys;

	struct TShadowCasterKey {
		const CMaterial*   material;
		const CMesh*       mesh;
		bool			   character;
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
	bool  prev_it_emissive_on;
	bool prev_it_swap_textures;

	static bool sort_by_material_then_mesh(const CRenderManager::TKey& k1, const CRenderManager::TKey& k2);

	// Occlusion
	typedef std::vector< OcclusionPlane > VOcclusionPlanes;
	VOcclusionPlanes occlusionPlanes;

public:

	VPlanes planes_active_camera;
	VPlanes planes2;
	CHandle		  activeCamera;

	void addKey(const CMesh*      mesh
		, const CMaterial*  material
		, int  mesh_id
		, CHandle owner
		, bool* active
		, XMVECTOR color
		);

	void init();

	void removeKeysFromOwner(CHandle owner);

	void renderAll(const CCamera* camera, bool solids, bool double_sided);
	void renderAll(const CCamera* camera, TTransform* camera_transform, bool solids, bool double_sided);
	
	void renderShadowsCasters(const CCamera* camera, bool characters);

	void destroyAllKeys();

	void cullActiveCamera();

	void addOcclusionPlane(TTransform* a_plane, float the_width, float the_height);
	void clearOcclusionPlanes();


};

extern CRenderManager render_manager;


#endif
