#include "mcv_platform.h"
#include "comp_hfx_zone.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "aabb.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"

TCompHfxZone::TCompHfxZone() {
	echo_params = nullptr;
	reverb_params = nullptr;
	kind = 0;
}

TCompHfxZone::~TCompHfxZone() {
	if (echo_params)
		delete echo_params;
	if (reverb_params)
		delete reverb_params;

	CLogicManager::get().unregisterHFXZone(this);
}

void TCompHfxZone::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);
	if (elem == "HFX") {
		std::string typeHFX = atts.getString("type", "none");
		if (typeHFX == "reverb"){
			float FInG = atts.getFloat("in_gain", 0.f);
			float FReverbMix = atts.getFloat("reverb_mix", 0.f);
			float FReverbTime = atts.getFloat("reverb_time", 0.f);
			float FHighFreqRTRatio = atts.getFloat("high_freq_ratio", 0.f);

			setRevertZoneAtributtes(FInG, FReverbMix, FReverbTime, FHighFreqRTRatio);
			kind = kind | type::REVERB;
		}
		
		if (typeHFX == "echo"){
			float fWetDryMix = atts.getFloat("wet_dry_mix", 0.f);
			float fFeedback = atts.getFloat("feedback", 0.f);
			float fLeftDelay = atts.getFloat("left_delay", 0.f);
			float fRightDelay = atts.getFloat("right_delay", 0.f);
			BOOL lPanDelay = atts.getBool("pan_delay", false);

			setEchoZoneAtributtes(fWetDryMix, fFeedback, fLeftDelay, fRightDelay, lPanDelay);
			kind = kind | type::ECHO;
		}
	}

}

unsigned int TCompHfxZone::getType(){
	return kind;
}

HFX TCompHfxZone::getHFXZoneAtributtes(){
	return HFXEffect;
}

bool TCompHfxZone::isEmitterInside(XMVECTOR emitter_pos){
	AABB struct_aabb = AABB(((TCompAABB*)m_aabb)->min, ((TCompAABB*)m_aabb)->max);
	if (struct_aabb.containts(emitter_pos))
		return true;
	else
		return false;
}

void TCompHfxZone::setRevertZoneAtributtes(float fInGain, float fReverbMix, float fReverbTime, float fHighFreqRTRatio){
	reverb_params = new BASS_DX8_REVERB();
	reverb_params->fInGain = fInGain;
	reverb_params->fReverbMix = fReverbMix;
	reverb_params->fReverbTime = fReverbTime;
	reverb_params->fHighFreqRTRatio = fHighFreqRTRatio;
	//bool success = BASS_FXSetParameters(HFXEffect, reverb_params);
	/*if (!success){
		int code = BASS_ErrorGetCode();
		if (code == BASS_ERROR_HANDLE){
			XASSERT(code, "Error, FX handle invalid");
		}else if (code == BASS_ERROR_ILLPARAM){
			XASSERT(code, "Error, FX params invalid");
		}else if (code == BASS_ERROR_UNKNOWN){
			XASSERT(code, "Error, FX Unknown error");
		}
	}else{
		delete reverb_params;
	}*/
}

void TCompHfxZone::setEchoZoneAtributtes(float fWetDryMix, float fFeedback, float fLeftDelay, float fRightDelay, bool lPanDelay){
	echo_params = new BASS_DX8_ECHO();
	echo_params->fWetDryMix = fWetDryMix;
	echo_params->fFeedback = fFeedback;
	echo_params->fLeftDelay = fLeftDelay;
	echo_params->fRightDelay = fRightDelay;
	echo_params->lPanDelay = lPanDelay;

	/*bool success = BASS_FXSetParameters(HFXEffect, echo_params);
	if (!success){
		int code = BASS_ErrorGetCode();
		if (code == BASS_ERROR_HANDLE){
			XASSERT(code, "Error, FX handle invalid");
		}
		else if (code == BASS_ERROR_ILLPARAM){
			XASSERT(code, "Error, FX params invalid");
		}
		else if (code == BASS_ERROR_UNKNOWN){
			XASSERT(code, "Error, FX Unknown error");
		}
	}else{
		delete echo_params;
	}*/
}

BASS_DX8_ECHO* TCompHfxZone::getEcho(){
	return echo_params;
}

BASS_DX8_REVERB* TCompHfxZone::getReverb(){
	return reverb_params;
}

void TCompHfxZone::init() {
	CLogicManager::get().registerHFXZone(this);
}


