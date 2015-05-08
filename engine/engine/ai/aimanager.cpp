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
	CErrorContext ec("Getting bot", "aimanager");
	XASSERT(id, "bot doesn't exists");
	return nullptr;
}

//Para evitar el retorno de un vector de punteros de aicontrollers pasamos la referencia del vector 
//donde queremos que se almacenen los bots que esten en el rango dado. El metodo se encargara
//simplemente de rellenarlo.
void aimanager::getAI(XMVECTOR pos, float radius, std::vector<aicontroller*>& botsInRange){
	fsm_basic_enemy* initialization = new fsm_basic_enemy;
	aicontroller* bot = initialization;
	for (auto & element : bots) {
		TCompTransform* e_transform = element->getTransform();
		float aux_distance = V3DISTANCE(e_transform->position, pos);
		if (aux_distance <= radius){
			botsInRange.push_back(element);
		}
	}
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

void aimanager::warningToClose(aicontroller* me, float warning_distance){
	bt_grandma* me_bt = (bt_grandma*)me;
	TCompTransform* player_transform=(TCompTransform*)me_bt->getPlayerTransform();
	TCompTransform* me_transform = ((CEntity*)me->GetEntity())->get<TCompTransform>();
	for (int i = 0; i < bots.size(); i++){
		//I don´t warn myself
		if (bots[i] != me){
			//Check if the bot is close enought to recieve the warning
			TCompTransform* bot_transform = ((CEntity*)bots[i]->GetEntity())->get<TCompTransform>();
			if (V3DISTANCE(me_transform->position, bot_transform->position) < warning_distance){
				((CEntity*)bots[i]->GetEntity())->sendMsg(TWarWarning(((CEntity*)bots[i]->GetEntity()), player_transform->position));
			}
		}
	}
}

void aimanager::warningPlayerFound(aicontroller* me){
	//Warning all the angry grandma that the player have been found

	for (int i = 0; i < bots.size(); i++){
		bt_grandma* bt_bot = (bt_grandma*)bots[i];
		// TENDRIA QUE DESCARTAR A LA ABUELA QUE INVOCA ESTE METODO!!
		if (me != bots[i]){
			if (bt_bot->isAngry()){
				((CEntity*)bots[i]->GetEntity())->sendMsg(TPlayerFound());
			}
		}
	}
}