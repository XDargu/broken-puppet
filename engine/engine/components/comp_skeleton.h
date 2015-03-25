#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "base_component.h"

class CalModel;
class CalBone;


struct TCompSkeleton : TBaseComponent {    // 2 ...
  CalModel*  model;

  TCompSkeleton() : model(nullptr) { }
  void loadFromAtts(const std::string& elem, MKeyValue &atts);
  void update(float elapsed);
  void renderDebug3D() const;
  void renderBoneAxis(int bone_id) const;

  void uploadBonesToGPU() const;

};

#endif
