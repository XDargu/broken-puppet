#include "mcv_platform.h"
#include "comp_golden_needle_logic.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"
#include "comp_transform.h"
#include "comp_aabb.h"
#include "font\font.h"

TCompGNLogic::TCompGNLogic() {
	clue_point = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

TCompGNLogic::~TCompGNLogic() {
	CLogicManager::get().unregisterGNLogic(CHandle(this));
}

void TCompGNLogic::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	TCompTransform* transform = (TCompTransform*)m_transform;
	TCompAABB* aabb = (TCompAABB*)m_aabb;
	clue_point = atts.getPoint("cluePoint");

	CLogicManager::get().registerGNLogic(CHandle(this));
}

void TCompGNLogic::init() {
	player = CEntityManager::get().getByName("Player");
	player_transform=((CEntity*)player)->get<TCompTransform>();
}

void TCompGNLogic::update(float elapsed) {
	TCompAABB* aabb = (TCompAABB*)m_aabb;
	TCompTransform* transform = (TCompTransform*)m_transform;
}

bool TCompGNLogic::checkPlayerInside(){
	TCompTransform* p_transform = (TCompTransform*)player_transform;
	AABB GNaabb = AABB(((TCompAABB*)m_aabb)->min, ((TCompAABB*)m_aabb)->max);
	if (GNaabb.containts(p_transform->position))
		return true;
	else
		return false;
}

XMVECTOR TCompGNLogic::getCluePoint(){
	return clue_point;
}

void TCompGNLogic::renderDebug3D() {
	std::string a = "";

	TCompTransform* transform = (TCompTransform*)m_transform;
	font.print3D(transform->position, a.c_str());
	font.print3D(clue_point, "CLUE_POINT");
}
