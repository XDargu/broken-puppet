#ifndef INC_COMP_RENDER_H_
#define INC_COMP_RENDER_H_

#include "base_component.h"
class CMaterial;

struct TCompRender : TBaseComponent {

	struct RKey {
		std::string meshName;
		std::string matName;
		int subMeshId;
	};

	const CMesh*		mesh;
	unsigned			submesh_id;
	const CMaterial*	mat;
	bool emissive_on;
	bool swap_textures;

	XMVECTOR color;

	std::vector< RKey >* keys;

	TCompRender();
	~TCompRender();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
};

#endif
