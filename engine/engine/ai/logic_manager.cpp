#include "mcv_platform.h"
#include "logic_manager.h"
#include "components\comp_trigger.h"
#include "components\comp_transform.h"
#include "components\comp_skeleton.h"
#include "components\comp_rigid_body.h"
#include "components\comp_name.h"
#include "components\comp_player_controller.h"
#include "components\comp_third_person_camera_controller.h"
#include "components\comp_camera.h"
#include "components\comp_player_pivot_controller.h"
#include "components\comp_camera_pivot_controller.h"
#include "components\comp_golden_needle_logic.h"
#include "components\comp_zone_aabb.h"
#include "components\comp_hfx_zone.h"
#include "components\comp_audio_source.h"
#include "components\comp_particle_group.h"
#include "ai\fsm_player_legs.h"
#include "components\comp_platform_path.h"
#include "entity_manager.h"
#include "render\render_manager.h"
#include "entity_inspector.h"
//#include <SLB\include\SLB\SLB.hpp>
#include "SLB\include\SLB\SLB.hpp"
#include "audio\sound_manager.h"
#include "handle\prefabs_manager.h"
#include "render\render_utils.h"
#include "particles\importer_particle_groups.h"
#include "font/font.h"
#include "localization_parser.h"
#include "cinematic_manager.h"


static CLogicManager logic_manager;
lua_State* L;

CFont         subtitles_font;

CLogicManager& CLogicManager::get() {
	return logic_manager;
}

CLogicManager::CLogicManager() {
	first_blood = false;
}

void CLogicManager::init()
{
	if (subtitles_font.font == nullptr) {
		// Load subtitles
		CLocalizationParser p;
		p.xmlParseFile("data/subtitles/subtitles.xml");
		subtitles_font.create(L"Segoe UI");
	}

	current_subtitle = Subtitle();
	current_subtitle.text = "notext";
	subtitle_counter = 0;	

	lock_on_target = CHandle();

	current_player_zone = 0;

	particle_group_counter = 0;
	p_group_counter = 0;
	water_transform = CEntityManager::get().getByName("water");
	water2_transform = CEntityManager::get().getByName("water2");
	player = CEntityManager::get().getByName("Player");
	camera = CEntityManager::get().getByName("PlayerCamera");
	player_pivot = CEntityManager::get().getByName("PlayerPivot");
	camera_pivot = CEntityManager::get().getByName("CameraPivot");
	if (water_transform.isValid()) {		
		TCompTransform* water_t = ((CEntity*)water_transform)->get<TCompTransform>();
		water_level_dest = XMVectorGetY(water_t->position);
		lerp_water = 0.05f;
	}

	ambient_light = XMVectorSet(1, 1, 1, 1);
	lerp_ambient_light = 0.05f;

	band_heigth = 0;
	band_heigth_dest = 0;
	lerp_bands = 0.05f;

	lock_on_position = XMVectorSet(0, 0, 0, -1);
	lock_player_view = false;

	scene_to_load = "";
	exit_next_frame = false;

	shake_cam = false;
	shake_amount = 0;

	target_fov = 60.0f;
	fov_lerp = -1;

	//triggers.clear();
	timers.clear();
	GNLogic.clear();
}

CLogicManager::~CLogicManager() {}

std::vector<std::string> timers_to_delete;
std::vector<TKeyFrame> keyframes_to_delete;

void CLogicManager::destroy() {
	subtitles_font.destroy();
	clearAnimations();
}

