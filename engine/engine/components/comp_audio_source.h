#ifndef INC_COMP_AUDIO_SOURCE_H_
#define INC_COMP_AUDIO_SOURCE_H_

#include "base_component.h"
#include "audio\bass.h"
#include "audio\sound.h"

struct TCompAudioSource : TBaseComponent {
private:
	CHandle m_transform;
	BASS_3DVECTOR* pos;
	BASS_3DVECTOR* front;
	DWORD m_mode;
	float m_min;
	float m_max;
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

};

#endif

