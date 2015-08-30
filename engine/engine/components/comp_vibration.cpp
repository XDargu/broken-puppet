#include "mcv_platform.h"
#include "comp_vibration.h"
#include "comp_transform.h"
#include "comp_static_body.h"

void TCompVibration::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
	frequency = atts.getFloat("frequency", 100);
	amount = atts.getFloat("amount", 0.01f);
}

void TCompVibration::update(float elapsed) {
	
	TCompTransform* c_transform = h_transform;

	double time = CApp::get().total_time;
	float delta = sin(time * frequency) * amount;

	XMVECTOR v_delta = XMVectorSet(delta, delta, delta,0);

	c_transform->position += v_delta;
	
}