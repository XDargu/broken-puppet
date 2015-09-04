#ifndef INC_COMP_AUDIO_LISTENER_H_
#define INC_COMP_AUDIO_LISTENER_H_

#include "base_component.h"

struct TCompAudioListener : TBaseComponent {
private:
	CHandle h_transform;
public:

	TCompAudioListener(){}
	~TCompAudioListener();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

};

#endif

