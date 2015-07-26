#include "mcv_platform.h"
#include "aifsmcontroller.h"

aifsmcontroller::aifsmcontroller()
{
	statemap = new map<string, statehandler>();
	on_enter = true;
	state_changed = true;
	state_time = 0.f;
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
	state_time += elapsed;
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
	// if the state we wish to jump to does not exist. we abort
	XASSERT(statemap->find(newstate) != statemap->end(), "Trying to change to unexisting state %s", newstate.c_str());
	
	state = newstate;
	state_time = 0.f;
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

void aifsmcontroller::setRol(int r){}

void aifsmcontroller::setAttackerSlot(int s){}

int aifsmcontroller::getAttackerSlot(){ return 0; }

int aifsmcontroller::getRol(){ return 0; }

float aifsmcontroller::getDistanceToPlayer(){ return 0.f; }

int aifsmcontroller::getNearestSlot(bool free_north, bool free_east, bool free_west){ return 0; }

bool aifsmcontroller::isAngry(){ return false; }

int aifsmcontroller::getIndRecastAABB(){ return 0; }

void aifsmcontroller::setActive(bool act){}

void aifsmcontroller::setType(aicontroller::types kind){};

aicontroller::types aifsmcontroller::getType(){ return aicontroller::types::untyped; };
