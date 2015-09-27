#include "mcv_platform.h"
#include "comp_hfx_zone.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "aabb.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"

TCompHfxZone::TCompHfxZone() { 
	preset_name[0] = 0x00;
}

TCompHfxZone::~TCompHfxZone() {
	CSoundManager::get().unregisterHFXZone(this);
}

void TCompHfxZone::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	strcpy(preset_name, atts.getString("preset_name", "").c_str());
}

void TCompHfxZone::init() {
	CSoundManager::get().registerHFXZone(this);
}



