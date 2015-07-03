#include "mcv_platform.h"
#include "aimanager.h"
//#include "aicontroller.h"
#include "../components/all_components.h"

using namespace DirectX;

static aimanager the_aimanager;
const int max_size_attackers = 3;

aimanager::aimanager()
{
	last_id = 0;
	attackers_rol.push_back(nullptr);
	attackers_rol.push_back(nullptr);
	attackers_rol.push_back(nullptr);
}


aimanager::~aimanager()
{
}

aimanager& aimanager::get(){
	return the_aimanager;
}

aicontroller* aimanager::getAI(unsigned int id){
	for (auto & element : bots) {
		if (((bt_grandma*)element)->getId() == id)
			return element;
		// TODO: ARREGLAR ESTO
		/*if (element->getInt() == id)
			return element;*/
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
	TCompTransform* player_transform = (TCompTransform*)me_bt->getPlayerTransform();
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
				if ((CEntity*)bots[i]!=nullptr)
					((CEntity*)bots[i]->GetEntity())->sendMsg(TPlayerFound());
			}
		}
	}

}

bool aimanager::attackRolEmptySlot(){
	bool empty = false;
	for (int i = 0; i < attackers_rol.size(); ++i){
		if (attackers_rol[i] == nullptr){
			empty = true;
			return empty;
		}
	}
	return empty;
}

void aimanager::setEnemyRol(aicontroller* enemy){
	bool free_slot = false;
	bool already_attacker = false;
	bool sustitute_attacker = false;
	bool no_reasignated = false;
	aicontroller* bt_enemy = (aicontroller*)enemy;
	int ind_asignated = -1;
	int ind_sustitution = -1;
	int ind_my_already = -1;
	if (attackRolEmptySlot()){
		for (int i = 0; i < attackers_rol.size(); ++i){
			if (attackers_rol[i] == enemy){
				ind_my_already = i;
				already_attacker = true;
				break;
			}
		}
		if (!already_attacker){
			//attackers_rol.push_back(enemy);
			//Hay slots libres, tendré que comprobar cual es el más cercano a la posición del enemy, y escoger ese
			bool free_north = false;
			bool free_east = false;
			bool free_west = false;
			if (attackers_rol[0] == nullptr)
				free_north = true;
			if (attackers_rol[1] == nullptr)
				free_east = true;
			if (attackers_rol[2] == nullptr)
				free_west = true;

			int nearest_slot = bt_enemy->getNearestSlot(free_north, free_east, free_west);
			ind_asignated = nearest_slot;
			attackers_rol[nearest_slot] = enemy;
			bt_enemy->setRol(1);
			free_slot = true;
		}
		free_slot = true;
	}
	else{
		//Comprobar si es atacante, de serlo, no cambiar nada. Tendré que conseguir devolver mi slot actual
		int role = bt_enemy->getRol();
		if (role != 1){
			for (int i = 0; i < attackers_rol.size(); ++i){
				bt_grandma* bot_bt = (bt_grandma*)attackers_rol[i];
				if ((attackers_rol[i] != enemy) && (attackers_rol[i] != nullptr)){
					if (bt_enemy->getDistanceToPlayer() < bot_bt->getDistanceToPlayer()){
						bt_grandma* bot_bol = (bt_grandma*)attackers_rol[i];
						bot_bol->setRol(2);
						attackers_rol[i] = enemy;
						ind_sustitution = i;
						bt_enemy->setRol(1);
						sustitute_attacker = true;
						free_slot = true;
						break;
					}
				}
			}
			//Aqui si sustitute_attacker==false tenemos que meter al enemy en taunter
			if (!sustitute_attacker){
				bt_enemy->setRol(2);
				free_slot = false;
			}
		}
		else{
			free_slot = true;
			no_reasignated = true;
		}
	}

	if (free_slot){
		//Attacker role 
		bt_enemy->setRol(1);
		if (sustitute_attacker)
			ind_asignated = ind_sustitution;
		else if (no_reasignated){
			ind_asignated = bt_enemy->getAttackerSlot()-1;
		}
		else if (!already_attacker){
			//ind_asignated = attackers_rol.size() - 1;
			int prueba = 1;
		}
		else
			ind_asignated = ind_my_already;

		if (ind_asignated == 0){
			//North slot
			bt_enemy->setAttackerSlot(1);
		}
		else if (ind_asignated == 1){
			//East slot
			bt_enemy->setAttackerSlot(2);
		}
		else if (ind_asignated == 2){
			//West slot
			bt_enemy->setAttackerSlot(3);
		}
	}
	else{
		//Taunter role
		bt_enemy->setRol(2);
	}
}

void aimanager::RemoveEnemyTaunt(aicontroller* enemy){
	for (int i = 0; i < taunters_rol.size(); ++i){
		if (taunters_rol[i] == enemy){
			taunters_rol[i] = nullptr;
		}
	}
}

void aimanager::RemoveEnemyAttacker(aicontroller* enemy){
	for (int i = 0; i < attackers_rol.size(); ++i){
		if (attackers_rol[i] == enemy){
			attackers_rol[i] = nullptr;
		}
	}
}

void aimanager::recastAABBActivate(int ind){
	for (int i = 0; i < bots.size(); ++i){
		if (((bt_grandma*)bots[i])->getIndRecastAABB()==ind){
			((bt_grandma*)bots[i])->setActive(true);
		}
	}
}

void aimanager::recastAABBDesactivate(int ind){
	for (int i = 0; i < bots.size(); ++i){
		if (((bt_grandma*)bots[i])->getIndRecastAABB() == ind){
			((bt_grandma*)bots[i])->setActive(false);
		}
	}
}

void aimanager::clear(){
	attackers_rol.clear();
	attackers_rol.push_back(nullptr);
	attackers_rol.push_back(nullptr);
	attackers_rol.push_back(nullptr);
	taunters_rol.clear();
	bots.clear();
	last_id = 0;
}