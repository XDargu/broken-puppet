#ifndef _FSM_PLAYER_TORSO_H_
#define _FSM_PLAYER_TORSO_H_

#include "aifsmcontroller.h"
#include "physics_manager.h"
#include "handle\handle.h"
#include "rope_manager.h"

class FSMPlayerLegs;

class FSMPlayerTorso : public aifsmcontroller
{
private:
	//deque<CHandle> strings;

	int entitycount;
	PxRigidActor* first_actor;
	PxVec3 first_position;
	PxVec3 first_offset;
	CHandle first_needle;
	CHandle comp_skeleton;
	CHandle current_rope_entity;
	bool first_throw;

	PxVec3 standard_camera_offset;
public:

	bool up_animation;
	bool can_move;
	unsigned int max_num_string;

	CHandle comp_transform;
	CHandle camera_entity;

	FSMPlayerLegs* legs;

	FSMPlayerTorso();
	~FSMPlayerTorso();

	void Init();

	void ThrowString(float elapsed);
	void PullString(float elapsed);
	void GrabString(float elapsed);
	void Inactive(float elapsed);

	void ProcessHit(float elapsed);

};

#endif