#ifndef INC_COMP_SKELETON_LOOKAT_H_
#define INC_COMP_SKELETON_LOOKAT_H_

struct TCompSkeletonLookAt { 
  XMVECTOR target;
  float    amount;
  TCompSkeletonLookAt() { }
  void loadFromAtts(const std::string& elem, MKeyValue &atts);
  void update(float elapsed);
};

#endif
