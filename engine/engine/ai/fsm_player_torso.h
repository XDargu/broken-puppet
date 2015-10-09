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
	CHandle GNLogic;

	XMVECTOR standard_camera_offset;
	XMVECTOR golden_needle_point;
	bool looking_at_pointer;
public:

	bool first_throw;
	bool up_animation;
	bool can_move;
	unsigned int max_num_string;

	bool can_throw;
	bool can_cancel;
	bool can_tense;
	bool can_pull;

	CHandle comp_transform;
	CHandle camera_entity;

	FSMPlayerLegs* legs;

	FSMPlayerTorso();
	~FSMPlayerTorso();

	void Init();

	void ThrowString(float elapsed);
	void ThrowGoldenNeedle(float elapsed);
	void PullString(float elapsed);
	void GrabString(float elapsed);
	void Inactive(float elapsed);

	void ProcessHit(float elapsed);

	void getThrowingData(PxActor* &hit_actor, PxVec3 &actor_position, PxVec3 &actor_normal);
	bool canThrow();

	void CancelGrabString();

};

#endif