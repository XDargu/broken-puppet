#ifndef INC_COMP_TRANSFORM_H_
#define INC_COMP_TRANSFORM_H_

#include "mcv_platform.h"
#include "base_component.h"
#include "transform.h"

// ----------------------------------------
struct TCompTransform : public TTransform, TBaseComponent {     // 1
private:
	TTransform prev_transform;
public:
	TCompTransform() : TTransform() {}
	TCompTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : TTransform(np, nr, ns) {}

	void loadFromAtts(const std::string& elem, MKeyValue& atts);
	
	void update(float elapsed);

	bool transformChanged();
};

#endif
