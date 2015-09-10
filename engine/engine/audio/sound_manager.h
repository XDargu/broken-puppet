#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "components\comp_transform.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

class CSoundManager
{
public:
	struct SoundParameter {
		std::string name;
		float value;
	};
private:
	FMOD::Studio::Bank* masterBank;
	FMOD::Studio::Bank* stringsBank;

	FMOD::Studio::System* system;
	std::map<std::string, FMOD::Studio::EventDescription*> event_descriptions;
	std::map<std::string, FMOD::Studio::EventInstance*> mixer_event_instances;

	std::vector<CHandle> HFXZones;

	XMVECTOR invalidPosition;

	bool slowed;

	FMOD::Studio::EventInstance* underwater_mixer;

	void createMixerEvent(std::string mixer_event);
	void setMixerEventParams(std::string mixer_event, SoundParameter param);
	
public:
	CSoundManager();
	~CSoundManager();
	static CSoundManager& get();

	void init();

	void playEvent(std::string path, SoundParameter* parameters, int nparameters);
	void playEvent(std::string path);

	void playEvent(std::string path, SoundParameter* parameters, int nparameters, XMVECTOR pos);

	FMOD::Studio::EventInstance* getInstance(std::string path);
	bool setInstanceParams(FMOD::Studio::EventInstance* eventInstance, SoundParameter* parameters, int nparameters);
	bool setInstancePos(FMOD::Studio::EventInstance* eventInstance, TTransform trans);

	void playEvent(std::string path, XMVECTOR pos);

	void setListenerTransform(TTransform listener);

	void registerHFXZone(CHandle hfx_zone);

	void unregisterHFXZone(CHandle hfx_zone);

	CHandle listenerInsideHFXZone(XMVECTOR cam_pos);

	void update(float elapsed);

	void playImpactFX(float force, float mass, CHandle transform, std::string material);

	FMOD_VECTOR XMVECTORtoFmod(XMVECTOR vector);
	XMVECTOR FmodToXMVECTOR(FMOD_VECTOR vector);

	void activateSlowMo();
	void desactivateSlowMo();

	float getMaterialTagValue(std::string material);

	void ERRCHECK(FMOD_RESULT result);

	bool getSlow();
};


#endif