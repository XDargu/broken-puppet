#include "mcv_platform.h"
#include "comp_skeleton_lookat.h"
#include "comp_skeleton.h"
#include "handle/handle.h"
#include "skeletons/skeleton_manager.h"

void TCompSkeletonLookAt::loadFromAtts(const std::string& elem, MKeyValue &atts) {
  target = atts.getPoint("target");
  amount = atts.getFloat("amount", 1.0f);
  active = true;
}

void TCompSkeletonLookAt::update(float elapsed) {

  // Access to the sibling comp skeleton component
  // where we can access the cal_model instance
  CEntity* e = CHandle(this).getOwner();
  TCompSkeleton *comp_skel = e->get<TCompSkeleton>();
  if (comp_skel == nullptr)
    return;

  CalModel* model = comp_skel->model;
  
  if (active) {
    // Apply all the corrections of the core skeleton definition
    CCoreModel *core = (CCoreModel*)model->getCoreModel();
    for (auto bc : core->bone_corrections) {
      bc.apply(model, DX2Cal(target), amount * bc.local_amount );
    }
  }

}
