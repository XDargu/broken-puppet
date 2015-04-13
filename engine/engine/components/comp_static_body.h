#ifndef INC_COMP_STATIC_BODY_H_
#define INC_COMP_STATIC_BODY_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompStaticBody : TBaseComponent {
private:
	CHandle h_transform;
public:
	physx::PxRigidStatic* staticBody;

	TCompStaticBody() : staticBody(nullptr) {}

	~TCompStaticBody();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void fixedUpdate(float elapsed);
};

#endif