void CLogicManager::update(float elapsed) {
	
	// Update player zone
	for (int i = 0; i < ZonesAABB.size(); ++i){
		TCompZoneAABB* Zone_comp = (TCompZoneAABB*)ZonesAABB[i];
		if (Zone_comp->isPlayerInside()){
			current_player_zone = Zone_comp->getID();
			break;
		}
	}

	char buffer[64];
	sprintf(buffer, "updateCoroutines( %f )", elapsed);
	luaL_dostring(L, buffer);

	SET_ERROR_CONTEXT("Upadating timers", "");
	// Update the timers
	for (auto& it : timers) {
		it.second.update(elapsed);

		// Check if the timer has reached the limit
		if (it.second.counter >= it.second.limit) {
			timers_to_delete.push_back(it.first);
		}
	};

	// Remove timers that has reached the limit
	for (auto& it : timers_to_delete) {
		XDEBUG("Timer %s has reached the time limit", it.c_str());
		execute("onTimerEnd_" + it + "();");
		timers.erase(it);
	}

	// Clear the delete vector, if needed
	if (timers_to_delete.size() > 0)
		timers_to_delete.clear();

	CErrorContext ce2("Updating animations", "");
	for (auto& it : animations) {
		// Update the animation
		it.update(elapsed);		
	};

	// Update ambient light
	
	
	ctes_global.get()->added_ambient_color = XMVectorLerp(ctes_global.get()->added_ambient_color, ambient_light, lerp_ambient_light* elapsed);
	//ctes_global.uploadToGPU();

	// Update water level
	if (water_transform.isValid()) {
		TCompTransform* water_t = ((CEntity*)water_transform)->get<TCompTransform>();
		XMVECTOR water_dest = water_t->position;
		water_dest = XMVectorSetY(water_dest, water_level_dest);
		water_t->position = XMVectorLerp(water_t->position, water_dest, lerp_water * elapsed);

		// Ver el agua por debajo
		if (water2_transform.isValid()) {
			TCompTransform* water2_t = ((CEntity*)water2_transform)->get<TCompTransform>();
			XMVECTOR water2_dest = water2_t->position;
			water2_dest = XMVectorSetY(water2_dest, water_level_dest);
			water2_t->position = XMVectorLerp(water2_t->position, water2_dest, lerp_water * elapsed);
		}

		// �apa hacer flotar
		CHandle madera = ((CEntity*)CEntityManager::get().getByName("plataforma_madera"));
		if (madera.isValid()){
			CHandle rigid = ((CEntity*)madera)->get<TCompRigidBody>();
			if (rigid.isValid()){
				PxTransform aux_pos = ((TCompRigidBody*)rigid)->rigidBody->getGlobalPose();
				aux_pos.p.y = XMVectorGetY(water_t->position);
				((TCompRigidBody*)rigid)->rigidBody->setKinematicTarget(aux_pos);
			}
		}


	}

	// Update FOV
	if (fov_lerp > 0) {
		if (player_pivot.isValid() && camera_pivot.isValid() && camera.isValid()) {
			CHandle camera_c = ((CEntity*)camera)->get<TCompCamera>();

			if (camera_c.isValid()) {
				float current_fov = ((TCompCamera*)camera_c)->getFov();
				float fov_new = lerp(current_fov, deg2rad(target_fov), fov_lerp * elapsed);
				((TCompCamera*)camera_c)->setFov(fov_new);
				((TCompCamera*)camera_c)->update(elapsed);

			}
		}
	}
	// Update lock on
	if (lock_on_target.isValid()) {
		CEntity* lock_on_entity = lock_on_target;
		if (lock_on_entity) {

			if (XMVectorGetW(lock_on_position) == -2) {
				TCompSkeleton* skeleton = lock_on_entity->get<TCompSkeleton>();
				if (skeleton) {
					if (player_pivot.isValid() && camera_pivot.isValid() && camera.isValid()) {
						CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
						CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();
						CHandle camera_c = ((CEntity*)camera)->get<TCompCamera>();
						CHandle camera_controller = ((CEntity*)camera)->get<TCompThirdPersonCameraController>();
						CHandle camera_trans = ((CEntity*)camera)->get<TCompTransform>();

						if (player_pivot_c.isValid() && camera_pivot_c.isValid() && camera_c.isValid() && camera_controller.isValid()) {
							XMVECTOR aux_pos = skeleton->getPositionOfBone(XMVectorGetX(lock_on_position));
							((TCompPlayerPivotController*)player_pivot_c)->aimAt(aux_pos, min(1, 20 * elapsed));
							((TCompCameraPivotController*)camera_pivot_c)->aimAt(aux_pos, min(1, 20 * elapsed));
							((TCompCamera*)camera_c)->update(elapsed);

							if (camera_trans.isValid()) {
								//((TCompTransform*)camera_trans)->aimAt(aux_pos, XMVectorSet(0, 1, 0, 0), min(1, 20 * elapsed));
								((TCompTransform*)camera_trans)->lookAt(aux_pos, XMVectorSet(0, 1, 0, 0));
							}
						}
					}
				}
			}
			else {
				TCompTransform* transform = lock_on_entity->get<TCompTransform>();
				if (transform) {
					if (player_pivot.isValid() && camera_pivot.isValid() && camera.isValid()) {
						CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
						CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();
						CHandle camera_c = ((CEntity*)camera)->get<TCompCamera>();

						if (player_pivot_c.isValid() && camera_pivot_c.isValid() && camera_c.isValid()) {
							XMVECTOR aux_pos = transform->position + lock_on_position;
							((TCompPlayerPivotController*)player_pivot_c)->pointAt(aux_pos);
							((TCompCameraPivotController*)camera_pivot_c)->pointAt(aux_pos);
							((TCompCamera*)camera_c)->update(elapsed);
						}
					}
				}
			}
		}
	}

	if (XMVectorGetW(lock_on_position) == 0) {
		if (player_pivot.isValid() && camera_pivot.isValid() && camera.isValid()) {
			CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
			CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();
			CHandle camera_c = ((CEntity*)camera)->get<TCompCamera>();
			CHandle player_t = ((CEntity*)player)->get<TCompTransform>();

			if (player_pivot_c.isValid() && camera_pivot_c.isValid() && camera_c.isValid()) {
				((TCompPlayerPivotController*)player_pivot_c)->aimAt(lock_on_position, 3 * elapsed);
				((TCompCameraPivotController*)camera_pivot_c)->aimAt(lock_on_position, 3 * elapsed);
				((TCompCamera*)camera_c)->update(elapsed);

				if (lock_player_view && player_t.isValid()) {
					((TCompTransform*)player_t)->aimAt(lock_on_position, XMVectorSet(0, 1, 0, 0), 3 * elapsed);
				}
			}
		}
	}

	if (shake_amount > 0) {
		if (!shake_cam) {
			shake_amount = lerp(shake_amount, 0, 3 * elapsed);
		}
		if (camera.isValid()) {
			CHandle camera_c = ((CEntity*)camera)->get<TCompCamera>();

			if (camera_c.isValid()) {
				// Apply shake to the camera
				TCompTransform* tmx = ((CEntity*)camera_c.getOwner())->get<TCompTransform>();
				if (tmx) {
					XMVECTOR rng = getRandomVector3(XMFLOAT3(-shake_amount, -shake_amount, -shake_amount), XMFLOAT3(shake_amount, shake_amount, shake_amount));
					tmx->position += rng;
					((TCompCamera*)camera_c)->update(elapsed);
				}
			}
		}
	}


	// Quitar, �apa, peligro
	/*CHandle a_camera = render_manager.activeCamera;
	if (a_camera.isValid()) {
		CEntity* a_entity = a_camera.getOwner();
		if (a_entity) {
			if (char_equal(a_entity->getName(), "camera_sustituta")) {
				CEntity* subs = CEntityManager::get().getByName("Substitute");
				if (subs) {
					TCompTransform* subs_t = subs->get<TCompTransform>();
					TCompTransform* cam_t = a_entity->get<TCompTransform>();

					cam_t->aimAt(subs_t->position + XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0), 0.05f);
				}
			}
		}
	}*/

	// Update band height
	band_heigth = lerp(band_heigth, band_heigth_dest, lerp_bands);
	setCinematicBands(band_heigth);

	// Update subtitle counter
	if (current_subtitle.text != "notext") {
		subtitle_counter += elapsed;
		if (subtitle_counter >= current_subtitle.time) {
			subtitle_counter = 0;

			// Next subtitle
			if (current_subtitle.next != "") {
				playSubtitles(current_subtitle.next);
			}
			else {
				current_subtitle = Subtitle();
				current_subtitle.text = "notext";
			}
		}
	}

	if (scene_to_load != "") {
		CApp::get().loadScene(scene_to_load);
	}

	if (exit_next_frame) {
		CApp::get().exitApp();
	}
}

