#include "mcv_platform.h"
#include "comp_hfx_zone.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "aabb.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"

TCompHfxZone::TCompHfxZone() {}

TCompHfxZone::~TCompHfxZone() {
	CSoundManager::get().unregisterHFXZone(this);
}

void TCompHfxZone::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);
	
	if (elem == "reverb"){
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
	}
}

void TCompHfxZone::init() {
	CSoundManager::get().registerHFXZone(this);
}



