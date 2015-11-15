#ifndef INC_COMP_PREFAB_GENERATOR_H_
#define INC_COMP_PREFAB_GENERATOR_H_

#include "base_component.h"

struct TCompPrefabGenerator : TBaseComponent {

	int amount_to_generate;
	int debris_created;
	int prefab_creation_delay;
	XMVECTOR last_random_pos;
	XMVECTOR last_created_pos;

	bool generate;

public:
	TCompPrefabGenerator();
	~TCompPrefabGenerator();

	void init();
	void update(float elapsed);

	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void generatePrefab(int amount);

private:
	bool generatingPrefabs(int amount, float elapsed);

};

#endif
