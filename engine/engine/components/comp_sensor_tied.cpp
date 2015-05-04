#include "mcv_platform.h"
#include "comp_sensor_tied.h"

void TCompSensorTied::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	tied = false;
}

void TCompSensorTied::init() {
	tied = false;
}

void TCompSensorTied::changeTiedState(bool b, CHandle ropeRef){
	rope = ropeRef;
	tied = b;
}

bool TCompSensorTied::getTiedState(){
	return tied;
}

CHandle TCompSensorTied::getRopeRef(){
	return rope;
}
