#ifndef INC_COMP_RENDER_H_
#define INC_COMP_RENDER_H_

class CMaterial;

struct TCompRender {

	struct RKey {
		std::string meshName;
		std::string matName;
		int subMeshId;
	};

	const CMesh*     mesh;
	unsigned         submesh_id;
	const CMaterial* mat;

	std::vector< RKey > keys;

	TCompRender() : mesh(nullptr), submesh_id(0), mat(nullptr) { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts);
};

#endif
