#include "mcv_platform.h"
#include "sound_manager.h"
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
	ERRCHECK(system->loadBankFile("data/sounds/Master Bank.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	stringsBank = NULL;
	ERRCHECK(system->loadBankFile("data/sounds/Master Bank.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

	invalidPosition = XMVectorSet(0, 0, 0, -112233);

	// Underwater mixer effect
	FMOD::Studio::EventDescription* underwater_description;
	system->getEvent("event:/Mixer/underwater", &underwater_description);

	underwater_mixer = NULL;
	ERRCHECK(underwater_description->createInstance(&underwater_mixer));
	
	FMOD::Studio::ParameterInstance* param = NULL;
	FMOD_RESULT r = underwater_mixer->getParameter("deepness", &param);
	ERRCHECK(param->setValue(0));
	
	ERRCHECK(underwater_mixer->start());

	
}

CSoundManager::~CSoundManager()
{

	ERRCHECK(stringsBank->unload());
	ERRCHECK(masterBank->unload());

	ERRCHECK(system->release());
	event_descriptions.clear();
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
		float camera_pos = XMVectorGetY(cam->getPosition());
		float level = CApp::get().water_level;

		float deepness = level - camera_pos;

		FMOD::Studio::ParameterInstance* param = NULL;
		FMOD_RESULT r = underwater_mixer->getParameter("deepness", &param);
		ERRCHECK(param->setValue(deepness));
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
