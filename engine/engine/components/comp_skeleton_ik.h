#ifndef INC_COMP_SKELETON_IK_H_
#define INC_COMP_SKELETON_IK_H_

#include "base_component.h"

struct TCompSkeletonIK : TBaseComponent { 
	float     amount;

	struct TBoneMod {
		XMVECTOR  normal;
		int       bone_id;
	};

	TBoneMod  mods[2];
	void  solveBone(TBoneMod* bm);

  TCompSkeletonIK() : amount(1.0f) {}
  void loadFromAtts(const std::string& elem, MKeyValue &atts);
  void update(float elapsed);

};

#endif
