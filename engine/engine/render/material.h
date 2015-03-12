#ifndef INC_MATERIAL_H_
#define INC_MATERIAL_H_

#include "items_by_name.h"
#include "texture.h"
#include "XMLParser.h"

class CMaterial : public CXMLParser {
	const CTexture*           diffuse;
	const CRenderTechnique*   tech;
	std::string               name;
	void onStartElement(const std::string &elem, MKeyValue &atts);

public:
	CMaterial() : diffuse(nullptr), tech(nullptr) {}
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const std::string& getName() const { return name; }
	void activateTextures() const;
	const CRenderTechnique* getTech() const { return tech; }
};

typedef CItemsByName< CMaterial > CMaterialManager;
extern CMaterialManager material_manager;


#endif
