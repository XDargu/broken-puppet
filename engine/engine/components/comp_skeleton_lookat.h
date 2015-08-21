#ifndef INC_COMP_SKELETON_LOOKAT_H_
#define INC_COMP_SKELETON_LOOKAT_H_

#include "base_component.h"

struct TCompSkeletonLookAt : TBaseComponent { 
  XMVECTOR target;
  float    amount;
  bool active;

  TCompSkeletonLookAt() { }
  void loadFromAtts(const std::string& elem, MKeyValue &atts);
  void update(float elapsed);
};

#endif
