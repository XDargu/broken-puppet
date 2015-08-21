#ifndef INC_COMP_HFX_ZONE_H_
#define INC_COMP_HFX_ZONE_H_

#include "base_component.h"
#include "audio\bass.h"
#include "aabb.h"

struct TCompHfxZone : TBaseComponent {
	struct type
	{
		enum Enum
		{
			REVERB = (1 << 1),
			ECHO = (1 << 2),
		};
	};
private:
	BASS_DX8_ECHO* echo_params;
	BASS_DX8_REVERB* reverb_params;
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

	bool isEmitterInside(XMVECTOR emitter_pos);

	BASS_DX8_ECHO* getEcho();

	BASS_DX8_REVERB* getReverb();

	unsigned int getType();

	void init();

};
#endif
