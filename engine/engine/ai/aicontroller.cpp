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

