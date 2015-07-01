#include "mcv_platform.h"
#include "aicontroller.h"


aicontroller::aicontroller()
{
}

aicontroller::~aicontroller()
{
}

void aicontroller::setId(unsigned int id){
	my_id = id;
}

unsigned int aicontroller::getId(){
	return my_id;
}

void aicontroller::setKind(kind type){
	enemy_type = type;
}

aicontroller::kind aicontroller::getKind(){
	return enemy_type;
}

