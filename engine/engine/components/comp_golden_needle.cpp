#include "mcv_platform.h"
#include "comp_golden_needle.h"
#include "comp_transform.h"

static const float max_distance = 0.2f;
static const float velocity = 0.1f;

TCompGoldenNeedle::TCompGoldenNeedle():used(false){}


void TCompGoldenNeedle::loadFromAtts(const std::string& elem, MKeyValue& atts) {
	
}

void TCompGoldenNeedle::create(XMVECTOR init_pos, XMVECTOR init_rot, XMVECTOR finish_pos) {
	m_transform = assertRequiredComponent<TCompTransform>(this);
	TCompTransform* trans = (TCompTransform*)m_transform;

	// Static position
	initial_position = init_pos;
	trans->position = init_pos;
	finish_position = finish_pos;
	trans->rotation = init_rot;
	used = true;
}

void TCompGoldenNeedle::fixedUpdate(float elapsed) {
	TCompTransform* trans = (TCompTransform*)m_transform;
	float distance = V3DISTANCE(trans->position, finish_position);
	if (distance > max_distance){
		XMVECTOR direction = initial_position - finish_position;
		direction = XMVector3Normalize(direction);
		XMVECTOR new_pos = (trans->position - (direction*velocity));
		trans->rotation = direction;
		trans->position = new_pos;
	}
}

bool TCompGoldenNeedle::getUsed(){
	return used;
}