#ifndef INC_COMP_HFX_ZONE_H_
#define INC_COMP_HFX_ZONE_H_

#include "base_component.h"
#include "audio\bass.h"
#include "audio\bass_fx.h"
#include "aabb.h"

struct TCompHfxZone : TBaseComponent {
	struct type
	{
		enum Enum
		{
			REVERB = (1 << 1),
			ECHO = (1 << 2),
			FREE_REVERB = (1 << 3)
		};
	};
private:
	BASS_DX8_ECHO* echo_params;
	BASS_DX8_REVERB* reverb_params;
	BASS_BFX_FREEVERB* free_reverb_params;
	CHandle m_transform;
	CHandle m_aabb;
	HFX HFXEffect;
	unsigned int kind;
public:
	TCompHfxZone();
	~TCompHfxZone();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	HFX getHFXZoneAtributtes();

	void setRevertZoneAtributtes(float fInGain, float fReverbMix, float fReverbTime, float fHighFreqRTRatio);

	void setEchoZoneAtributtes(float fWetDryMix, float fFeedback, float fLeftDelay, float fRightDelay, bool lPanDelay);

	void setFreeReverbZoneAtributtes(float fDryMix, float fWetMix, float fRoomSize, float fDamp, float fWidth, DWORD lMode);

	bool isEmitterInside(XMVECTOR emitter_pos);

	BASS_DX8_ECHO* getEcho();

	BASS_DX8_REVERB* getReverb();

	BASS_BFX_FREEVERB* getFreeReverb();

	unsigned int getType();

	void init();

};
#endif
