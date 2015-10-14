#ifndef INC_COMP_AI_BOSS_H_
#define INC_COMP_AI_BOSS_H_

#include "base_component.h"
#include "physics_manager.h"
#include "../ai/fsm_boss.h"

struct TCompAiBoss : TBaseComponent {
private:
	fsm_boss* m_fsm_boss;

	CHandle substitute;
	CHandle mPlayer;
	CHandle player_trans;
	CHandle mBoss;
	CHandle boss_trans;

	CHandle comp_skeleton;

	bool hitchs_opened;
	bool heart_opened;

	CHandle R_hitch;
	PxFixedJoint* R_hitch_joint;
	CHandle R_hitch_light;
	

	XMVECTOR original_ligh_color;
	bool openLight(float elapsed);
	void closeLight();

	CHandle L_hitch;
	PxFixedJoint* L_hitch_joint;
	CHandle L_hitch_light;

	CHandle H_hitch;
	PxFixedJoint* H_hitch_joint;
	CHandle H_hitch_light;

	PxVec3 point_to_go;
	PxVec3 point_offset;
	float distance_to_point;
	float force;
	float proximity_distance;

	bool activate;
	bool shoot;
	bool debris_debris;
	bool attack2Active;
	float attack2Time;

	float debris_creation_delay;
	int debris_created;
	int move_things;

	bool is_death;
	bool death_time;
	bool calculate_break_point;
	XMVECTOR break_point;

	void brokeHeart();

	float boss_creation_delay;
	XMVECTOR last_created_pos;
	XMVECTOR last_random_pos;
	
	bool safe_raining;
	bool safeRain(float elapsed, int debris_amount);
	int safe_debris_amount;

public:

	TCompAiBoss();
	~TCompAiBoss();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void fixedUpdate(float elapsed);
	void update(float elapsed);

	void breakHitch(CHandle m_hitch);	
	void stun();
	void openHeart( float elapsed);

	bool can_break_hitch;

	void initBoss();


	// Publicar en LUA
	CHandle objToStun();		
	void initialRain(int debris_amount);

};


#endif