#include "mcv_platform.h"
#include "comp_golden_needle.h"
#include "comp_transform.h"

static const float max_distance = 0.2f;
static const float velocity = 6.5f;
static const float first_velocity = 1.f;

TCompGoldenNeedle::TCompGoldenNeedle():used(false), first_reached(false){}


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
	if (!first_reached){
		XMVECTOR first_position = initial_position + XMVectorSet(0.f, 1.5f, 0.f, 0.f);
		float first_distance = V3DISTANCE(trans->position, first_position);
		if (first_distance > max_distance){
			XMVECTOR direction = first_position - initial_position;
			direction = XMVector3Normalize(direction);
			XMVECTOR new_pos = (trans->position + direction * first_velocity * elapsed);
			trans->lookAt(first_position, trans->getUp());
			trans->position = new_pos;
		}else{
			first_reached = true;
			first_reached_pos = trans->position;
		}
	}else{
		float distance = V3DISTANCE(trans->position, finish_position);
		if (distance > max_distance){
			XMVECTOR direction = finish_position-first_reached_pos;
			
			direction = XMVector3Normalize(direction);
			XMVECTOR new_pos = (trans->position + direction * velocity * elapsed);
			trans->lookAt(finish_position, trans->getUp());
			trans->position = new_pos;
		}
	}
}

bool TCompGoldenNeedle::getUsed(){
	return used;
}