#include "mcv_platform.h"
#include "comp_golden_needle_logic.h"
#include "entity_manager.h"
#include "ai\logic_manager.h"
#include "comp_transform.h"
#include "handle\prefabs_manager.h"
#include "comp_name.h"
#include "comp_golden_needle.h"
#include "comp_aabb.h"
#include "font\font.h"

TCompGNLogic::TCompGNLogic() {
	used = false;
	clue_point = XMVectorSet(0.f, 0.f, 0.f, 0.f);
}

TCompGNLogic::~TCompGNLogic() {
	CLogicManager::get().unregisterGNLogic(CHandle(this));
}

void TCompGNLogic::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	m_transform = assertRequiredComponent<TCompTransform>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	clue_point = atts.getPoint("cluePoint");

	CLogicManager::get().registerGNLogic(CHandle(this));
}

void TCompGNLogic::init() {
	player = CEntityManager::get().getByName("Player");
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	player_aabb = ((CEntity*)player)->get<TCompAABB>();
}

void TCompGNLogic::update(float elapsed) {
	TCompAABB* aabb = (TCompAABB*)m_aabb;
	TCompTransform* transform = (TCompTransform*)m_transform;
}

bool TCompGNLogic::checkPlayerInside(){
	TCompTransform* p_transform = (TCompTransform*)player_transform;
	TCompAABB* p_aabb = (TCompAABB*)player_aabb;
	AABB GNaabb = AABB(((TCompAABB*)m_aabb)->min, ((TCompAABB*)m_aabb)->max);
	if (GNaabb.intersects(p_aabb))
		return true;
	else
		return false;
}

XMVECTOR TCompGNLogic::getCluePoint(){
	return clue_point;
}

void TCompGNLogic::throwGoldenNeedle(){
	if (!used){
		TCompTransform* p_transform = (TCompTransform*)player_transform;
		CEntity* new_golden_needle = prefabs_manager.getInstanceByName("GoldenNeedle");
		TCompName* new__golden_needle_name = new_golden_needle->get<TCompName>();
		std::strcpy(new__golden_needle_name->name, "GoldenNeedle");

		TCompGoldenNeedle* new_e_golden_needle = new_golden_needle->get<TCompGoldenNeedle>();

		new_e_golden_needle->create(p_transform->position + XMVectorSet(0.f, 1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 0.f), clue_point);
		used = true;
	}
}

void TCompGNLogic::renderDebug3D() {
	std::string a = "";

	TCompTransform* transform = (TCompTransform*)m_transform;
	font.print3D(transform->position, a.c_str());
	font.print3D(clue_point, "CLUE_POINT");
}
