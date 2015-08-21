#ifndef INC_COMP_TRANSFORM_H_
#define INC_COMP_TRANSFORM_H_

#include "mcv_platform.h"
#include "base_component.h"
#include "transform.h"

// ----------------------------------------
struct TCompTransform : public TTransform, TBaseComponent {     // 1
private:
	TTransform prev_transform;
	CHandle parent;
	char parent_name[64];
	TTransform parent_offset;
	// Tipos:
	// - 0: Static
	// - 0.2: Player
	// - 1: Dynamic
	// - 0.8: Selected dynamic
	// - 0.9: Selected dynamic with needle
	// - 0.95: Dynamic with needle
	float type;
public:
	TCompTransform() : TTransform(), parent(CHandle()) { parent_name[0] = 0x00; prev_transform = TTransform(); }
	TCompTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : TTransform(np, nr, ns), parent(CHandle()) { parent_name[0] = 0x00; }

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void init();
	
	void update(float elapsed);

	void teleport(XMVECTOR the_position);

	bool transformChanged();

	int getType() { return (int)(type * 100); }

	void setType(float atype) { type = atype; }

	TTransform getPrevTransform() { return prev_transform; }
};

#endif
