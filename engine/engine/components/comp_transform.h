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
public:
	TCompTransform() : TTransform(), parent(CHandle()) { parent_name[0] = 0x00; }
	TCompTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : TTransform(np, nr, ns), parent(CHandle()) { parent_name[0] = 0x00; }

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void init();
	
	void update(float elapsed);

	void teleport(XMVECTOR the_position);

	bool transformChanged();
};

#endif
