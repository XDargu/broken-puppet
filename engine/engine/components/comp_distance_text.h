#ifndef INC_COMP_DISTANCE_TEXT_H_
#define INC_COMP_DISTANCE_TEXT_H_

#include "base_component.h"
#include "entity_manager.h"

struct TCompDistanceText : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_transform;
	float distance;
	
public:
	char text[256];
	float size;
	unsigned int color;
	bool in_range;

	TCompDistanceText() : size(0), color(0), in_range(false) { text[0] = 0x00; }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		std::strcpy(text, atts.getString("text", "no text").c_str());
		distance = atts.getFloat("distance", 1);
		size = atts.getFloat("size", 16);
		color = std::strtoul(atts.getString("color", "0xffffffff").c_str(), NULL, 16);
	}

	void init() {
		m_transform = assertRequiredComponent<TCompTransform>(this);
		CHandle player = CEntityManager::get().getByName("Player");
		player_transform = ((CEntity*)player)->get<TCompTransform>();

	}

	void update(float elapsed) {
		TCompTransform* transform = (TCompTransform*)m_transform;
		TCompTransform* p_transform = (TCompTransform*)player_transform;
		
		float p_distance = XMVectorGetX(XMVector3Length(transform->position - p_transform->position));

		if (p_distance < distance) {
			in_range = true;
		}
		else {
			in_range = false;
		}
	}

	std::string toString() {
		return "Distance text";
	}
};

#endif