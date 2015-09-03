#include "mcv_platform.h"
#include "comp_trigger.h"
#include "entity_manager.h"
#include "comp_audio_listener.h"
#include "ai\logic_manager.h"
#include "comp_transform.h"
#include "audio\sound_manager.h"

TCompAudioListener::~TCompAudioListener() {
}

void TCompAudioListener::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
}

void TCompAudioListener::init() {
}

void TCompAudioListener::update(float elapsed){

	TCompTransform* transform = h_transform;
	CSoundManager::get().setListenerTransform(*transform);


	//Hacer metodo que convierta XMVECTOR en BASS_3DVECTOR
	/*BASS_3DVECTOR pos_ref;
	BASS_3DVECTOR front_ref;
	BASS_3DVECTOR top_ref;
	TCompTransform* transform = getSibling<TCompTransform>(this);
	pos_ref.x = XMVectorGetX(transform->position);
	pos_ref.y = XMVectorGetY(transform->position);
	pos_ref.z = XMVectorGetZ(transform->position);
	front_ref.x = XMVectorGetX(transform->getFront());
	front_ref.y = XMVectorGetY(transform->getFront());
	front_ref.z = XMVectorGetZ(transform->getFront());
	top_ref.x = XMVectorGetX(transform->getUp());
	top_ref.y = XMVectorGetY(transform->getUp());
	top_ref.z = XMVectorGetZ(transform->getUp());
	pos = &pos_ref;
	front = &front_ref;
	top = &top_ref;
	BASS_Set3DPosition(pos, NULL, front, top);*/
	//BASS_Apply3D();
}