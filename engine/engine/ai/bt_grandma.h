#ifndef _BT_GRANDMA_INC
#define _BT_GRANDMA_INC

#include "mcv_platform.h"
#include "behaviour_trees.h"
#include "physics_manager.h"
#include "components\comp_transform.h"
#include "../components/comp_sensor_needles.h"
#include "../components/comp_sensor_tied.h"
#include "../components/comp_player_position_sensor.h"

class bt_grandma : public bt
{
private:

	float find_path_time;
	float last_time_player_saw;
	float time_searching_player;
	float sensor_acum;
	float sensor_const;
	float distance_old;
	float distance_new;
	float time_searching_needle;
	XMVECTOR previous_point_search;
	XMVECTOR center;
	XMVECTOR last_point_player_saw;
	XMVECTOR rand_point;
	XMVECTOR slot_position;
	CHandle character_controller;
	CHandle player;
	int ind_path;
	int ind_recast_aabb;
	physx::PxVec3 last_look_direction;
	physx::PxVec3 mov_direction;
	physx::PxVec3 look_direction;
	std::vector<XMVECTOR> path;
	XMVECTOR wander_target;
	XMVECTOR player_detected_pos;
	bool jump;
	bool tied_event;
	bool event_detected;
	bool needle_to_take;
	bool needle_is_valid;
	bool can_reach_needle;
	bool is_needle_tied;
	bool too_close_attack;
	bool is_angry;
	bool have_to_warcry;
	bool is_ragdoll;
	bool hurt_event;
	bool see_player;
	bool initial_attack;
	bool active;
	bool cut_animation_done;
	bool take_animation_done;
	//bool player_touch;
	int last_anim_id;

	bool player_previously_lost;
	bool player_cant_reach;

	bool null_node;
	bool player_out_navMesh;

	CHandle ropeRef;
	CHandle m_sensor;
	CHandle own_transform;
	CHandle player_pos_sensor;
	CHandle tied_sensor;
	CHandle player_transform;
	CHandle enemy_skeleton;
	CHandle enemy_ragdoll;
	CHandle enemy_rigid;

	
	enum role{ UNASIGNATED, ATTACKER, TAUNTER };
	role rol;
	enum attacker_slots{ NO_SLOT, NORTH, EAST, WEST};
	attacker_slots slot;

public:
	void create(string);

	//Se mantiene en modo ragdoll durante un tiempo
	int actionRagdoll();
	//Ejecuta la animacin de levantarse
	int actionWakeUp();
	//Corta todas las cuerdas a la que est atada
	int actionCutOwn();
	//Espera unos segundos
	int actionWaitSec();
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
	//plays the looking for player animation
	int actionLookingFor();
	// Plays the animation of looking for
	int actionLookingForPlayer();
	//Takes a rol, attacker or taunter and a poisition to go
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
	//Check the time for looking the player
	int conditionLook_time();
	//Search random point around the last place player wasnotify saw
	int actionSearchArroundLastPoint();
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
	void tiedSensor();
	void hurtSensor(float damage);
	void PlayerFoundSensor();
	//void PlayerTouchSensor(bool touch);
	void WarWarningSensor(XMVECTOR player_position);
	void update(float elapsed);
	//unsigned getId();
	//void setId(unsigned int id);

	bool player_viewed_sensor;
	unsigned int lastNumNeedlesViewed;
	unsigned int currentNumNeedlesViewed;

	CHandle getPlayerTransform();
	bool findPlayer();
	bool isAngry();
	void setRol(int r);
	int getRol();
	void setAttackerSlot(int s);
	int getAttackerSlot();
	float getDistanceToPlayer();
	int getNearestSlot(bool free_north, bool free_east, bool free_west);

	void drawdebug();

	void stopAllAnimations();
	void playAnimationIfNotPlaying(int id);
	void stopAnimation(int id);
	float getAnimationDuration(int id);
	void resetTimeAnimation();

	void setActive(bool act);

	void setIndRecastAABB(int ind);
	int getIndRecastAABB();

	void resetBot();
	void stopMovement();
	void BeAngry();

	void setRagdoll() { is_ragdoll = true; setCurrent(NULL); }
};

#endif