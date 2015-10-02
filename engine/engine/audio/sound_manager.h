#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "components\comp_transform.h"
#include "components\comp_hfx_zone.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

class CSoundManager
{
friend class CSoundEventsParser;
public:
	struct SoundParameter {
		std::string name;
		float value;
	};

	enum MixerInstanceType {
		UNDERWATER,
		REVERB,
		ROOM,
		COUNT
	};

	struct MixerInstance {
		FMOD::Studio::EventInstance* instance;
		MixerInstanceType type;
		CHandle hfz_zone;
	};
private:
	FMOD::Studio::Bank* masterBank;
	FMOD::Studio::Bank* stringsBank;

	FMOD::Studio::System* system;
	std::map<std::string, FMOD::Studio::EventDescription*> event_descriptions;
	std::map<std::string, MixerInstance> mixer_event_instances;

	std::map<std::string, std::string> sound_events;

	std::vector<CHandle> HFXZones;

	XMVECTOR invalidPosition;

	bool slowed;

	FMOD::Studio::EventInstance* underwater_mixer;

	FMOD::Studio::EventInstance* current_event_instance;

	CEntity* player;
	TCompTransform* p_transform;

	void createMixerEvent(std::string sound_id, MixerInstanceType type, CHandle hfx_zone = CHandle());
	void setMixerEventParams(std::string sound_id, SoundParameter param, float lerp_val = 0.02f);
	float getMixerEventParamValue(std::string sound_id, std::string param_name);
	void setCurrentReverbEvent(std::string sound_id, TCompHfxZone* hfx_zone, XMVECTOR listener_pos);

	int scene_id;
	
public:
	CSoundManager();
	~CSoundManager();
	static CSoundManager& get();

	void init();

	void playEvent(std::string sound_id, SoundParameter* parameters, int nparameters);
	void playEvent(std::string sound_id);

	void playEvent(std::string sound_id, SoundParameter* parameters, int nparameters, XMVECTOR pos);

	FMOD::Studio::EventInstance* getInstance(std::string sound_id);
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

	void setCurrentPresetEvent(std::string event_path);

	void setSceneID(int id) { scene_id = id; }
};


#endif