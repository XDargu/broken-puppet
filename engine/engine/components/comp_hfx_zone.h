#ifndef INC_COMP_HFX_ZONE_H_
#define INC_COMP_HFX_ZONE_H_

#include "base_component.h"
#include "audio\bass.h"
#include "audio\bass_fx.h"
#include "aabb.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

struct TCompHfxZone : TBaseComponent {
private:	

	CHandle m_transform;

public:
	CHandle m_aabb;
	char preset_name[128];

	TCompHfxZone();
	~TCompHfxZone();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();


};
#endif