void CLogicManager::draw() {
	if (current_subtitle.text != "notext") {
		unsigned old_col = subtitles_font.color;
		float old_size = subtitles_font.size;
		subtitles_font.color = current_subtitle.color;
		subtitles_font.size = current_subtitle.size;

		XMVECTOR measure = subtitles_font.measureString(current_subtitle.text.c_str());
		float bottom_offset = CApp::get().yres / 15.0f;
		float width = XMVectorGetZ(measure);
		float height = XMVectorGetW(measure);
		
		subtitles_font.printCentered(CApp::get().xres / 2.0f, CApp::get().yres - height * 0.5f - bottom_offset, current_subtitle.text.c_str());

		subtitles_font.color = old_col;
		subtitles_font.size = old_size;
	}
}

void CLogicManager::addSubtitle(std::string guid, Subtitle subtitle) {
	subtitle_map[guid] = subtitle;
}

void CLogicManager::setTimer(std::string the_name, float time) {
	SET_ERROR_CONTEXT("Setting a timer", the_name.c_str());
	XDEBUG("Timer %s is set to %f seconds", the_name.c_str(), time);
	timers[the_name] = CTimer(time);
}

void CLogicManager::registerGNLogic(CHandle golden_logic){
	GNLogic.push_back(golden_logic);
}

void CLogicManager::registerTrigger(CHandle trigger) {
	triggers.push_back(trigger);
}

void CLogicManager::registerZoneAABB(CHandle zone_aabb){
	ZonesAABB.push_back(zone_aabb);
}

void CLogicManager::onTriggerEnter(CHandle trigger, CHandle who) {
	TCompName* c_name = ((CEntity*)trigger)->get<TCompName>();
	TCompName* c_name_who = ((CEntity*)who)->get<TCompName>();

	if (c_name && c_name_who)
		execute("onTriggerEnter_" + std::string(c_name->name) + "(\"" + std::string(c_name_who->name) + "\");");
}
void CLogicManager::onTriggerFirstEnter(CHandle trigger, CHandle who) {
	TCompName* c_name = ((CEntity*)trigger)->get<TCompName>();
	TCompName* c_name_who = ((CEntity*)who)->get<TCompName>();

	if (c_name && c_name_who)
		execute("onTriggerFirstEnter_" + std::string(c_name->name) + "(\"" + std::string(c_name_who->name) + "\");");
}
void CLogicManager::onTriggerExit(CHandle trigger, CHandle who) {
	TCompName* c_name = ((CEntity*)trigger)->get<TCompName>();
	TCompName* c_name_who = ((CEntity*)who)->get<TCompName>();

	if (c_name && c_name_who)
		execute("onTriggerExit_" + std::string(c_name->name) + "(\"" + std::string(c_name_who->name) + "\");");
}

void CLogicManager::unregisterGNLogic(CHandle golden_logic) {
	auto it = std::find(GNLogic.begin(), GNLogic.end(), golden_logic);
	GNLogic.erase(it);
}

void CLogicManager::unregisterTrigger(CHandle trigger) {
	auto it = std::find(triggers.begin(), triggers.end(), trigger);
	triggers.erase(it);
}

void CLogicManager::unregisterZoneAABB(CHandle zone_aabb) {
	auto it = std::find(ZonesAABB.begin(), ZonesAABB.end(), zone_aabb);
	ZonesAABB.erase(it);
}

CHandle CLogicManager::getPlayerZoneName(){
	for (int i = 0; i < ZonesAABB.size(); ++i){
		TCompZoneAABB* Zone_comp = (TCompZoneAABB*)ZonesAABB[i];
		if (Zone_comp->isPlayerInside()){
			CHandle name = Zone_comp->getZoneName();
			if (name.isValid())
				return Zone_comp->getZoneName();
		}
	}
	return CHandle();
}	

int CLogicManager::getPlayerZoneID(){
	return current_player_zone;
}

