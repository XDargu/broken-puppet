#ifndef INC_MATERIAL_H_
#define INC_MATERIAL_H_

#include "items_by_name.h"
#include "texture.h"
#include "XMLParser.h"

class CMaterial : public CXMLParser {
	const CTexture*           diffuse;
	const CTexture*           ao;
	const CTexture*           normal;
	const CTexture*           specular;
	const CTexture*           glossiness;
	const CTexture*           emissive;
	const CTexture*           cubemap;
	const CRenderTechnique*   tech;
	std::string               name;
	bool                      casts_shadows;
	void onStartElement(const std::string &elem, MKeyValue &atts);

public:
	CMaterial() : diffuse(nullptr), normal(nullptr), specular(nullptr), tech(nullptr), casts_shadows(true) {}
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	void destroy();
	const std::string& getName() const { return name; }
	void activateTextures() const;
	const CRenderTechnique* getTech() const { return tech; }
	bool castsShadows() const { return casts_shadows; }
};

typedef CItemsByName< CMaterial > CMaterialManager;
extern CMaterialManager material_manager;


#endif
