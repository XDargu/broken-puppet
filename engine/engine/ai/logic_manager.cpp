#include "mcv_platform.h"
#include "logic_manager.h"
#include "components\comp_trigger.h"

static CLogicManager logic_manager;

CLogicManager& CLogicManager::get() {
	return logic_manager;
}

CLogicManager::CLogicManager() {}

CLogicManager::~CLogicManager() {}

std::vector<std::string> timers_to_delete;

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