int CLogicManager::getPointZoneID(XMVECTOR position) {
	for (int i = 0; i < ZonesAABB.size(); ++i){
		TCompZoneAABB* Zone_comp = (TCompZoneAABB*)ZonesAABB[i];
		if (Zone_comp->isPointInside(position)){
			return Zone_comp->getID();
		}
	}
	return 0;
}


void CLogicManager::onSwitchPressed(CHandle the_switch) {
	TCompName* c_name = ((CEntity*)the_switch)->get<TCompName>();

	if (c_name)
		execute("onSwitchPressed_" + std::string(c_name->name) + "()");
}

void CLogicManager::onSwitchReleased(CHandle the_switch) {
	TCompName* c_name = ((CEntity*)the_switch)->get<TCompName>();

	if (c_name)
		execute("onSwitchReleased_" + std::string(c_name->name) + "()");
}

void CLogicManager::addRigidAnimation(CRigidAnimation animation) {
	// Remove previous animations
	auto it = animations.begin();
	while (it != animations.end()) {
		if (animation.target_transform == it->target_transform) {
			it = animations.erase(it);			
		}
		else ++it;		
	}

	animations.push_back(animation);
}

void CLogicManager::clearAnimations() {
	animations.clear();
}

void CLogicManager::changeWaterLevel(float pos1, float time)
{

	lerp_water = time;
	water_level_dest = pos1;
}

void CLogicManager::changeAmbientLight(float r, float g, float b, float time) {
	ambient_light = XMVectorSet(r, g, b, 1);
	lerp_ambient_light = time;
}

CHandle CLogicManager::instantiateParticleGroup(std::string pg_name, CVector position, CQuaterion rotation){
	CHandle entity = prefabs_manager.getInstanceByName("EmptyEntity");
	if (entity.isValid()) {
		TCompName* name = ((CEntity*)entity)->get<TCompName>();
		if (name) {
			TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
			if (transform) {
				transform->position = XMVectorSet(position.x, position.y, position.z, 0);
				transform->rotation = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
				std::string n_pg_name = "created_pg_" + std::to_string(particle_group_counter);
				std::strcpy(name->name, n_pg_name.c_str());
				particle_group_counter++;
				particle_groups_manager.addParticleGroupToEntity(entity, pg_name);
			}
		}
		return entity;
	}

	return CHandle();
}

CHandle CLogicManager::instantiateParticleGroupOneShot(std::string pg_name, CVector position, CQuaterion rotation){
	CHandle entity = prefabs_manager.getInstanceByName("EmptyEntity");
	if (entity.isValid()) {
		TCompName* name = ((CEntity*)entity)->get<TCompName>();
		if (name) {
			TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
			if (transform) {
				transform->position = XMVectorSet(position.x, position.y, position.z, 0);
				transform->rotation = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
				std::string n_pg_name = "created_pg_" + std::to_string(particle_group_counter);
				std::strcpy(name->name, n_pg_name.c_str());
				particle_group_counter++;
				particle_groups_manager.addParticleGroupToEntity(entity, pg_name);
				TCompParticleGroup* pg = ((CEntity*)entity)->get<TCompParticleGroup>();
				if (pg) {
					pg->destroy_on_death = true;
				}
			}
		}
		return entity;
	}

	return CHandle();
}

/*void CLogicManager::addKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time) {
	// Create the keyframe
	TKeyFrame kf(the_target_transform, the_target_position, the_target_rotation, the_time);

	// Find if there is a keyframe of this transform already there
	bool already_there = false;
	for (auto& it : current_keyframes) {
		// Check if the keyframe has reached the limit
		if (it.target_transform == the_target_transform) {
			already_there = true;
			break;
		}
	};

	if (already_there) {
		keyframe_queue.push_back(kf);
	}
	else {
		current_keyframes.push_back(kf);
	}
}

void CLogicManager::addRelativeKeyFrame(CHandle the_target_transform, XMVECTOR position_offset, XMVECTOR rotation_offset, float the_time) {
	TCompTransform* t = the_target_transform;
	
	XMVECTOR last_pos;
	XMVECTOR last_rot;
	bool found = false;

	// Before create the keyframe, we should find for the last keyframe with this transform
	// Starting with the queue vector
	for (auto& it = keyframe_queue.rbegin(); it != keyframe_queue.rend(); ++it) {
		if (it->target_transform == the_target_transform) {			
			last_pos = XMLoadFloat3(&it->target_position);
			last_rot = XMLoadFloat4(&it->target_rotation);
			found = true;
			break;
		}
	}

	// If we found it, add it
	if (found) {
		TKeyFrame kf(the_target_transform, last_pos + position_offset, XMQuaternionMultiply(last_rot, rotation_offset), the_time);
		keyframe_queue.push_back(kf);
	}

	// If the transform wasn't in the queue vector, search in the current vector
	else {
		for (auto& it = current_keyframes.rbegin(); it != current_keyframes.rend(); ++it) {
			if (it->target_transform == the_target_transform) {
				last_pos = XMLoadFloat3(&it->target_position);
				last_rot = XMLoadFloat4(&it->target_rotation);
				found = true;
			}
		}

		// If it is in the current vector, add it to the queue vector
		if (found) {
			TKeyFrame kf(the_target_transform, last_pos + position_offset, XMQuaternionMultiply(last_rot, rotation_offset), the_time);
			keyframe_queue.push_back(kf);
		}

		// If the transform wasn't either in the current vector, add it
		else {
			TKeyFrame kf(the_target_transform, t->position + position_offset, XMQuaternionMultiply(t->rotation, rotation_offset), the_time);
			current_keyframes.push_back(kf);
		}

	}
}

void CLogicManager::stopKeyframes(CHandle the_target_transform) {

	XDEBUG("Keyframe has been stopped");

	// Erase the keyframe from the current list
	for (auto& it : current_keyframes) {
		// Check if the keyframe has reached the limit
		if (it.target_transform == the_target_transform) {
			keyframes_to_delete.push_back(it);
		}
	};

	// Erase the keyframe from the queue list
	for (auto& it : keyframe_queue) {
		// Check if the keyframe has reached the limit
		if (it.target_transform == the_target_transform) {
			keyframes_to_delete.push_back(it);
		}
	};	
}

void CLogicManager::clearKeyframes() {
	current_keyframes.clear();
	keyframe_queue.clear();
	keyframes_to_delete.clear();
}
*/

