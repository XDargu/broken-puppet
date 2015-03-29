#include "mcv_platform.h"
#include "aimanager.h"
//#include "aicontroller.h"
#include "../components/all_components.h"

using namespace DirectX;

static aimanager the_aimanager;
std::vector<aicontroller*> bots;

aimanager::aimanager()
{
	last_id = 0;
}


aimanager::~aimanager()
{
}

aimanager& aimanager::get(){
	return the_aimanager;
}

aicontroller* aimanager::getAI(unsigned int id){
	for (auto & element : bots) {
		if (element->getInt() == id)
			return element;
	}
	//Meter un xassert
	return nullptr;
}

aicontroller* aimanager::getAI(XMVECTOR pos, float radius){
	fsm_basic_enemy* initialization = new fsm_basic_enemy;
	aicontroller* bot = initialization;
	for (auto & element : bots) {
		TCompTransform* e_transform = element->getTransform();
		float aux_distance = V3DISTANCE(e_transform->position, pos);
		if (aux_distance <= radius){
			return bot;
		}
	}
	return nullptr;
}

aicontroller* aimanager::getClosest(XMVECTOR pos){
	fsm_basic_enemy* initialization = new fsm_basic_enemy;
	aicontroller* closest_bot = initialization;
	bool first = false;
	float closest_distance = 0.f;
	for (auto & element : bots) {
		TCompTransform* e_transform = element->getTransform();
		float aux_distance = V3DISTANCE(e_transform->position, pos);
		if ((aux_distance < closest_distance) || (first == false)){
			closest_distance = aux_distance;
			closest_bot = element;
			first = true;
		}
	}
	return closest_bot;
}

void aimanager::addBot(aicontroller* bot){
	bot->setId(last_id);
	bots.push_back(bot);
	last_id++;
}

void aimanager::removeBot(unsigned int id){
	aicontroller* bot_to_remove = getAI(id);
	std::vector<aicontroller*>::iterator iter = bots.begin();
	while (iter != bots.end())
	{
		if (*iter == bot_to_remove)
		{
			iter = bots.erase(iter);
			return;
		}
		else
		{
			++iter;
		}
	}
}