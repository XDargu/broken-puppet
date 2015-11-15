#ifndef INC_APP_MSGS_H_
#define INC_APP_MSGS_H_

// Add here all the msgs of your game
struct TMsgExplosion {
	XMVECTOR source;
	float    radius;
	float    damage;
	TMsgExplosion(XMVECTOR aSource, float aRadius, float aDamage) : source(aSource), radius(aRadius), damage(aDamage) { }
	DECLARE_MSG_ID();
};
struct TMsgOnDetonate {
	float    impact_force;
	bool is_boss;
	TMsgOnDetonate(float m_impact_force, bool m_is_boss) : impact_force(m_impact_force), is_boss(m_is_boss){}
	DECLARE_MSG_ID();
};


struct TMsgDied {
	int     who;
	TMsgDied( int awho ) : who (awho ) { }
	DECLARE_MSG_ID();
};

struct TMsgImpact {
	int     who;
	TMsgImpact(int awho) : who(awho) { }
	DECLARE_MSG_ID();
};

struct TGroundHit {
	CEntity* who;
	float vel;
	TGroundHit(CEntity* awho, float velocity):who(awho), vel(velocity) { }
	DECLARE_MSG_ID();
};

struct TActorHit {
	CEntity* who;
	float damage;
	bool is_boss;
	TActorHit(CEntity* awho, float hurt, bool ais_boss) :who(awho), damage(hurt), is_boss(ais_boss){ }
	DECLARE_MSG_ID();
};

struct TWarWarning {
	CEntity* who;
	XMVECTOR player_position;
	TWarWarning(CEntity* awho, XMVECTOR player_pos) :who(awho), player_position(player_pos) { }
	DECLARE_MSG_ID();
};

struct TPlayerFound {
	TPlayerFound() { }
	DECLARE_MSG_ID();
};

struct TPlayerTouch {
	CEntity* who;
	bool touch;
	TPlayerTouch(CEntity* awho, bool touched) :who(awho), touch(touched){}
	DECLARE_MSG_ID();
};

/*struct TMsgEnemyTied {
	CEntity* rope;
	TMsgEnemyTied(CHandle arope) :rope(arope) { }
	DECLARE_MSG_ID();
};*/

struct TMsgAttackDamage {
	CEntity* who;
	float damage;
	TMsgAttackDamage(CEntity* awho, float hurt) :who(awho), damage(hurt) { }
	DECLARE_MSG_ID();
};

struct TMsgRopeTensed {
	float sqrRopeDistance;
	TMsgRopeTensed(float adistance) : sqrRopeDistance(adistance) { }
	DECLARE_MSG_ID();
};

struct TMsgGenerateBomb {
	TMsgGenerateBomb(){ }
	DECLARE_MSG_ID();
};

struct TMsgNeedleHit {
	CEntity* who;
	TMsgNeedleHit(CEntity* awho) : who(awho) { }
	DECLARE_MSG_ID();
};

struct TVictoryCondition {
	CEntity* whoEnter;
	TVictoryCondition(CEntity* awho):whoEnter(awho) { }
	DECLARE_MSG_ID();
};

#endif
