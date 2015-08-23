#ifndef _AIFSMCONTROLLER_H
#define _AIFSMCONTROLLER_H

// ai controllers using maps to function pointers for easy access and scalability. 

// we put this here so you can assert from all controllers without including everywhere
#include <assert.h>	
#include <string>
#include "aicontroller.h"
#include <map>
#include "../handle/handle.h"

using namespace std;

// states are a map to member function pointers, to 
// be defined on a derived class. 
class aifsmcontroller;

typedef void (aifsmcontroller::*statehandler)(float deltaTime);

class aifsmcontroller :public aicontroller
{
	string state;
	// the states, as maps to functions
	map<string, statehandler>* statemap;
	bool state_changed;
protected:
	bool on_enter;
	float state_time;
public:

	aifsmcontroller();
	~aifsmcontroller();

	CHandle entity;
	void SetEntity(CHandle the_entity);
	CHandle GetEntity();
	virtual void setId(unsigned int id);
	virtual unsigned int getInt();
	void ChangeState(string);	// state we wish to go to
	string getCurrentNode();	// returns state name
	virtual void create(string);	// resets the controller
	void recalc(float deltaTime);	// recompute behaviour
	void AddState(string, statehandler);
	void update(float elapsed);
	virtual CHandle getTransform();

	//---------------------------------------------
	void setRol(int r);
	int getRol();
	void setAttackerSlot(int s);
	int getAttackerSlot();
	float getDistanceToPlayer();
	int getNearestSlot(bool free_north, bool free_east, bool free_west);
	bool isAngry();
	int getIndRecastAABB();
	void setActive(bool act);
	std::string getState() { return state; }
	//---------------------------------------------
};

#endif

