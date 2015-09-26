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
	char type[64];
	char preset_name[64];
public:
	float FReverbTime;
	float FHighCut;
	float FHFDecay;
	float FEarlyLate;
	float FLateDelay;
	float FHFReference;
	float FDiffusion;
	float FDensity;
	float FLowGain;
	float FLowFreq;
	float FWetLevel;
	float FDryLevel;
	float FEarlyDelay;

	float intensity;
	bool parametred;

	enum preset_kind{
		UNDEFINED,
		CARPETTEDHALLWAY,
		BATHROOM,
		AUDITORIUM,
		CONCERTHALL,
		LIVINGROOM,
		HALLWAY,
		HANGAR,
		ALLEY,
		SEWERPIPE,
		STONECORRIDOR,
		PADDEDCELL,
		ROOM,
		QUARRY,
		PLAIN
	};

	preset_kind kind;
	CHandle m_transform;
	CHandle m_aabb;

public:
	TCompHfxZone();
	~TCompHfxZone();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();


};
#endif

