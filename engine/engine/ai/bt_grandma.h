#ifndef _BT_GRANDMA_INC
#define _BT_GRANDMA_INC

#include "mcv_platform.h"
#include "behaviour_trees.h"
#include "physics_manager.h"
#include "components\comp_transform.h"

class bt_grandma : public bt
{
private:
	float radius;
	XMVECTOR center;
	XMVECTOR rand_point;
	CHandle character_controller;
	CHandle player;
	int ind_path;
	physx::PxVec3 last_look_direction;
	physx::PxVec3 mov_direction;
	physx::PxVec3 look_direction;
	std::vector<XMVECTOR> path;
	XMVECTOR wander_target;
	bool jump;
public:
	void create(string);

	//Se mantiene en modo ragdoll durante un tiempo
	int actionRagdoll();
	//Ejecuta la animacin de levantarse
	int actionWakeUp();
	//Corta todas las cuerdas a la que est atada
	int actionCutOwn();
	//
	int actionLeave();
	//Attack to the player when he is too close
	int actionTooCloseAttack();
	//Go to the needle position (leave if cant reach)
	int actionChaseNeedlePosition();
	//Select the priority needle
	int actionSelectNeedleToTake();
	//Cut the needles rope
	int actionCutRope();
	//Take the needle
	int actionTakeNeedle();
	//Select the idle and play it
	int actionIdle();
	//Select a point to go
	int actionSearchPoint();
	//Chase the selected point
	int actionWander();
	//Makes a warcry
	int actionWarcry();
	//Alert to the other grandma about the player
	int actionPlayerAlert();
	//Leave the angry state, go to peacefull
	int actionCalmDown();
	//look the player around the his last point
	int actionLookAround();
	//Takes a roll, attacker or taunter and a poisition to go
	int actionSelectRole();
	//Go to his position
	int actionChaseRoleDistance();
	//First attack
	int actionInitialAttack();
	//Move step by step to the roll position (leave on reach or lost)
	int actionSituate();
	//
	int actionNormalAttack();
	//Play a Idle war animation
	int actionIdleWar();
	//Play a taunter routine
	int actionTaunter();
	//Calculate if hurts or ragdoll, if ragdoll then clean all events (los events solo tocan su flag, excepto el ragdoll)
	int actionHurtEvent();
	//
	int actionNeedleAppearsEvent();
	//
	int actionTiedEvent();
	//Keeps in falling state till ti
	int actionFallingEvent();
	//
	int actionGetAngry();
	//Puts are_events var to false
	int actionNoevents();

	//
	int conditionTied();
	//
	int conditionis_ragdoll();
	//
	int conditionis_grounded();
	//
	int conditiontrue();
	//
	int conditionis_angry();
	//Check if the player is close enought for an annoying attack
	int conditiontoo_close_attack();
	//Check if there is a needle to take
	int conditionneedle_to_take();
	//
	int conditionis_needle_tied();
	//Check if is necesary a warcry
	int conditionhave_to_warcry();
	//Check if there player is not visible for any grandma (and reach the last position)
	int conditionplayer_lost();
	//check if the player is visible
	int conditionsee_player();
	//Check the look for timer
	int conditionLook_for_timeout();
	//Check if the role is attacker and is close enought
	int conditionis_attacker();
	//
	int conditionnormal_attack();
	//Init on false
	int conditionare_events();
	//Check if is a hurt event
	int conditionhurt_event();
	//Check if is a falling event
	int conditionfalling_event();
	//Check if is a tied event
	int conditiontied_event();
	//Check if can reach the selected needle
	int conditioncan_reach_needle();
	//Check if the role is taunter and is close enought
	int conditionis_taunter();
	//
	int conditioninitial_attack();
	//Check if it is too far from the target position
	int conditionfar_from_target_pos();

	bool trueEveryXSeconds(float time);
	void chasePoint(TCompTransform* own_position, XMVECTOR chase_point);

	/*Sensores*/

	void playerViewedSensor();
	void needleViewedSensor();	
	void update(float elapsed);

	bool player_viewed_sensor;
	unsigned int lastNumNeedlesViewed;
	unsigned int currentNumNeedlesViewed;
};

#endif