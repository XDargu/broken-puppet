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
const unsigned int max_dist_events = 40;

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
	createMixerEvent("event:/Mixer/underwater", MixerInstanceType::UNDERWATER);
}

void CSoundManager::init(){
	HFXZones.clear();
	player = (CEntity*)CEntityManager::get().getByName("Player");
	p_transform = (TCompTransform*)player->get<TCompTransform>();
}

void CSoundManager::createMixerEvent(std::string mixer_event, MixerInstanceType type) {
	FMOD::Studio::EventDescription* event_description;
	system->getEvent(mixer_event.c_str(), &event_description);
	
	FMOD::Studio::EventInstance* instance = NULL;
	ERRCHECK(event_description->createInstance(&instance));

	ERRCHECK(instance->start());

	MixerInstance m_instance;
	m_instance.instance = instance;
	m_instance.type = type;

	mixer_event_instances[mixer_event] = m_instance;
}

void CSoundManager::setMixerEventParams(std::string mixer_event, SoundParameter parameter, float lerp_val) {

	// Set the parameter
	FMOD::Studio::ParameterInstance* param = NULL;
	FMOD_RESULT r = mixer_event_instances[mixer_event].instance->getParameter(parameter.name.c_str(), &param);

	float prev_value;
	param->getValue(&prev_value);
	parameter.value = lerp(prev_value, parameter.value, lerp_val);

	ERRCHECK(param->setValue(parameter.value));
}

float CSoundManager::getMixerEventParamValue(std::string mixer_event, std::string param_name) {
	FMOD::Studio::ParameterInstance* param = NULL;
	FMOD_RESULT r = mixer_event_instances[mixer_event].instance->getParameter(param_name.c_str(), &param);

	float prev_value;
	param->getValue(&prev_value);
	return prev_value;
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
	TCompCamera* cam = render_manager.activeCamera;
	if (cam) {
		XMVECTOR camera_position = cam->getPosition();
		float distance_to_listener = V3DISTANCE(camera_position, pos);
		if (distance_to_listener <= max_dist_events){
			playEvent(path, 0, 0, pos);
		}
	}
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

		// Reverb zones
		TCompHfxZone* hfx_zone = listenerInsideHFXZone(cam->getPosition());
		if (hfx_zone) {
			setCurrentReverbEvent(hfx_zone->preset_name);
		}
		else {
			setCurrentReverbEvent("");
		}
	}

	system->update();
}

void CSoundManager::setCurrentReverbEvent(std::string event_path) {

	SoundParameter param;
	param.name = "intensity";
	param.value = 0;

	// If the event is not registered, create one with intensity 0
	if (!mixer_event_instances.count(event_path)) {
		if (event_path != "") {
			createMixerEvent(event_path, MixerInstanceType::REVERB);
			param.value = 100;
			setMixerEventParams(event_path, param, 0.2f);
		}
	}

	// Lerp values
	for (auto& it : mixer_event_instances) {
		if (it.second.type == MixerInstanceType::REVERB) {
			// Lerp to 100 if the value exists
			if (it.first == event_path) {
				param.value = 100;
				setMixerEventParams(it.first, param, 0.2f);
			}
			// Lerp to 0 if it doesn't exists
			else {
				param.value = 0;
				setMixerEventParams(it.first, param, 0.2f);
			}
		}
	}

	auto it = mixer_event_instances.begin();

	// Delete event where value afert lerping is 0
	while (it != mixer_event_instances.end()) {
		if (it->second.type == MixerInstanceType::REVERB) {
			if (getMixerEventParamValue(it->first, "intensity") < 0.02) {
				ERRCHECK(it->second.instance->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE));
				ERRCHECK(it->second.instance->release());
				it = mixer_event_instances.erase(it);
			}
		}
		it++;
	}
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

	if (CApp::get().total_time < 2.5f) { return; }

	float material_type = getMaterialTagValue(material);

	//float f = force * 8;
	/*loadScene("data/scenes/scene_1_noenemy.xml");*/

	XDEBUG("force sound+ %f", force);

	CSoundManager::SoundParameter params[] = {
		{ "force", force },
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
			FMOD::Studio::EventInstance* instace_array[1024];
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
			FMOD::Studio::EventInstance* instace_array[1024];
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
