#ifndef INC_COMP_MESH_H_
#define INC_COMP_MESH_H_

#include "base_component.h"

struct TCompMesh : TBaseComponent {
	const CMesh* mesh;
	char path[32];
	XMVECTOR color;

	TCompMesh() { mesh = nullptr; color = XMVectorSet(1, 1, 1, 1); }
	TCompMesh(const CMesh* the_mesh) { mesh = the_mesh; strcpy(path, "unknown"); color = XMVectorSet(1, 1, 1, 1); }
	TCompMesh(const char* the_name) { mesh = mesh_manager.getByName(the_name); strcpy(path, the_name); color = XMVectorSet(1, 1, 1, 1); }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		strcpy(path, atts.getString("path", "missing_mesh").c_str());
		mesh = mesh_manager.getByName(path);
		color = atts.getQuat("color");
	}

	std::string toString() {
		return "Mesh: " + std::string(path);
	}
};

#endif
