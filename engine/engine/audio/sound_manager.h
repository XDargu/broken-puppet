#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "components\comp_transform.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

class CSoundManager
{
private:
	FMOD::Studio::Bank* masterBank;
	FMOD::Studio::Bank* stringsBank;

	FMOD::Studio::System* system;
	std::map<std::string, FMOD::Studio::EventDescription*> event_descriptions;

	XMVECTOR invalidPosition;

	bool slowed;

	FMOD::Studio::EventInstance* underwater_mixer;

public:
	
	struct SoundParameter {
		std::string name;
		float value;
	};

public:
	CSoundManager();
	~CSoundManager();
	static CSoundManager& get();

	void playEvent(std::string path, SoundParameter* parameters, int nparameters);
	void playEvent(std::string path);

	void playEvent(std::string path, SoundParameter* parameters, int nparameters, XMVECTOR pos);

	FMOD::Studio::EventInstance* getInstance(std::string path);
	bool setInstanceParams(FMOD::Studio::EventInstance* eventInstance, SoundParameter* parameters, int nparameters);
	bool setInstancePos(FMOD::Studio::EventInstance* eventInstance, TTransform trans);

	void playEvent(std::string path, XMVECTOR pos);

	void setListenerTransform(TTransform listener);

	void update(float elapsed);

	void playImpactFX(float force, CHandle transform, std::string material);

	FMOD_VECTOR XMVECTORtoFmod(XMVECTOR vector);
	XMVECTOR FmodToXMVECTOR(FMOD_VECTOR vector);

	void activateSlowMo();
	void desactivateSlowMo();

	void ERRCHECK(FMOD_RESULT result);

	bool getSlow();

};


#endif