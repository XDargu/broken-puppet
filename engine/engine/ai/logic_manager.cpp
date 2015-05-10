#include "mcv_platform.h"
#include "logic_manager.h"
#include "components\comp_trigger.h"
#include "components\comp_transform.h"
#include "components\comp_rigid_body.h"
#include "components\comp_name.h"
#include "components\comp_platform_path.h"
#include "entity_manager.h"
#include "entity_inspector.h"
//#include <SLB\include\SLB\SLB.hpp>
#include "SLB\include\SLB\SLB.hpp"

static CLogicManager logic_manager;
lua_State* L;

CLogicManager& CLogicManager::get() {
	return logic_manager;
}

CLogicManager::CLogicManager() {

}

void CLogicManager::init()
{
	water_transform = CEntityManager::get().getByName("water");
	if (water_transform.isValid()) {		
		TCompTransform* water_t = ((CEntity*)water_transform)->get<TCompTransform>();
		water_level_dest = XMVectorGetY(water_t->position);
		lerp_water = 0.05f;
	}
}

CLogicManager::~CLogicManager() {}

std::vector<std::string> timers_to_delete;
std::vector<TKeyFrame> keyframes_to_delete;

void CLogicManager::update(float elapsed) {
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

	// Update water level
	if (water_transform.isValid()) {
		TCompTransform* water_t = ((CEntity*)water_transform)->get<TCompTransform>();
		XMVECTOR water_dest = water_t->position;
		water_dest = XMVectorSetY(water_dest, water_level_dest);
		water_t->position = XMVectorLerp(water_t->position, water_dest, lerp_water);

		// Ñapa hacer flotar
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

	/*CErrorContext ce2("Updating keyframes", "");
	// Update the keyframes
	for (auto& it : current_keyframes) {
		// Check if the keyframe has reached the limit
		if (it.update(elapsed)) {
			XDEBUG("Keyframe has reached the time limit");
			keyframes_to_delete.push_back(it);
		}
	};

	// Remove keyframes that has reached the limit
	for (auto& it : keyframes_to_delete) {
	
		// Find if there is another keyframe in the queue with the same target transform, and add it to the current keyframe list
		for (auto& it2 : keyframe_queue) {
			// Check if the keyframe has reached the limit
			if (it2.target_transform == it.target_transform) {
				current_keyframes.push_back(it2);
				
				// Erase the keyframe
				auto it3 = std::find(keyframe_queue.begin(), keyframe_queue.end(), it);
				if (it3 != keyframe_queue.end())
					keyframe_queue.erase(it3);
				break;
			}
		};

		// Erase the keyframe
		auto it3 = std::find(current_keyframes.begin(), current_keyframes.end(), it);
		if (it3 != current_keyframes.end())
			current_keyframes.erase(it3);		
	};

	// Clear the delete vector, if needed
	if (keyframes_to_delete.size() > 0)
		keyframes_to_delete.clear();
	*/
}

void CLogicManager::setTimer(std::string the_name, float time) {
	SET_ERROR_CONTEXT("Setting a timer", the_name.c_str());
	XDEBUG("Timer %s is set to %f seconds", the_name.c_str(), time);
	timers[the_name] = CTimer(time);
}

void CLogicManager::registerTrigger(CHandle trigger) {
	triggers.push_back(trigger);
}

void CLogicManager::onTriggerEnter(CHandle trigger, CHandle who) {
	TCompName* c_name = ((CEntity*)trigger)->get<TCompName>();
	TCompName* c_name_who = ((CEntity*)who)->get<TCompName>();

	if (c_name && c_name_who)
		execute("onTriggerEnter_" + std::string(c_name->name) + "(\"" + std::string(c_name_who->name) + "\");");
}
void CLogicManager::onTriggerExit(CHandle trigger, CHandle who) {
	TCompName* c_name = ((CEntity*)trigger)->get<TCompName>();
	TCompName* c_name_who = ((CEntity*)who)->get<TCompName>();

	if (c_name && c_name_who)
		execute("onTriggerExit_" + std::string(c_name->name) + "(\"" + std::string(c_name_who->name) + "\");");
}

void CLogicManager::unregisterTrigger(CHandle trigger) {
	auto it = std::find(triggers.begin(), triggers.end(), trigger);
	triggers.erase(it);
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
		.set("getPrismaticJoint", &CLogicManager::getPrismaticJoint)
		.set("getHingeJoint", &CLogicManager::getHingeJoint)
		.set("getMovingPlatform", &CLogicManager::getMovingPlatform)
		.set("changeWaterLevel", (void (CLogicManager::*)(float, float)) &CLogicManager::changeWaterLevel)
		.set("print", &CLogicManager::print)
	;

	// Register the bot class
	SLB::Class< CBot >("Bot")
		.set("getLife", &CBot::getLife)
		.set("setLife", &CBot::setLife)
		.set("hurt", &CBot::hurt)
		.set("getPos", &CBot::getPos)
		.set("teleport", (void (CBot::*)(float, float, float)) &CBot::teleport)
		.set("teleportToPos", (void (CBot::*)(CVector)) &CBot::teleport)
		.set("help", &CBot::help)
	;

	SLB::Class<CVector>("Vector")
		.constructor()
		.constructor<float, float, float>()
		.property("x", &CVector::x)
		.property("y", &CVector::y)
		.property("z", &CVector::z)
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

	SLB::setGlobal<CLogicManager*>(L, &get(), "logicManager");

	luaL_dofile(L, "test.lua");

	execute("onInit();");
}

void CLogicManager::loadScene(std::string scene_name) {
	CApp::get().loadScene(scene_name);
}

void CLogicManager::onSceneLoad(std::string scene_name) {
	execute("onSceneLoad_" + scene_name + "();");
}

CBot CLogicManager::getBot(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid bot name: " + name + "\")");

	return CBot(entity);
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

CHingeJoint CLogicManager::getHingeJoint(std::string name) {
	CHandle entity = CEntityManager::get().getByName(name.c_str());
	if (!entity.isValid())
		execute("error(\"Invalid prismatic joint name: " + name + "\")");

	return CHingeJoint(entity);
}

void CLogicManager::print(std::string text) {
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