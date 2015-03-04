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

#endif
