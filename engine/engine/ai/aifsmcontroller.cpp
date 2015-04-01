#include "mcv_platform.h"
#include "aifsmcontroller.h"

aifsmcontroller::aifsmcontroller()
{
	statemap = new map<string, statehandler>();
	on_enter = true;
	state_changed = true;
}

aifsmcontroller::~aifsmcontroller()
{
	delete statemap;
	statemap = nullptr;
}

void aifsmcontroller::create(string)
{
}


void aifsmcontroller::update(float elapsed)
{
	// this is a trusted jump as we've tested for coherence in ChangeState
	state_changed = false;
	(this->*(*statemap)[state])(elapsed);
	if (!state_changed)
		on_enter = false;
}

CHandle aifsmcontroller::GetEntity(){
	return entity;
}

void aifsmcontroller::SetEntity(CHandle the_entity)
{
	entity = the_entity;
}

void aifsmcontroller::ChangeState(std::string newstate)
{
	// try to find a state with the suitable name
	if (statemap->find(newstate) == statemap->end())
	{
		// the state we wish to jump to does not exist. we abort
		exit(-1);
	}
	state = newstate;
	state_changed = true;
	on_enter = true;
}

std::string aifsmcontroller::getCurrentNode()
{
	return state;
}

void aifsmcontroller::AddState(std::string name, statehandler sh)
{

	// try to find a state with the suitable name
	if (statemap->find(name) != statemap->end())
	{
		// the state we wish to jump to does exist. we abort
		exit(-1);
	}
	(*statemap)[name] = sh;
}

void aifsmcontroller::recalc(float deltaTime){
	// this is a trusted jump as we've tested for coherence in ChangeState
}

void aifsmcontroller::setId(unsigned int id){
	//Deber�a delegar en la clase hija
}

unsigned int aifsmcontroller::getInt(){
	//Deber�a delegar en la clase hija
	return 0;
}

CHandle aifsmcontroller::getTransform(){
	//Deber�a delegar en la clase hija
	CHandle aux;
	return aux;
}