void CLogicManager::bootLUA() {

	// Create a lua State, using normal lua API
	L = luaL_newstate();

	// load default functions (optional)
	luaL_openlibs(L);
	
	// Register SLB inside the lua_State, we use here the
	// default manager, but any SLB::Manager could be used.
	SLB::Manager::defaultManager()->registerSLB(L);

	SLB::Class< CLogicManager >("LogicManager")
		.set("loadScene", &CLogicManager::loadScene)
		.set("setTimer", &CLogicManager::setTimer)
		.set("getBot", &CLogicManager::getBot)
		.set("getObject", &CLogicManager::getObject)
		.set("getPrismaticJoint", &CLogicManager::getPrismaticJoint)
		.set("getHingeJoint", &CLogicManager::getHingeJoint)
		.set("getFixedJoint", &CLogicManager::getFixedJoint)
		.set("getMovingPlatform", &CLogicManager::getMovingPlatform)
		.set("changeWaterLevel", (void (CLogicManager::*)(float, float)) &CLogicManager::changeWaterLevel)
		.set("print", &CLogicManager::print)
		.set("pushPlayerLegsState", &CLogicManager::pushPlayerLegsState)
		.set("cameraLookAtBot", (void (CLogicManager::*)(CBot)) &CLogicManager::cameraLookAtBot)
		.set("cameraLookAtPosition", (void (CLogicManager::*)(CVector)) &CLogicManager::cameraLookAtPosition)
		.set("pushPlayerLegsState", &CLogicManager::pushPlayerLegsState)
		.set("changeCamera", &CLogicManager::changeCamera)
		/*.set("changeTrack", &CLogicManager::changeTrack)
		.set("stopMusic", &CLogicManager::stopMusic)
		.set("playMusic", &CLogicManager::playMusic)*/
		.set("changeAmbientLight", &CLogicManager::changeAmbientLight)
		.set("createParticleGroup", (void (CLogicManager::*)(std::string, CVector, CQuaterion)) &CLogicManager::createParticleGroup)
		.set("setBands", &CLogicManager::setBand)
		.set("playEvent", &CLogicManager::playEvent)
		.set("playEventAtPosition", &CLogicManager::playEventAtPosition)
		.set("playEventParameter", &CLogicManager::playEventParameter)
		.set("playEventParameterAtPosition", &CLogicManager::playEventParameterAtPosition)
		.set("playSubtitles", &CLogicManager::playSubtitles)
		.set("setMediumShotActive", &CLogicManager::setPlayerCameraMediumShotActive)
		.set("setLongShotActive", &CLogicManager::setPlayerCameraLongShotActive)
		.set("resetPlayerCamera", &CLogicManager::resetPlayerCamera)
		.set("lockCameraOnBot", &CLogicManager::lockOnBot)
		.set("lockCameraOnBotBone", &CLogicManager::lockOnBotBone)
		.set("lockCameraOnPosition", &CLogicManager::lockOnPosition)
		.set("lockCameraOnObject", &CLogicManager::lockOnObject)
		.set("releaseCameraLock", &CLogicManager::releaseCameraLock)
		.set("playAnimation", &CLogicManager::playAnimation)
		.set("setCanThrow", &CLogicManager::setCanThrow)
		.set("setCanTense", &CLogicManager::setCanTense)
		.set("setCanCancel", &CLogicManager::setCanCancel)
		.set("setCanPull", &CLogicManager::setCanPull)
		.set("setCanMove", &CLogicManager::setCanMove)
		.set("shakeCamera", &CLogicManager::shakeCamera)
		.set("stopShakeCamera", &CLogicManager::stopShakeCamera)
		.set("changeFov", &CLogicManager::changeFov)
		.set("stopFovChange", &CLogicManager::stopFovChange)
		.set("createPrefab", (void (CLogicManager::*)(std::string, CVector, CQuaterion)) &CLogicManager::createPrefab)
		.set("drawGUI", &CLogicManager::setDrawGUI)
		.set("setFogAttributes", &CLogicManager::setFogAttributes)
		; 

	// Register the bot class
	SLB::Class< CBot >("Bot")
		.set("getLife", &CBot::getLife)
		.set("setLife", &CBot::setLife)
		.set("hurt", &CBot::hurt)
		.set("getPosition", &CBot::getPos)
		.set("teleport", (void (CBot::*)(float, float, float)) &CBot::teleport)
		.set("teleportToPos", (void (CBot::*)(CVector)) &CBot::teleport)
		.set("help", &CBot::help)
	;

	// Register the object class
	SLB::Class< CMCVObject >("Object")
		.set("getPos", &CMCVObject::getPosition)
		.set("setPos", (void (CMCVObject::*)(CVector)) &CMCVObject::setPosition)
		.set("move", (void (CMCVObject::*)(CVector, float)) &CMCVObject::moveToPosition)
		.set("setEmissive", &CMCVObject::setEmissive)
		.set("applyForce", (void (CMCVObject::*)(CVector)) &CMCVObject::applyForce)
		.set("setRender", (void (CMCVObject::*)(bool)) &CMCVObject::setRender)
		.set("riseUpBoss", &CMCVObject::riseUpBoss)
		.set("initLittleTalk", &CMCVObject::initLittleTalk)
		.set("initialRain", &CMCVObject::initialRain)
		.set("firstBombBoss", &CMCVObject::firstBombBoss)
		.set("hitSubstitute", &CMCVObject::hitSubstitute)
		;

	SLB::Class<CVector>("Vector")
		.constructor()
		.constructor<float, float, float>()
		.property("x", &CVector::x)
		.property("y", &CVector::y)
		.property("z", &CVector::z)
	;

	SLB::Class<CQuaterion>("Quaternion")
		.constructor()
		.constructor<float, float, float, float>()
		.property("x", &CQuaterion::x)
		.property("y", &CQuaterion::y)
		.property("z", &CQuaterion::z)
		.property("w", &CQuaterion::w)
		;

	// Moving platforms
	SLB::Class<CMovingPlatform>("MovingPlatform")
		.set("start", (void (CMovingPlatform::*)(float)) &CMovingPlatform::start)
		.set("stop", &CMovingPlatform::stop)
	;

	// Distance joint
	SLB::Class<CPrismaticJoint>("PrismaticJoint")
		.set("setLinearLimit", (void (CPrismaticJoint::*)(float, float, float))&CPrismaticJoint::setLinearLimit)
	;
	
	// Hinge joint
	SLB::Class<CHingeJoint>("PrismaticJoint")
		.set("setMotor", (void (CHingeJoint::*)(float, float))&CHingeJoint::setMotor)
		.set("setLimit", (void (CHingeJoint::*)(float))&CHingeJoint::setLimit)
	;

	// Fixed joint
	SLB::Class<CFixedJoint>("FixedJoint")
		.set("breakJoint", &CFixedJoint::breakJoint)
		;

	SLB::setGlobal<CLogicManager*>(L, &get(), "logicManager");

	luaL_dofile(L, "data/lua/scheduler.lua");
	luaL_dofile(L, "data/lua/test.lua");
	

	execute("onInit();");
}

