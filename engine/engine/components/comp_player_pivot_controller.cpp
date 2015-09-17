#include "mcv_platform.h"
#include "comp_player_pivot_controller.h"
#include "comp_transform.h"
#include "entity_manager.h"
#include "io\iostatus.h"

void TCompPlayerPivotController::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	rotation_velocity = deg2rad(atts.getFloat("rotationVelocity", 90));
}

void TCompPlayerPivotController::init() {
	CEntity* e_player = CEntityManager::get().getByName("Player");

	assert(e_player || fatal("TCompPlayerPivotController requieres a player entity"));

	player_transform = e_player->get<TCompTransform>();
	TCompTransform* player_trans = (TCompTransform*)player_transform;

	assert(player_trans || fatal("TCompPlayerPivotController requieres a player entity with a TTransform component"));

	CEntity* e = CHandle(this).getOwner();
	m_transform = e->get<TCompTransform>();
	TCompTransform* transform = (TCompTransform*)m_transform;

	// Move the pivot to the player position
	transform->position = player_trans->position;

	assert(transform || fatal("TCompPlayerPivotController requieres a TTransform component"));
}

void TCompPlayerPivotController::update(float elapsed) {
	TCompTransform* player_trans = (TCompTransform*)player_transform;
	TCompTransform* transform = (TCompTransform*)m_transform;

	// Move the pivot to the player position
	transform->position = player_trans->position;

	CIOStatus &io = CIOStatus::get();

	CIOStatus::TMouse mouse = io.getMouse();
	XMVECTOR rot = XMQuaternionRotationAxis(transform->getUp(), -rotation_velocity * mouse.dx * 0.01f /* elapsed*/ * io.mouse_sensibility);

	float lerp_value = 0.45f;
	transform->rotation = XMQuaternionSlerp(transform->rotation, XMQuaternionMultiply(transform->rotation, rot), clamp(lerp_value, 0, 1));

	//transform->rotation = XMQuaternionMultiply(transform->rotation, rot);
}

void TCompPlayerPivotController::pointAt(XMVECTOR target) {
	TCompTransform* transform = (TCompTransform*)m_transform;

	transform->lookAt(target, transform->getUp());

	// Get only the Y axis rotation from yaw
	float player_pivot_yaw = getYawFromVector(transform->getFront());
	XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), player_pivot_yaw);

	// Set the new rotation
	transform->rotation = player_pivot_rot;
}