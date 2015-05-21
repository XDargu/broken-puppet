#ifndef INC_COMP_AUDIO_LISTENER_H_
#define INC_COMP_AUDIO_LISTENER_H_

#include "base_component.h"
#include "audio\bass.h"

struct TCompAudioListener : TBaseComponent {
private:
	CHandle m_transform;
	BASS_3DVECTOR* pos;
	BASS_3DVECTOR* front;
	BASS_3DVECTOR* top;
public:

	TCompAudioListener(){}
	~TCompAudioListener();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

};

#endif

