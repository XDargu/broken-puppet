#include "mcv_platform.h"
#include "moving_platform.h"
#include "components\comp_platform_path.h"

CMovingPlatform::CMovingPlatform(CHandle the_entity) { entity = the_entity; }
CMovingPlatform::~CMovingPlatform() {}

// LUA

void CMovingPlatform::start(){
	if (!entity.isValid())
		return;
	TCompPlatformPath* platform = ((CEntity*)entity)->get<TCompPlatformPath>();
	if (platform)
		platform->startMoving();
}

void CMovingPlatform::stop() {
	if (!entity.isValid())
		return;
	TCompPlatformPath* platform = ((CEntity*)entity)->get<TCompPlatformPath>();
	if (platform)
		platform->stopMovement();
}