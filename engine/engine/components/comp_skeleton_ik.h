#ifndef INC_COMP_SKELETON_IK_H_
#define INC_COMP_SKELETON_IK_H_

#include "base_component.h"

struct TCompSkeletonIK : TBaseComponent { 
  float     amount;
  XMVECTOR  normals[2];
  int       bone_ids_to_adapt[2];
  void solveBone(int bone_id_c, XMVECTOR n);

  TCompSkeletonIK() : amount(1.0f) {}
  void loadFromAtts(const std::string& elem, MKeyValue &atts);
  void update(float elapsed);
};

#endif
