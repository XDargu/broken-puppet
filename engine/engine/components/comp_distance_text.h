#ifndef INC_COMP_DISTANCE_TEXT_H_
#define INC_COMP_DISTANCE_TEXT_H_

#include "base_component.h"
#include "entity_manager.h"

struct TCompDistanceText : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_transform;
	float distance;
	char initial_text[256];
	
public:
	char text[256];
	float size;
	unsigned int color;

	TCompDistanceText() : size(0), color(0) { text[0] = 0x00; }

	void loadFromAtts(MKeyValue &atts) {
		std::strcpy(initial_text, atts.getString("text", "no text").c_str());
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
			std::strcpy(text, initial_text);
		}
		else {
			std::strcpy(text, "");
		}
	}

	std::string toString() {
		return "Distance text";
	}
};

#endif