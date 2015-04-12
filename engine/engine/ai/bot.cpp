#include "mcv_platform.h"
#include "bot.h"
#include "components\comp_life.h"
#include "components\comp_transform.h"
#include "logic_manager.h"

CBot::CBot(CHandle the_entity) { entity = the_entity; }
CBot::~CBot() {}

// LUA
void CBot::kill() {
	TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
	if (life)
		life->life = 0;
}

void CBot::teleport(CVector pos) {
	TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
	if (transform)
		transform->teleport(XMVectorSet(pos.x, pos.y, pos.z, 0));
}

void CBot::teleport(float x, float y, float z) {
	teleport(CVector(x, y, z));
}

CVector CBot::getPos() {
	TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
	if (transform)
		return CVector(transform->position);
	else
		return CVector(0, 0, 0);
}

float CBot::getLife() {
	TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
	if (life)
		return life->life;
	else
		return 0;
}

void CBot::setLife(float value) {
	TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
	if (life)
		life->life = value;
}

void CBot::hurt(float value) {
	TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
	if (life)
		life->life -= value;
}

void CBot::help() {
	CLogicManager::get().execute("logicManager:print(tostring(LogicManager))");
}