void CLogicManager::loadScene(std::string scene_name) {
	scene_to_load = scene_name;
}

void CLogicManager::onSceneLoad(std::string scene_name) {
	execute("onSceneLoad_" + scene_name + "();");
}

CBot CLogicManager::getBot(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid bot name: " + name + "\")");

	//CMCVObject e(entity);
	return CBot(entity);
}

CMCVObject CLogicManager::getObject(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid object name: " + name + "\")");

	return CMCVObject(entity);
}

CMovingPlatform CLogicManager::getMovingPlatform(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid moving platform name: " + name + "\")");

	return CMovingPlatform(entity);
}

CPrismaticJoint CLogicManager::getPrismaticJoint(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid prismatic joint name: " + name + "\")");

	return CPrismaticJoint(entity);
}

CFixedJoint CLogicManager::getFixedJoint(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid fixed joint name: " + name + "\")");

	return CFixedJoint(entity);
}

CHingeJoint CLogicManager::getHingeJoint(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid hinge joint name: " + name + "\")");

	return CHingeJoint(entity);
}

void CLogicManager::print(std::string text) {
	dbg(text.c_str());
#ifdef _DEBUG
	CConsole::get().print(text);
#endif
}

void CLogicManager::help() {
	execute("SLB.using(SLB)\nlogicManager:print(tostring(LogicManager))");
}

void CLogicManager::execute(std::string text) {
	std::string ex = "SLB.using(SLB)\n" + text;
	luaL_dostring(L, ex.c_str());
}

void CLogicManager::cameraLookAt(XMVECTOR target) {
	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			((TCompPlayerPivotController*)player_pivot_c)->pointAt(target);
			((TCompCameraPivotController*)camera_pivot_c)->pointAt(target);
		}
	}
}

void CLogicManager::cameraLookAtBot(CBot bot) {
	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			CVector pos = bot.getPos();
			XMVECTOR v_pos = XMVectorSet(pos.x, pos.y, pos.z, 0);
			((TCompPlayerPivotController*)player_pivot_c)->pointAt(v_pos);
			((TCompCameraPivotController*)camera_pivot_c)->pointAt(v_pos);
		}
	}
}

void CLogicManager::cameraLookAtPosition(CVector target){
	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			XMVECTOR v_pos = XMVectorSet(target.x, target.y, target.z, 0);
			((TCompPlayerPivotController*)player_pivot_c)->pointAt(v_pos);
			((TCompCameraPivotController*)camera_pivot_c)->pointAt(v_pos);
		}
	}
}

void CLogicManager::playerDead(float phrase, std::string subtitle_guid, float time) {
	std::string message = "onPlayerDead(" + std::to_string(phrase) + ", \"" + subtitle_guid + "\", " + std::to_string(time) + ")";
	execute(message);
}

