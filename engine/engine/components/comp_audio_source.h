#ifndef INC_COMP_AUDIO_SOURCE_H_
#define INC_COMP_AUDIO_SOURCE_H_

#include "base_component.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

struct TCompAudioSource : TBaseComponent {
private:
	CHandle player;
	CHandle player_transform;
	CHandle own_transform;
	bool played;
	bool autoPlay;
	FMOD::Studio::EventInstance* asociated_sound;
	std::vector<std::string> params_names;
	std::vector<float> params_value;
	CHandle name;
public:
	FMOD::Studio::EventInstance* asociated_sound;
public:

	TCompAudioSource();
	~TCompAudioSource();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void setInstance(std::string event_desc_name);

	void update(float elapsed);

	CHandle getName();

	void play();
};

#endif

