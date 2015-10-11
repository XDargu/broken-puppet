#ifndef INC_MATERIAL_H_
#define INC_MATERIAL_H_

#include "items_by_name.h"
#include "texture.h"
#include "XMLParser.h"
#include "render\render_utils.h"

class CMaterial : public CXMLParser {
	const CTexture*           diffuse;
	const CTexture*           ao;
	const CTexture*           normal;
	const CTexture*           specular;
	const CTexture*           glossiness;
	const CTexture*           emissive;
	const CTexture*           emissive_off;
	const CTexture*           cubemap;
	const CRenderTechnique*   tech;
	std::string               name;
	bool                      casts_shadows;
	bool                      solid;
	bool                      double_sided;
	BlendConfig				  blend_mode;
	
	void onStartElement(const std::string &elem, MKeyValue &atts);

public:
	CMaterial() : diffuse(nullptr), normal(nullptr), specular(nullptr), tech(nullptr), casts_shadows(true), solid(true), double_sided(false) {}
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	void destroy();
	const std::string& getName() const { return name; }
	void activateTextures(bool emissive_on, bool swap_diffuse) const;
	void activateEmissive(bool emissive_on) const;
	const CRenderTechnique* getTech() const { return tech; }
	bool castsShadows() const { return casts_shadows; }
	bool isSolid() const { return solid; }
	bool isDoubleSided() const { return double_sided; }
	BlendConfig getBlendMode() const { return blend_mode; }

	void setButtonHoverTextures(bool is_hovering);

};

typedef CItemsByName< CMaterial > CMaterialManager;
extern CMaterialManager material_manager;


#endif