void CLogicManager::pushPlayerLegsState(std::string state_name) {
	if (player.isValid()) {
		CEntity* p_entity = player;
		CHandle p_controller = p_entity->get<TCompPlayerController>();
		if (p_controller.isValid()) {
			TCompPlayerController* controller = p_controller;
			controller->fsm_player_legs->ChangeState(state_name);
		}
	}

}

void CLogicManager::changeCamera(std::string name) {
	CHandle camera_entity = CEntityManager::get().getByName(name.c_str());
	if (camera_entity.isValid()) {
		CEntity* e = camera_entity;
		TCompCamera* camera = e->get<TCompCamera>();
		if (camera) {
			render_manager.activeCamera = camera;
		}
	}
}

bool CLogicManager::playerInsideGNZone(XMVECTOR& vector, CHandle& logicGN){
	for (int i = 0; i < GNLogic.size(); ++i){
		TCompGNLogic* GN_comp = (TCompGNLogic*)GNLogic[i];
		if (GN_comp->checkPlayerInside()){
			vector = GN_comp->getCluePoint();
			logicGN = GN_comp;
			return true;
		}
	}
	return false;
}

/*void CLogicManager::changeTrack(std::string name, bool loop) {
	CSoundManager::get().playTrack(name, loop);
}

void CLogicManager::stopMusic() {
	CSoundManager::get().stopMusic();
}

void CLogicManager::playMusic(bool loop) {
	CSoundManager::get().playMusic(loop);
}*/

void CLogicManager::playEvent(std::string name) {
	CSoundManager::get().playEvent(name);
}

void CLogicManager::playEventAtPosition(std::string name, CVector pos){
	CSoundManager::get().playEvent(name, XMVectorSet(pos.x, pos.y, pos.z, 0));
}

void CLogicManager::playEventParameterAtPosition(std::string name, std::string parameter, float value, CVector pos) {
	CSoundManager::SoundParameter params[] = {
		{ parameter, value }
	};

	CSoundManager::get().playEvent(name, params, sizeof(params) / sizeof(CSoundManager::SoundParameter), XMVectorSet(pos.x, pos.y, pos.z, 0));
}

void CLogicManager::playEventParameter(std::string name, std::string parameter, float value)
{
	CSoundManager::SoundParameter params[] = {
		{ parameter, value }
	};

	CSoundManager::get().playEvent(name, params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
}

void CLogicManager::stringThrown() {
	execute("onStringThrown()");
}

void CLogicManager::stringPulled() {
	execute("onStringPulled()");
}

void CLogicManager::stringsTensed() {
	execute("onStringTensed()");
}

void CLogicManager::stringCancelled() {
	execute("onStringCancel()");
}

void CLogicManager::stringAllCancelled() {
	execute("onStringCancelAll()");
}

void CLogicManager::createParticleGroup(std::string pg_name, CVector position, CQuaterion rotation) {
	CHandle entity = prefabs_manager.getInstanceByName("EmptyEntity");
	if (entity.isValid()) {
		TCompName* name = ((CEntity*)entity)->get<TCompName>();
		TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
		transform->position = XMVectorSet(position.x, position.y, position.z, 0);
		transform->rotation = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
		std::string n_pg_name = "created_pg_" + std::to_string(particle_group_counter);
		std::strcpy(name->name, n_pg_name.c_str());
		particle_group_counter++;
		particle_groups_manager.addParticleGroupToEntity(entity, pg_name);
	}
}

void CLogicManager::setBand(bool bands) {
	if (bands)
		band_heigth_dest = 0.1f;
	else
		band_heigth_dest = 0.0f;
}

// SUBTITLES
void CLogicManager::playSubtitles(std::string guid) {
	if (subtitle_map.count(guid)) {
		// The subtitle exists
		Subtitle subtitle = subtitle_map[guid];
		playEvent(subtitle.sound);
		current_subtitle = subtitle;
	}	
}

void CLogicManager::setPlayerCameraMediumShotActive(bool active) {
	CEntity* camera_entity = CEntityManager::get().getByName("PlayerCamera");
	if (camera_entity) {
		TCompThirdPersonCameraController* camera_controller = camera_entity->get<TCompThirdPersonCameraController>();
		if (camera_controller) {
			camera_controller->medium_shot = active;
			if (active)
				camera_controller->long_shot = false;
		}
	}
}

void CLogicManager::setPlayerCameraLongShotActive(bool active) {
	CEntity* camera_entity = CEntityManager::get().getByName("PlayerCamera");
	if (camera_entity) {
		TCompThirdPersonCameraController* camera_controller = camera_entity->get<TCompThirdPersonCameraController>();
		if (camera_controller) {
			camera_controller->long_shot = active;
			if (active)
				camera_controller->medium_shot = false;
		}
	}
}

void CLogicManager::resetPlayerCamera() {
	CEntity* camera_entity = CEntityManager::get().getByName("PlayerCamera");
	if (camera_entity) {
		TCompThirdPersonCameraController* camera_controller = camera_entity->get<TCompThirdPersonCameraController>();
		if (camera_controller) {
			camera_controller->long_shot = false;
			camera_controller->medium_shot = false;
		}
	}
}

void CLogicManager::lockOnBot(CBot bot, CVector offset) {
	lock_on_target = bot.getEntityHandle();

	lock_on_position = XMVectorSet(offset.x, offset.y, offset.z, -1);

	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			((TCompPlayerPivotController*)player_pivot_c)->active = false;
			((TCompCameraPivotController*)camera_pivot_c)->active = false;
		}
	}
}

