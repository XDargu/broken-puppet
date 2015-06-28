#include "mcv_platform.h"
#include "comp_golden_needle_item.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"
#include "comp_transform.h"
#include "handle\prefabs_manager.h"
#include "comp_name.h"
#include "comp_aabb.h"
#include "font\font.h"

TCompGNItem::TCompGNItem() {
	taked = false;
}

TCompGNItem::~TCompGNItem() {
}

void TCompGNItem::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);
}

void TCompGNItem::init() {
	player = CEntityManager::get().getByName("Player");
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	player_aabb = ((CEntity*)player)->get<TCompAABB>();
}

void TCompGNItem::update(float elapsed) {
	TCompAABB* aabb = (TCompAABB*)m_aabb;
	TCompTransform* transform = (TCompTransform*)m_transform;
	TCompTransform* p_transform = (TCompTransform*)player_transform;
	TCompAABB* p_aabb = (TCompAABB*)player_aabb;
	AABB GNaabb = AABB(((TCompAABB*)m_aabb)->min, ((TCompAABB*)m_aabb)->max);
	if (GNaabb.intersects(p_aabb)){
		//Aqui faltaría crear un efecto de particulas en la misma posicion del item para dar
		//feedback al player de que lo ha cogido
		CEntityManager::get().remove(m_transform.getOwner());
	}
}

void TCompGNItem::renderDebug3D() {
	std::string a = "";

	TCompTransform* transform = (TCompTransform*)m_transform;
	font.print3D(transform->position, a.c_str());
}
