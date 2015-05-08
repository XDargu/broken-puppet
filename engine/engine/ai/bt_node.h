#ifndef _BT_NODE_H_
#define _BT_NODE_H_

#include "mcv_platform.h"

using namespace std;

class bt;

class btnode;

#define RANDOM 1
#define SEQUENCE 2
#define PRIORITY 3
#define ACTION 4
#define DECORATOR 5

#define STAY 0
#define LEAVE 1

enum subType{ DEC_LIMIT_TIMES, CONDITIONAL_NODE, TIMER_NODE, LOOP_UNTIL };

enum typeInterrupAllowed{INTERNAL, EXTERNAL, BOTH, NONE};


struct decorator{
	virtual void init(subType t) = 0;
	virtual void updateCondition(bool t) = 0;
	virtual bool getCondition() = 0;
	virtual void initCondition(float n) = 0;
	virtual float getMainInfo() = 0;
};


struct decorator_limiter:virtual decorator{
	subType decSubType;
	float numIterations;
	float maxIterations;
	void init(subType t){ numIterations = 0; decSubType = t; };

	void updateCondition(bool t){
		numIterations++;
	};

	bool getCondition(){ 
		if (numIterations >= maxIterations){
			return true;
		}
		return false;
	};

	void initCondition(float n){
		maxIterations = n;
	}

	float getMainInfo(){
		return 0.f;
	};
};

struct decorator_conditional :virtual decorator{
	subType decSubType;
	bool condition;

	void init(subType t){ 
		bool aux = false;  
		condition = aux; 
		decSubType = t; 
	};

	void updateCondition(bool t){ 
		condition = t; 
	};

	bool getCondition(){ 
		return condition;
	};

	void initCondition(float n){};

	float getMainInfo(){
		return 0.f;
	};
};

struct decorator_timer :virtual decorator{
	subType decSubType;
	float max_time;
	bool condition;

	void init(subType t){
		condition = false;
		decSubType = t;
	};

	void updateCondition(bool t){
		//time += CApp::get().delta_time;
		/*if (time >= max_time) {
			
			condition = true;
			time = 0;
		}else{
			condition = false;
		}*/
		condition = t;
	};

	bool getCondition(){
		return condition;
	};

	void initCondition(float n){
		condition = false;
		max_time = n;
	};

	float getMainInfo(){
		return max_time;
	};
};

struct decorator_loop :virtual decorator{
	subType decSubType;
	bool condition;

	void init(subType t){
		condition = true;
		decSubType = t;
	};

	void updateCondition(bool t){
		condition = t;
	};

	bool getCondition(){
		return condition;
	};

	void initCondition(float n){
		condition = true;
	};

	float getMainInfo(){
		return 0.f;
	};
};

class btnode
{
	string name;
	int type;
	int weight;

	vector<btnode *>children;
	btnode *parent;
	btnode *right;

	subType decSubType;

	typeInterrupAllowed typeInter;

public:
	decorator* decorator_node;
	btnode(string);
	void create(string);
	bool isRoot();
	void setParent(btnode *);
	void setRight(btnode *);
	void addChild(btnode *);
	void setType(int);
	void recalc(bt *);
	string getName();
	int getNodeWeight();
	void setNodeWeight(int w);
	void setDecSubType(subType);
	void setTypeInter(typeInterrupAllowed);
	typeInterrupAllowed getTypeInter();
};

#endif

