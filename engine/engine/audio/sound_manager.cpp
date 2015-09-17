#include "mcv_platform.h"
#include "sound_manager.h"
#include "components\comp_aabb.h"
#include "components\comp_hfx_zone.h"
#include "ai\logic_manager.h"
#include "entity_manager.h"
#include "render\render_manager.h"
#include "components\comp_camera.h"

static CSoundManager the_sound_manager;
static float volume_factor = 1000;

void CSoundManager::ERRCHECK(FMOD_RESULT result)	// this is an error handling function
{						// for FMOD errors
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}	
}

CSoundManager& CSoundManager::get() {
	return the_sound_manager;
}

CSoundManager::~CSoundManager()
{

	ERRCHECK(stringsBank->unload());
	ERRCHECK(masterBank->unload());

	ERRCHECK(system->release());
	event_descriptions.clear();
}

CSoundManager::CSoundManager()
{
	slowed = false;

	void *extraDriverData = NULL;

	system = NULL;
	ERRCHECK(FMOD::Studio::System::create(&system));

	// The example Studio project is authored for 5.1 sound, so set up the system output mode to match
	FMOD::System* lowLevelSystem = NULL;
	ERRCHECK(system->getLowLevelSystem(&lowLevelSystem));
	ERRCHECK(lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));
	
	ERRCHECK(system->initialize(32, FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, extraDriverData));

	// Load sound bnaks
	masterBank = NULL;
	ERRCHECK(system->loadBankFile("data/sounds/Desktop/Master Bank.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	stringsBank = NULL;
	ERRCHECK(system->loadBankFile("data/sounds/Desktop/Master Bank.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

	invalidPosition = XMVectorSet(0, 0, 0, -112233);

	// Underwater mixer effect
	createMixerEvent("event:/Mixer/underwater");
	createMixerEvent("event:/Mixer/reverbTime");
	createMixerEvent("event:/Mixer/reverbHighCut");
	createMixerEvent("event:/Mixer/reverbHFDecay");
	createMixerEvent("event:/Mixer/reverbEarlyLate");
	createMixerEvent("event:/Mixer/reverbLateDelay");
	createMixerEvent("event:/Mixer/reverbHFReference");
	createMixerEvent("event:/Mixer/reverbDiffusion");
	createMixerEvent("event:/Mixer/reverbDensity");
	createMixerEvent("event:/Mixer/reverbLowGain");
	createMixerEvent("event:/Mixer/reverbLowFreq");
	createMixerEvent("event:/Mixer/reverbWetLevel");
	createMixerEvent("event:/Mixer/reverbDryLevel");
	createMixerEvent("event:/Mixer/reverbEarlyDelay");

	createMixerEvent("event:/Mixer/presetCarpettedHallway");
	createMixerEvent("event:/Mixer/presetBathroom");
	createMixerEvent("event:/Mixer/presetAuditorium");
	createMixerEvent("event:/Mixer/presetConcerthall");
	createMixerEvent("event:/Mixer/presetLivingRoom");
	createMixerEvent("event:/Mixer/presetHallway");
	createMixerEvent("event:/Mixer/presetHangar");
	createMixerEvent("event:/Mixer/presetAlley");
	createMixerEvent("event:/Mixer/presetSewerPipe");
	createMixerEvent("event:/Mixer/presetStoneCorridor");
	createMixerEvent("event:/Mixer/presetPaddedCell");
	createMixerEvent("event:/Mixer/presetRoom");
	createMixerEvent("event:/Mixer/presetQuarry");
	createMixerEvent("event:/Mixer/presetPlain");
}

void CSoundManager::init(){
	HFXZones.clear();
	player = (CEntity*)CEntityManager::get().getByName("Player");
	p_transform = (TCompTransform*)player->get<TCompTransform>();
}

void CSoundManager::createMixerEvent(std::string mixer_event) {
	FMOD::Studio::EventDescription* event_description;
	system->getEvent(mixer_event.c_str(), &event_description);
	
	FMOD::Studio::EventInstance* instance = NULL;
	ERRCHECK(event_description->createInstance(&instance));

	ERRCHECK(instance->start());

	mixer_event_instances[mixer_event] = instance;
}

void CSoundManager::setMixerEventParams(std::string mixer_event, SoundParameter parameter) {

	// Set the parameter
	FMOD::Studio::ParameterInstance* param = NULL;
	FMOD_RESULT r = mixer_event_instances[mixer_event]->getParameter(parameter.name.c_str(), &param);

	float prev_value;
	param->getValue(&prev_value);
	parameter.value = lerp(prev_value, parameter.value, 0.02f);

	ERRCHECK(param->setValue(parameter.value));
}

void CSoundManager::playEvent(std::string path) {
	playEvent(path, 0, 0, invalidPosition);
}

void CSoundManager::playEvent(std::string path, SoundParameter* parameters, int nparameters) {
		
	playEvent(path, parameters, nparameters, invalidPosition);
}

void CSoundManager::playEvent(std::string path, SoundParameter* parameters, int nparameters, XMVECTOR pos) {
	if (event_descriptions.count(path)) {
		// The event exists
	}
	else {
		// The event doesn't exists		
		// Load the event description
		event_descriptions[path] = NULL;
		system->getEvent(path.c_str(), &event_descriptions[path]);

		// The event doesn't exists
		if (event_descriptions[path] == NULL) {
			event_descriptions.erase(path);
			return;
		}
	}

	// Play one shot
	FMOD::Studio::EventInstance* eventInstance = NULL;
	ERRCHECK(event_descriptions[path]->createInstance(&eventInstance));

	// Set the parameters
	for (int i = 0; i < nparameters; ++i) {

		FMOD::Studio::ParameterInstance* param = NULL;

		FMOD_RESULT r = eventInstance->getParameter(parameters[i].name.c_str(), &param);
		if (r != FMOD_OK) { continue; }

		ERRCHECK(param->setValue(parameters[i].value));
	}

	if (XMVectorGetW(pos) != -112233) {
		// 3D Attributes
		FMOD_3D_ATTRIBUTES attributes = { { 0 } };
		attributes.position = XMVECTORtoFmod(pos);
		ERRCHECK(eventInstance->set3DAttributes(&attributes));
	}

	if (slowed)
		eventInstance->setPitch(0.25f);

	ERRCHECK(eventInstance->start());

	// Release will clean up the instance when it completes
	ERRCHECK(eventInstance->release());
}

FMOD::Studio::EventInstance* CSoundManager::getInstance(std::string path) {
	if (event_descriptions.count(path)) {
		// The event exists
	}
	else {
		// The event doesn't exists		
		// Load the event description
		event_descriptions[path] = NULL;
		system->getEvent(path.c_str(), &event_descriptions[path]);

		// The event doesn't exists
		if (event_descriptions[path] == NULL) {
			event_descriptions.erase(path);
			return nullptr;
		}
	}

	FMOD::Studio::EventInstance* eventInstance = NULL;
	ERRCHECK(event_descriptions[path]->createInstance(&eventInstance));

	return eventInstance;
}

bool CSoundManager::setInstanceParams(FMOD::Studio::EventInstance* eventInstance, SoundParameter* parameters, int nparameters){
	bool success = false;
	for (int i = 0; i < nparameters; ++i) {

		FMOD::Studio::ParameterInstance* param = NULL;

		FMOD_RESULT r = eventInstance->getParameter(parameters[i].name.c_str(), &param);
		if (r == FMOD_OK) {
			FMOD_RESULT result = param->setValue(parameters[i].value);
			if (result == FMOD_OK){
				success = true;
			}else{
				return false;
			}
		}else{
			return false;
		}
	}
	return success;
}

bool CSoundManager::setInstancePos(FMOD::Studio::EventInstance* eventInstance, TTransform trans){
	if (XMVectorGetW(trans.position) != -112233) {
		// 3D Attributes
		FMOD_3D_ATTRIBUTES attributes = { { 0 } };
		attributes.position = XMVECTORtoFmod(trans.position);
		attributes.forward = XMVECTORtoFmod(trans.getFront());
		attributes.up = XMVECTORtoFmod(trans.getUp());
		FMOD_RESULT r = eventInstance->set3DAttributes(&attributes);
		if (r == FMOD_OK)
			return true;
		else
			return false;
		return false;
	}
	return false;
}

void CSoundManager::playEvent(std::string path, XMVECTOR pos) {
	playEvent(path, 0, 0, pos);
}

void CSoundManager::setListenerTransform(TTransform listener) {
	FMOD_3D_ATTRIBUTES attributes = { { 0 } };
	attributes.forward = XMVECTORtoFmod(listener.getFront());
	attributes.up = XMVECTORtoFmod(listener.getUp());
	attributes.position = XMVECTORtoFmod(listener.position);
	
	ERRCHECK(system->setListenerAttributes(0, &attributes));
}

void CSoundManager::update(float elapsed) {
	// Update underwater effect
	TCompCamera* cam = render_manager.activeCamera;
	if (cam) {

		XMVECTOR camera_position = cam->getPosition();

		float camera_pos = XMVectorGetY(cam->getPosition());
		float level = CApp::get().water_level;

		float deepness = level - camera_pos;

		CSoundManager::SoundParameter param = { "deepness", deepness };
		setMixerEventParams("event:/Mixer/underwater", param);

		TCompHfxZone* hfx_zone = listenerInsideHFXZone(cam->getPosition());
		if ((hfx_zone) && ((hfx_zone->parametred))){

			param.name = "time";
			param.value = hfx_zone->FReverbTime;
			setMixerEventParams("event:/Mixer/reverbTime", param);

			param.name = "highcut";
			param.value = hfx_zone->FHighCut;
			setMixerEventParams("event:/Mixer/reverbHighCut", param);

			param.name = "hfdecay";
			param.value = hfx_zone->FHFDecay;
			setMixerEventParams("event:/Mixer/reverbHFDecay", param);

			param.name = "earlylate";
			param.value = hfx_zone->FEarlyLate;
			setMixerEventParams("event:/Mixer/reverbEarlyLate", param);

			param.name = "latedelay";
			param.value = hfx_zone->FLateDelay;
			setMixerEventParams("event:/Mixer/reverbLateDelay", param);

			param.name = "hfreference";
			param.value = hfx_zone->FHFReference;
			setMixerEventParams("event:/Mixer/reverbHFReference", param);

			param.name = "diffusion";
			param.value = hfx_zone->FDiffusion;
			setMixerEventParams("event:/Mixer/reverbDiffusion", param);

			param.name = "density";
			param.value = hfx_zone->FDensity;
			setMixerEventParams("event:/Mixer/reverbDensity", param);

			param.name = "lowgain";
			param.value = hfx_zone->FLowGain;
			setMixerEventParams("event:/Mixer/reverbLowGain", param);

			param.name = "lowfreq";
			param.value = hfx_zone->FLowFreq;
			setMixerEventParams("event:/Mixer/reverbLowFreq", param);

			param.name = "wetlevel";
			param.value = hfx_zone->FWetLevel;
			setMixerEventParams("event:/Mixer/reverbWetLevel", param);

			param.name = "drylevel";
			param.value = hfx_zone->FDryLevel;
			setMixerEventParams("event:/Mixer/reverbDryLevel", param);

			param.name = "earlydelay";
			param.value = hfx_zone->FEarlyDelay;
			setMixerEventParams("event:/Mixer/reverbEarlyDelay", param);

		}else{

			param.name = "time";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbTime", param);

			param.name = "highcut";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbHighCut", param);

			param.name = "hfdecay";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbHFDecay", param);

			param.name = "earlylate";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbEarlyLate", param);

			param.name = "latedelay";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbLateDelay", param);

			param.name = "hfreference";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbHFReference", param);

			param.name = "diffusion";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbDiffusion", param);

			param.name = "density";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbDensity", param);

			param.name = "lowgain";
			param.value = 75.f;
			setMixerEventParams("event:/Mixer/reverbLowGain", param);

			param.name = "lowfreq";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbLowFreq", param);

			param.name = "wetlevel";
			param.value = 89.f;
			setMixerEventParams("event:/Mixer/reverbWetLevel", param);

			param.name = "drylevel";
			param.value = 89.f;
			setMixerEventParams("event:/Mixer/reverbDryLevel", param);

			param.name = "earlydelay";
			param.value = 0.f;
			setMixerEventParams("event:/Mixer/reverbEarlyDelay", param);
			if (hfx_zone){
				if (hfx_zone->kind == TCompHfxZone::preset_kind::CARPETTEDHALLWAY){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetCarpettedHallway", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::BATHROOM){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetBathroom", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::AUDITORIUM){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetAuditorium", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::CONCERTHALL){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetConcerthall", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::LIVINGROOM){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetLivingRoom", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::HALLWAY){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetHallway", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::HANGAR){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetHangar", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::ALLEY){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetAlley", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::SEWERPIPE){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetSewerPipe", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::STONECORRIDOR){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetStoneCorridor", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::PADDEDCELL){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetPaddedCell", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::ROOM){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetRoom", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::QUARRY){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetQuarry", param);
				}else if (hfx_zone->kind == TCompHfxZone::preset_kind::PLAIN){
					param.name = "intensity";
					param.value = hfx_zone->intensity;
					setMixerEventParams("event:/Mixer/presetPlain", param);
				}
			}else{
				param.name = "intensity";
				param.value = 0.f;
				setMixerEventParams("event:/Mixer/presetCarpettedHallway", param);
				setMixerEventParams("event:/Mixer/presetBathroom", param);
				setMixerEventParams("event:/Mixer/presetAuditorium", param);
				setMixerEventParams("event:/Mixer/presetConcerthall", param);
				setMixerEventParams("event:/Mixer/presetLivingRoom", param);
				setMixerEventParams("event:/Mixer/presetHallway", param);
				setMixerEventParams("event:/Mixer/presetHangar", param);
				setMixerEventParams("event:/Mixer/presetAlley", param);
				setMixerEventParams("event:/Mixer/presetSewerPipe", param);
				setMixerEventParams("event:/Mixer/presetStoneCorridor", param);
				setMixerEventParams("event:/Mixer/presetPaddedCell", param);
				setMixerEventParams("event:/Mixer/presetRoom", param);
				setMixerEventParams("event:/Mixer/presetQuarry", param);
				setMixerEventParams("event:/Mixer/presetPlain", param);
			}
		}
	}

	system->update();
}

FMOD_VECTOR CSoundManager::XMVECTORtoFmod(XMVECTOR vector) {
	FMOD_VECTOR v;
	v.x = XMVectorGetX(vector);
	v.y = XMVectorGetY(vector);
	v.z = XMVectorGetZ(vector);
	return v;
}

XMVECTOR CSoundManager::FmodToXMVECTOR(FMOD_VECTOR vector) {
	return XMVectorSet(vector.x, vector.y, vector.z, 0);
}

void CSoundManager::playImpactFX(float force, float mass, CHandle transform, std::string material) {
	float material_type = getMaterialTagValue(material);

	float f = force * 8;
	/*loadScene("data/scenes/scene_1_noenemy.xml");*/
	CSoundManager::SoundParameter params[] = {
		{ "force", f },
		{ "mass", mass },
		{ "material", material_type }
	};

	CSoundManager::get().playEvent("event:/Enviroment/impact", params, sizeof(params) / sizeof(CSoundManager::SoundParameter), ((TCompTransform*)transform)->position);
}

void CSoundManager::activateSlowMo(){
	slowed = true;
	
	//play the slow motion transition sound

	//set the pitch in all the events playing
	std::map<std::string, FMOD::Studio::EventDescription*>::iterator it;
	for (it = event_descriptions.begin(); it != event_descriptions.end(); ++it){
		int size = 0;
		it->second->getInstanceCount(&size);
		for (int i = 0; i < size; ++i){
			FMOD::Studio::EventInstance* instace_array[256];
			int count=0;
			it->second->getInstanceList(instace_array, size, &count);
			for (int j = 0; j < count; ++j) {
				instace_array[j]->setPitch(0.25f);
			}
		}
		
	}
}

void CSoundManager::desactivateSlowMo(){
	slowed = false;

	//play inverse slow motion transition sound

	//set pitch to normal value in all the events playing
	std::map<std::string, FMOD::Studio::EventDescription*>::iterator it;
	for (it = event_descriptions.begin(); it != event_descriptions.end(); ++it){
		int size = 0;
		it->second->getInstanceCount(&size);
		for (int i = 0; i < size; ++i){
			FMOD::Studio::EventInstance* instace_array[256];
			int count = 0;
			it->second->getInstanceList(instace_array, size, &count);
			for (int j = 0; j < count; ++j) {
				instace_array[j]->setPitch(1.f);
			}
		}

	}

}

bool CSoundManager::getSlow(){
	return slowed;
}


float CSoundManager::getMaterialTagValue(std::string material) {
	if (material == "wood") { return 0; }
	if (material == "metal") { return 1; }
	if (material == "ceramics") { return 2; }
	if (material == "cloth") { return 3; }
	if (material == "wicker") { return 4; }
	if (material == "leather") { return 5; }
	if (material == "book") { return 6; }
	if (material == "rattle") { return 7; }
	if (material == "glass") { return 8; }
	if (material == "plastic") { return 9; }
	if (material == "piano") { return 10; }
	// Default: Wood
	return 0;
}

void CSoundManager::registerHFXZone(CHandle hfx_zone){
	HFXZones.push_back(hfx_zone);
}

void CSoundManager::unregisterHFXZone(CHandle hfx_zone){
	auto it = std::find(HFXZones.begin(), HFXZones.end(), hfx_zone);
	HFXZones.erase(it);
}

CHandle CSoundManager::listenerInsideHFXZone(XMVECTOR cam_pos){
	for (int i = 0; i < HFXZones.size(); ++i){
		TCompHfxZone* HFXZone_comp = HFXZones[i];
		CEntity* entity = HFXZones[i].getOwner();
		TCompAABB* HFXAABB = entity->get<TCompAABB>();
		if (HFXAABB->containts(cam_pos)){
			return HFXZone_comp;
		}
	}
	return CHandle();
}
