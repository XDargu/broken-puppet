#ifndef _AICONTROLLER_H
#define _AICONTROLLER_H

// ai controllers using maps to function pointers for easy access and scalability. 

// we put this here so you can assert from all controllers without including everywhere
#include <assert.h>	
#include <string>
#include <map>
#include "entity.h"

using namespace std;

// states are a map to member function pointers, to 
// be defined on a derived class. 
class aicontroller;

typedef void (aicontroller::*statehandler)(float deltaTime);

class aicontroller
{
	string state;
	// the states, as maps to functions
	map<string, statehandler>statemap;

public:

	CEntityOld* entity;
	void SetEntity(CEntityOld* the_entity);
	void ChangeState(string);	// state we wish to go to
	string GetState();	// returns state name
	virtual void Init();	// resets the controller
	void Recalc(float deltaTime);	// recompute behaviour
	void AddState(string, statehandler);
};

#endif