void CLogicManager::lockOnBotBone(CBot bot, int bone) {
	lock_on_target = bot.getEntityHandle();

	lock_on_position = XMVectorSet(bone, 0, 0, -2);

	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			((TCompPlayerPivotController*)player_pivot_c)->active = false;
			((TCompCameraPivotController*)camera_pivot_c)->active = false;
		}
	}
}

void CLogicManager::lockOnObject(CMCVObject object) {
	lock_on_target = object.getEntity();

	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			((TCompPlayerPivotController*)player_pivot_c)->active = false;
			((TCompCameraPivotController*)camera_pivot_c)->active = false;
		}
	}
}

void CLogicManager::lockOnPosition(CVector position, bool lock_player_view_active) {
	lock_on_position = XMVectorSet(position.x, position.y, position.z, 0);

	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			((TCompPlayerPivotController*)player_pivot_c)->active = false;
			((TCompCameraPivotController*)camera_pivot_c)->active = false;
			lock_player_view = lock_player_view_active;
		}
	}
}

void CLogicManager::releaseCameraLock() {
	lock_on_target = CHandle();
	lock_on_position = XMVectorSetW(lock_on_position, -1);

	if (player_pivot.isValid() && camera_pivot.isValid()) {
		CHandle player_pivot_c = ((CEntity*)player_pivot)->get<TCompPlayerPivotController>();
		CHandle camera_pivot_c = ((CEntity*)camera_pivot)->get<TCompCameraPivotController>();

		if (player_pivot_c.isValid() && camera_pivot_c.isValid()) {
			((TCompPlayerPivotController*)player_pivot_c)->active = true;
			((TCompCameraPivotController*)camera_pivot_c)->active = true;
			lock_player_view = false;

		}
	}
}

void CLogicManager::playAnimation(std::string name, CMCVObject target_object) {
	CEntity* e = target_object.getEntity();
	if (e) {
		CHandle t = e->get<TCompTransform>();
		if (t.isValid()) {
			addRigidAnimation(
				cinematic_manager.getInstanceByName(name.c_str(), t)
				);
		}
	}
	
}

void CLogicManager::setCanThrow(bool active) {
	CEntity* player_entity = CEntityManager::get().getByName("Player");
	if (player_entity) {		
		TCompPlayerController* player_controller = player_entity->get<TCompPlayerController>();
		if (player_controller) {
			player_controller->fsm_player_torso->can_throw = active;
		}
	}
}

void CLogicManager::setCanPull(bool active) {
	CEntity* player_entity = CEntityManager::get().getByName("Player");
	if (player_entity) {
		TCompPlayerController* player_controller = player_entity->get<TCompPlayerController>();
		if (player_controller) {
			player_controller->fsm_player_torso->can_pull = active;
		}
	}
}

void CLogicManager::setCanCancel(bool active) {
	CEntity* player_entity = CEntityManager::get().getByName("Player");
	if (player_entity) {
		TCompPlayerController* player_controller = player_entity->get<TCompPlayerController>();
		if (player_controller) {
			player_controller->fsm_player_torso->can_cancel = active;
		}
	}
}

void CLogicManager::setCanTense(bool active) {
	CEntity* player_entity = CEntityManager::get().getByName("Player");
	if (player_entity) {
		TCompPlayerController* player_controller = player_entity->get<TCompPlayerController>();
		if (player_controller) {
			player_controller->fsm_player_torso->can_tense = active;
		}
	}
}

void CLogicManager::setCanMove(bool active) {
	CEntity* player_entity = CEntityManager::get().getByName("Player");
	if (player_entity) {
		TCompPlayerController* player_controller = player_entity->get<TCompPlayerController>();
		if (player_controller) {
			player_controller->fsm_player_legs->can_move = active;
		}
	}
}

void CLogicManager::shakeCamera(float amount) {
	shake_cam = true;
	shake_amount = amount;
}

void CLogicManager::stopShakeCamera() {
	shake_cam = false;
}

void CLogicManager::createPrefab(std::string name, CVector position, CQuaterion rotation) {
	CHandle entity = prefabs_manager.getInstanceByName(name.c_str());
	if (entity.isValid()) {
		XDEBUG("Created prefab: %s at x: %f, y: %f, z: %f", name, position.x, position.y, position.z);
		TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
		transform->position = XMVectorSet(position.x, position.y, position.z, 0);
		transform->rotation = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
		transform->init();
		TCompAudioSource* audio = ((CEntity*)entity)->get<TCompAudioSource>();
		audio->init();
		TCompParticleGroup* particle = ((CEntity*)entity)->get<TCompParticleGroup>();
		particle->init();
	}
}

void CLogicManager::exitGame() {
	exit_next_frame = true;
}

void CLogicManager::onBossRopeThrow(){
	execute("onBossRopeThrow()");
}

void CLogicManager::onSubstituteHang(){
	execute("onSubstituteHanged()");
}

void CLogicManager::changeFov(float fov, float lerp) {
	target_fov = fov;
	fov_lerp = lerp;
}

void CLogicManager::stopFovChange() {
	fov_lerp = -1;
}

void CLogicManager::setDrawGUI(bool active) {
	CApp::get().draw_gui = active;
}

void CLogicManager::setFogAttributes(float distance, float level) {
	CApp::get().setFogAttributes(level, distance);
}