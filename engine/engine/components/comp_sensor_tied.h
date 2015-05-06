#ifndef INC_COMP_SENSOR_TIED_H_
#define INC_COMP_SENSOR_TIED_H_

#include "base_component.h"
#include "../item_manager.h"

struct TCompSensorTied : TBaseComponent{
private:
	bool tied;
	CHandle rope;
public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void changeTiedState(bool b, CHandle ropeRef);
	bool getTiedState();
	CHandle getRopeRef();
	void keepTied();
};
#endif

