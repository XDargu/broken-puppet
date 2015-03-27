#include "mcv_platform.h"
#include "aifsmcontroller.h"


aifsmcontroller::aifsmcontroller()
{
	statemap = new map<string, statehandler>();
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
	(this->*(*statemap)[state])(elapsed);
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
	//Debería delegar en la clase hija
}

unsigned int aifsmcontroller::getInt(){
	//Debería delegar en la clase hija
	return 0;
}

CHandle aifsmcontroller::getTransform(){
	//Debería delegar en la clase hija
	CHandle aux;
	return aux;
}
