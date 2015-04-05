#include "mcv_platform.h"
#include "logic_manager.h"
#include "components\comp_trigger.h"
#include "components\comp_transform.h"

static CLogicManager logic_manager;

CLogicManager& CLogicManager::get() {
	return logic_manager;
}

CLogicManager::CLogicManager() {}

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
			XDEBUG("Timer %s has reached the time limit", it.first.c_str());
			timers_to_delete.push_back(it.first);
		}
	};

	// Remove timers that has reached the limit
	for (auto& it : timers_to_delete) {
		timers.erase(it);
	}

	// Clear the delete vector, if needed
	if (timers_to_delete.size() > 0)
		timers_to_delete.clear();


	CErrorContext ce2("Updating keyframes", "");
	// Update the keyframes
	for (auto& it : current_keyframes) {
		// Check if the keyframe has reached the limit
		if (it.update(elapsed)) {
			XDEBUG("Keyframehas reached the time limit");
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
}

void CLogicManager::setTimer(std::string the_name, float time) {
	SET_ERROR_CONTEXT("Setting a timer", the_name.c_str());
	XDEBUG("Timer %s is set to %f seconds", the_name.c_str(), time);
	timers[the_name] = CTimer(time);
}

void CLogicManager::registerTrigger(CHandle trigger) {
	triggers.push_back(trigger);
}

void CLogicManager::unregisterTrigger(CHandle trigger) {
	auto it = std::find(triggers.begin(), triggers.end(), trigger);
	triggers.erase(it);
}

void CLogicManager::addKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time) {
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