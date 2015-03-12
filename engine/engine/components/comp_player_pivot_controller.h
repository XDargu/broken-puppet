#ifndef INC_COMP_PLAYER_PIVOT_CONTROLLER_H_
#define INC_COMP_PLAYER_PIVOT_CONTROLLER_H_

#include "base_component.h"
#include "comp_transform.h"
#include "../io/iostatus.h"

struct TCompPlayerPivotController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_transform;
public:

	float rotation_velocity;

	TCompPlayerPivotController() : rotation_velocity(deg2rad(90.0f)) {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		rotation_velocity = deg2rad(atts.getFloat("rotationVelocity", 90));
	}

	void init() {
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

	void update(float elapsed) {
		TCompTransform* player_trans = (TCompTransform*)player_transform;
		TCompTransform* transform = (TCompTransform*)m_transform;

		// Move the pivot to the player position
		transform->position = player_trans->position;

		CIOStatus &io = CIOStatus::get();

		CIOStatus::TMouse mouse = io.getMouse();
		XMVECTOR rot = XMQuaternionRotationAxis(transform->getUp(), -rotation_velocity * mouse.dx * elapsed);
		transform->rotation = XMQuaternionMultiply(transform->rotation, rot);
	}


	std::string toString() {
		return "Player pivot controller";
	}
};

#endif