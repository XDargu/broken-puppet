#ifndef INC_COMP_AUDIO_SOURCE_H_
#define INC_COMP_AUDIO_SOURCE_H_

#include "base_component.h"
#include "audio\bass.h"
#include "audio\sound.h"

struct TCompAudioSource : TBaseComponent {
private:
	CHandle player;
	CHandle player_transform;
	CHandle own_transform;
	BASS_3DVECTOR* pos;
	BASS_3DVECTOR* front;
	DWORD m_mode;
	float m_min;
	float m_max;
	float distance_max;
	float volume;
	bool autoPlaySound;
	bool played;
	bool loop;
public:

	CSound asociated_sound;

	TCompAudioSource();
	~TCompAudioSource();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void setSoundAsociated(std::string name);

	void setSoundAsociated(std::string name, DWORD mode, float min, float max);

	void set3DAttributes(DWORD mode, float min, float max);

	void setReverbHFX(CHandle comp_hfx, HSTREAM channel);

	void setFreeReverbHFX(CHandle comp_hfx, HSTREAM channel);

	void setEchoHFX(CHandle comp_hfx, HSTREAM channel);

	void activateSlowMoFilter(HSTREAM channel);

	void desactivateSlowMoFilter(HSTREAM channel);

	void activateLowPassFilter(HSTREAM channel);

	void desactivateLowPassFilter(HSTREAM channel);
};

#endif

