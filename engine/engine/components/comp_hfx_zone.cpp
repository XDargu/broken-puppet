#include "mcv_platform.h"
#include "comp_hfx_zone.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "aabb.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"

TCompHfxZone::TCompHfxZone() { 
	type[0] = 0x00;
	preset_name[0] = 0x00;
}

TCompHfxZone::~TCompHfxZone() {
	CSoundManager::get().unregisterHFXZone(this);
}

void TCompHfxZone::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);
	
	strcpy(type, atts.getString("kind", "undefined").c_str());
	//type = atts.getString("kind", "undefined");
	if (char_equal(type, "free_reverb")){
		FReverbTime = atts.getFloat("reverbTime", 0.f);
		FHighCut = atts.getFloat("highCut", 0.f);
		FHFDecay = atts.getFloat("hfDecay", 0.f);
		FEarlyLate = atts.getFloat("earlyLate", 0.f);
		FLateDelay = atts.getFloat("lateDelay", 0.f);
		FHFReference = atts.getFloat("hfReference", 0.f);
		FDiffusion = atts.getFloat("diffusion", 0.f);
		FDensity = atts.getFloat("density", 0.f);
		FLowGain = atts.getFloat("lowGain", 75.f);
		FLowFreq = atts.getFloat("lowFreq", 0.f);
		FWetLevel = atts.getFloat("wetLevel", 89.f);
		FDryLevel = atts.getFloat("dryLevel", 89.f);
		FEarlyDelay = atts.getFloat("earlyDelay", 0.f);
		kind = UNDEFINED;
		strcpy(preset_name, "undefined");
		//preset_name = "undefined";
		parametred = true;
	}else if (char_equal(type, "preset")){
		strcpy(preset_name, atts.getString("preset_name", "undefined").c_str());
		//preset_name = atts.getString("preset_name", "undefined");
		if (char_equal(preset_name, "carpettedHallway")){
			kind = CARPETTEDHALLWAY;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "bathroom")){
			kind = BATHROOM;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "auditorium")){
			kind = AUDITORIUM;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "concerthall")){
			kind = CONCERTHALL;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "livingroom")){
			kind = LIVINGROOM;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "hallway")){
			kind = HALLWAY;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "hangar")){
			kind = HANGAR;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "alley")){
			kind = ALLEY;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "sewerpipe")){
			kind = SEWERPIPE;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "stonecorridor")){
			kind = STONECORRIDOR;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "paddedcell")){
			kind = PADDEDCELL;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "room")){
			kind = ROOM;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "quarry")){
			kind = QUARRY;
			parametred = false;
			intensity = atts.getFloat("intensity", 0.f);
		}else if (char_equal(preset_name, "plain")){
			kind = PLAIN;
			parametred = false;
			intensity = 100;// atts.getFloat("intensity", 0.f);
		}
	}
}

void TCompHfxZone::init() {
	CSoundManager::get().registerHFXZone(this);
}



