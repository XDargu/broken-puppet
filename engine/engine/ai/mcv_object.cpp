#include "mcv_platform.h"
#include "mcv_object.h"
#include "components\comp_transform.h"
#include "components\comp_render.h"
#include "components\comp_rigid_body.h"
#include "components\comp_ai_boss.h"
#include "components\comp_ai_substitute.h"
#include "rigid_animation.h"
#include "logic_manager.h"


CMCVObject::CMCVObject(CHandle the_entity) { entity = the_entity; }
CMCVObject::~CMCVObject() {}

CVector CMCVObject::getPosition() {
	if (!entity.isValid())
		return CVector(0, 0, 0);
	TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
	if (transform)
		return CVector(transform->position);
	else
		return CVector(0, 0, 0);
}

void CMCVObject::setPosition(CVector newPos) {
	if (!entity.isValid())
		return;
	TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
	if (transform)
		transform->teleport(XMVectorSet(newPos.x, newPos.y, newPos.z, 0));
}

void CMCVObject::moveToPosition(CVector position, float speed) {
	if (!entity.isValid())
		return;
	TCompTransform* transform = ((CEntity*)entity)->get<TCompTransform>();
	if (transform) {
		CRigidAnimation r_anim(transform);

		// Speed = Space / Time
		// Time = Space / Speed
		// Space = Vector length
		// Speed given
		// TODO: Make animations speed dependant

		XMVECTOR newPos = XMVectorSet(position.x, position.y, position.z, 0);
		float space = V3DISTANCE(transform->position, newPos);
		float time = space / speed;
		if (time == 0)
			time = 0.1f;

		r_anim.addKeyframe(newPos, transform->rotation, time);

		CLogicManager::get().addRigidAnimation(
			r_anim
			);
	}
}

void CMCVObject::setEmissive(bool active) {
	if (!entity.isValid())
		return;
	TCompRender* m_render = ((CEntity*)entity)->get<TCompRender>();
	if (m_render) {
		m_render->emissive_on = active;
	}
}

void CMCVObject::applyForce(CVector force) {
	if (!entity.isValid())
		return;
	TCompRigidBody* rigid = ((CEntity*)entity)->get<TCompRigidBody>();
	if (rigid) {
		if (!rigid->isKinematic()) {
			rigid->rigidBody->addForce(PxVec3(force.x, force.y, force.z), PxForceMode::eVELOCITY_CHANGE);
		}
	}
}

void CMCVObject::riseUpBoss() {
	if (!entity.isValid())
		return;
	TCompAiBoss* boss = ((CEntity*)entity)->get<TCompAiBoss>();
	if (boss) {
		boss->initBoss();
	}
}


CMCVObject CMCVObject::firstBombBoss() {
	if (!entity.isValid())
		return CMCVObject(CHandle());
	TCompAiBoss* boss = ((CEntity*)entity)->get<TCompAiBoss>();
	if (boss) {
		return CMCVObject(boss->objToStun());
	}

	return CMCVObject(CHandle());
}

void CMCVObject::hitSubstitute() {
	if (!entity.isValid())
		return;
	TCompSubstituteBoss* substitute = ((CEntity*)entity)->get<TCompSubstituteBoss>();
	if (substitute) {
		substitute->hitSubstitute();
	}
}


void CMCVObject::initLittleTalk() {
	if (!entity.isValid())
		return;
	TCompSubstituteBoss* substitute = ((CEntity*)entity)->get<TCompSubstituteBoss>();
	if (substitute) {
		substitute->initLittleTalk();
	}
}

void CMCVObject::initialRain(int amount) {
	if (!entity.isValid())
		return;
	TCompAiBoss* boss = ((CEntity*)entity)->get<TCompAiBoss>();
	if (boss) {
		boss->initialRain(amount);
	}
}
void CMCVObject::setRender(bool active){
	if (!entity.isValid())
		return;
	TCompRender* render = ((CEntity*)entity)->get<TCompRender>();
	if (render) {
		render->active = active;
	}
}