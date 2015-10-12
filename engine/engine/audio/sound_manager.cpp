#include "mcv_platform.h"
#include "sound_manager.h"
#include "components\comp_aabb.h"
#include "components\comp_hfx_zone.h"
#include "ai\logic_manager.h"
#include "entity_manager.h"
#include "render\render_manager.h"
#include "components\comp_camera.h"
#include "sound_events_parser.h"

static CSoundManager the_sound_manager;
static float volume_factor = 1000;
const unsigned int max_dist_events = 400;

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

	// Load sound events
	CSoundEventsParser sep(this);
	sep.xmlParseFile("data/sounds/sound_paths.xml");

	slowed = false;

	void *extraDriverData = NULL;

	system = NULL;
	ERRCHECK(FMOD::Studio::System::create(&system));

	// The example Studio project is authored for 5.1 sound, so set up the system output mode to match
	FMOD::System* lowLevelSystem = NULL;
	ERRCHECK(system->getLowLevelSystem(&lowLevelSystem));
	ERRCHECK(lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0));
	
	ERRCHECK(system->initialize(1024, FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, extraDriverData));
	
	ERRCHECK(FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_WARNING, FMOD_DEBUG_MODE_TTY, 0, "fmod_log.txt"));

	// Load sound bnaks
	masterBank = NULL;
	ERRCHECK(system->loadBankFile("data/sounds/Desktop/Master Bank.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	stringsBank = NULL;
	ERRCHECK(system->loadBankFile("data/sounds/Desktop/Master Bank.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

	invalidPosition = XMVectorSet(0, 0, 0, -112233);

	// Underwater mixer effect
	createMixerEvent("UNDERWATER", MixerInstanceType::UNDERWATER);
	createMixerEvent("CURRENT_ROOM", MixerInstanceType::ROOM);
}

void CSoundManager::init(){
	HFXZones.clear();
	player = (CEntity*)CEntityManager::get().getByName("Player");
	p_transform = (TCompTransform*)player->get<TCompTransform>();
	scene_id = 0;
}

void CSoundManager::clear() {
	std::map<std::string, FMOD::Studio::EventDescription*>::iterator it;
	for (it = event_descriptions.begin(); it != event_descriptions.end(); ++it){
		int size = 0;
		it->second->getInstanceCount(&size);
		for (int i = 0; i < size; ++i){
			FMOD::Studio::EventInstance* instace_array[1024];
			int count = 0;
			it->second->getInstanceList(instace_array, size, &count);
			for (int j = 0; j < count; ++j) {
				instace_array[j]->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
				instace_array[j]->release();
			}
		}
	}
}

void CSoundManager::createMixerEvent(std::string sound_id, MixerInstanceType type, CHandle hfx_zone) {
	std::string path = sound_events[sound_id];

	FMOD::Studio::EventDescription* event_description;
	system->getEvent(path.c_str(), &event_description);
	
	FMOD::Studio::EventInstance* instance = NULL;
	ERRCHECK(event_description->createInstance(&instance));

	ERRCHECK(instance->start());

	MixerInstance m_instance;
	m_instance.instance = instance;
	m_instance.type = type;
	m_instance.hfz_zone = hfx_zone;

	mixer_event_instances[sound_id] = m_instance;
}

void CSoundManager::setMixerEventParams(std::string sound_id, SoundParameter parameter, float lerp_val) {

	// Set the parameter
	FMOD::Studio::ParameterInstance* param = NULL;
	FMOD_RESULT r = mixer_event_instances[sound_id].instance->getParameter(parameter.name.c_str(), &param);

	float prev_value;
	param->getValue(&prev_value);
	parameter.value = lerp(prev_value, parameter.value, lerp_val);

	ERRCHECK(param->setValue(parameter.value));
}

float CSoundManager::getMixerEventParamValue(std::string sound_id, std::string param_name) {
	FMOD::Studio::ParameterInstance* param = NULL;
	FMOD_RESULT r = mixer_event_instances[sound_id].instance->getParameter(param_name.c_str(), &param);

	float prev_value;
	param->getValue(&prev_value);
	return prev_value;
}

FMOD::Studio::EventInstance* CSoundManager::playEvent(std::string sound_id, std::string name) {
	return playEvent(sound_id, 0, 0, invalidPosition, name);
}

FMOD::Studio::EventInstance* CSoundManager::playEvent(std::string sound_id, SoundParameter* parameters, int nparameters, std::string name) {
		
	return playEvent(sound_id, parameters, nparameters, invalidPosition, name);
}

FMOD::Studio::EventInstance* CSoundManager::playEvent(std::string sound_id, SoundParameter* parameters, int nparameters, XMVECTOR pos, std::string name) {
	if (sound_id == "") { return nullptr; }

	std::string path = sound_events[sound_id];
	if (event_descriptions.count(path)) {
		// The event exists
	}
	else {
		// The event doesn't exists		
		// Load the event description
		event_descriptions[path] = NULL;
		ERRCHECK(system->getEvent(path.c_str(), &event_descriptions[path]));

		// The event doesn't exists
		if (event_descriptions[path] == NULL) {
			event_descriptions.erase(path);
			return nullptr;
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

	// Set the instance name
	if (name != "") {
		named_instances[name] = eventInstance;
	}

	return eventInstance;
}

FMOD::Studio::EventInstance* CSoundManager::getInstance(std::string sound_id) {
	if (sound_id == "") { return nullptr; }

	std::string path = sound_events[sound_id];

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

FMOD::Studio::EventInstance* CSoundManager::playEvent(std::string path, XMVECTOR pos, std::string name) {
	TCompCamera* cam = render_manager.activeCamera;
	if (cam) {
		XMVECTOR camera_position = cam->getPosition();
		float distance_to_listener = V3DISTANCE(camera_position, pos);
		if (distance_to_listener <= max_dist_events){
			return playEvent(path, 0, 0, pos, name);
		}
	}
	return nullptr;
}

void CSoundManager::playTalkEvent(std::string path, SoundParameter* parameters, int nparameters, XMVECTOR pos, std::string name, std::string guid){
	phrase.path = path;
	phrase.pos = pos;
	phrase.nparameters = nparameters;
	for (int i = 0; i < phrase.nparameters; ++i) {
		phrase.parameters[i] = parameters[i];
	}
	phrase.name = name;
	phrase.talked = false;
	phrase.guid = guid;
}

void CSoundManager::playTalkEvent(std::string path, XMVECTOR pos, std::string name, std::string guid){
	phrase.path = path;
	phrase.pos = pos;
	phrase.nparameters = 0;
	phrase.name = name;
	phrase.talked = false;
	phrase.guid = guid;
}

void CSoundManager::checkIfCanTalk(){
	if (!CApp::get().isSlowMotion()){
		if (!phrase.talked){
			phrase.talked = true;
			if (phrase.nparameters > 0){
				if (phrase.guid != ""){
					CLogicManager::get().playSubtitles(phrase.guid);
				}				
				playEvent(phrase.path, phrase.parameters, phrase.nparameters, phrase.pos, phrase.name);				
			}
			else{
				if (phrase.guid != ""){
					CLogicManager::get().playSubtitles(phrase.guid);
				}
				playEvent(phrase.path, 0, 0, phrase.pos, phrase.name);
			}
		}
	}
	else{
		int probando = 1;
		std::string ojete = "probando!!";
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

		CSoundManager::SoundParameter param = { "Deepness", deepness };
		setMixerEventParams("UNDERWATER", param, 1);

		int player_room = CLogicManager::get().getPointZoneID(camera_position);
		CSoundManager::SoundParameter param2 = { "Scene", scene_id };
		setMixerEventParams("CURRENT_ROOM", param2, 1);
		CSoundManager::SoundParameter param3 = { "Room", player_room };
		setMixerEventParams("CURRENT_ROOM", param3, 1);
		
		// Reverb zones
		TCompHfxZone* hfx_zone = listenerInsideHFXZone(cam->getPosition());
		if (hfx_zone) {
			setCurrentReverbEvent(hfx_zone->preset_name, hfx_zone, camera_position);
		}
		else {
			setCurrentReverbEvent("", hfx_zone, camera_position);
		}
	}

	checkIfCanTalk();
	system->update();
}

void CSoundManager::setCurrentReverbEvent(std::string sound_id, TCompHfxZone* hfx_zone, XMVECTOR listener_pos) {

	SoundParameter param;
	param.name = "DistanceReverb";
	param.value = 0;

	//TCompAABB* hfx_aabb = hfx_zone->m_aabb;
	//float dist = sqrt(hfx_aabb->sqrDistance(listener_pos));

	// If the event is not registered, create one with intensity 0
	if (!mixer_event_instances.count(sound_id)) {
		if (sound_id != "") {
			createMixerEvent(sound_id, MixerInstanceType::REVERB, CHandle(hfx_zone));
			/*param.value = dist;
			setMixerEventParams(event_path, param, 1);*/
		}
	}

	// Lerp values
	for (auto& it : mixer_event_instances) {
		if (it.second.type == MixerInstanceType::REVERB) {
			// Get reverb zone
			TCompHfxZone* hfx = it.second.hfz_zone;
			if (hfx) {
				TCompAABB* hfx_aabb = hfx->m_aabb;
				if (hfx_aabb) {
					float dist = sqrt(hfx_aabb->sqrDistance(listener_pos));

					// Set the distance
					param.value = dist;
					setMixerEventParams(it.first, param, 1);
				}
			}
		}
	}

	auto it = mixer_event_instances.begin();

	// Delete event where value afert lerping is 0
	while (it != mixer_event_instances.end()) {
		if (it->second.type == MixerInstanceType::REVERB) {
			float dist = getMixerEventParamValue(it->first, "DistanceReverb");
			if (dist > 4) {
				ERRCHECK(it->second.instance->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT));
				ERRCHECK(it->second.instance->release());
				it = mixer_event_instances.erase(it);
			}
		}
		if (mixer_event_instances.end() != it) {
			++it;
		}
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

	int material_type = getMaterialTagValue(material);

	//float f = force * 8;
	/*loadScene("data/scenes/scene_1_noenemy.xml");*/

	//XDEBUG("force sound+ %f", force);

	CSoundManager::SoundParameter params[] = {
		{ "force", force },
		{ "mass", mass },
	};

	//CSoundManager::get().playEvent("event:/Enviroment/impact", params, sizeof(params) / sizeof(CSoundManager::SoundParameter), ((TCompTransform*)transform)->position);
	switch (material_type)
	{
	case 0:	playEvent("HIT_WOOD", ((TCompTransform*)transform)->position); break;
	case 2:	playEvent("HIT_METAL", ((TCompTransform*)transform)->position); break;
	case 11: playEvent("HIT_RATTLE", ((TCompTransform*)transform)->position); break;
	case 12: playEvent("HIT_BOOK", ((TCompTransform*)transform)->position); break;
	case 13: playEvent("HIT_PIANO", ((TCompTransform*)transform)->position); break;
	case 14: playEvent("HIT_BALL", ((TCompTransform*)transform)->position); break;
	
	default:
		playEvent("HIT_WOOD", ((TCompTransform*)transform)->position);
		break;
	}
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


int CSoundManager::getMaterialTagValue(std::string material) {
	if (material == "wood") { return 0; }
	if (material == "wood_old") { return 1; }
	if (material == "metal") { return 2; }
	if (material == "metal_floor") { return 3; }
	if (material == "ceramics") { return 4; }
	if (material == "stone") { return 5; }
	if (material == "cloth") { return 6; }
	if (material == "wicker") { return 7; }
	if (material == "leather") { return 8; }
	if (material == "glass") { return 9; }
	if (material == "plastic") { return 10; }
	if (material == "rattle") { return 11; }
	if (material == "book") { return 12; }
	if (material == "piano") { return 13; }
	if (material == "ball") { return 14; }
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

void CSoundManager::stopNamedInstance(std::string name, FMOD_STUDIO_STOP_MODE mode) {
	if (name == "") { return; }

	if (named_instances.count(name)) {
		// The instance exists
		if (named_instances[name]->isValid()) {
			named_instances[name]->stop(mode);
			named_instances[name]->release();
		}
	}
}

void CSoundManager::positionNamedInstance(std::string name, XMVECTOR position) {
	if (name == "") { return; }

	if (named_instances.count(name)) {
		// The instance exists
		FMOD_3D_ATTRIBUTES attributes = { { 0 } };
		attributes.position = XMVECTORtoFmod(position);
		ERRCHECK(named_instances[name]->set3DAttributes(&attributes));
	}
}

FMOD::Studio::EventInstance* CSoundManager::getNamedInstance(std::string name) {
	if (name == "") { return nullptr; }

	if (named_instances.count(name)) {
		// The instance exists
		return named_instances[name];
	}

	return nullptr;
}

FMOD_STUDIO_PLAYBACK_STATE CSoundManager::getNamedInstanceState(std::string name) {
	if (name == "") { return FMOD_STUDIO_PLAYBACK_STATE::FMOD_STUDIO_PLAYBACK_STOPPED; }

	if (named_instances.count(name)) {
		// The instance exists
		if (named_instances[name]->isValid()) {
			FMOD_STUDIO_PLAYBACK_STATE state;
			named_instances[name]->getPlaybackState(&state);
			return state;
		}
	}

	return FMOD_STUDIO_PLAYBACK_STATE::FMOD_STUDIO_PLAYBACK_STOPPED;
}