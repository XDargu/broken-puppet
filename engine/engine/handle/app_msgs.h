#ifndef INC_APP_MSGS_H_
#define INC_APP_MSGS_H_

// Add here all the msgs of your game

struct TMsgExplosion {
	XMVECTOR source;
	float    radius;
	float    damage;
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
	TActorHit(CEntity* awho, float hurt) :who(awho), damage(hurt) { }
	DECLARE_MSG_ID();
};

struct TMsgAttackDamage {
	CEntity* who;
	float damage;
	TMsgAttackDamage(CEntity* awho, float hurt) :who(awho), damage(hurt) { }
	DECLARE_MSG_ID();
};

struct TVictoryCondition {
	CEntity* whoEnter;
	TVictoryCondition(CEntity* awho):whoEnter(awho) { }
	DECLARE_MSG_ID();
};

#endif
