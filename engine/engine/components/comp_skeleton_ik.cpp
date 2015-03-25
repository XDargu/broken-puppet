#include "mcv_platform.h"
#include "comp_skeleton_ik.h"
#include "comp_skeleton.h"
#include "handle/handle.h"
#include "skeletons/skeleton_manager.h"
#include "skeletons/ik_handler.h"
#include "comp_transform.h"
#include "physics_manager.h"

void TCompSkeletonIK::loadFromAtts(const std::string& elem, MKeyValue &atts) {
  //target = atts.getPoint("target");
  //amount = atts.getFloat("amount", 1.0f);
  bone_ids_to_adapt[0] = atts.getInt("bone_c0", -1);
  bone_ids_to_adapt[1] = atts.getInt("bone_c1", -1);
  normals[0] = atts.getPoint("normal_c0");
  normals[1] = atts.getPoint("normal_c1");
}

void TCompSkeletonIK::solveBone(int bone_id_c, XMVECTOR n) {

	SET_ERROR_CONTEXT("Solving IK bone", "");
  // Access to the sibling comp skeleton component
  // where we can access the cal_model instance
  CEntity* e = CHandle(this).getOwner();
  TCompSkeleton *comp_skel = e->get<TCompSkeleton>();
  if (comp_skel == nullptr)
    return;

  CalModel* model = comp_skel->model;
  XASSERT(bone_id_c != -1, "Invalid C bone from ik");
  CalBone* bone_c = model->getSkeleton()->getBone(bone_id_c);

  int bone_id_b = bone_c->getCoreBone()->getParentId();
  XASSERT(bone_id_b != -1, "Invalid B bone from ik");
  CalBone* bone_b = model->getSkeleton()->getBone(bone_id_b);

  int bone_id_a = bone_b->getCoreBone()->getParentId();
  XASSERT(bone_id_a != -1, "Invalid A bone from ik");
  CalBone* bone_a = model->getSkeleton()->getBone(bone_id_a);

  TIKHandle ik;
  ik.A = Cal2DX(bone_a->getTranslationAbsolute());
  ik.B = Cal2DX(bone_b->getTranslationAbsolute());
  ik.C = Cal2DX(bone_c->getTranslationAbsolute());

  // Distance from a to b, based on the skel CORE definition
  CalVector cal_ab = bone_b->getCoreBone()->getTranslationAbsolute()
  - bone_a->getCoreBone()->getTranslationAbsolute();
  ik.AB = cal_ab.length();
  XMVECTOR ab2 = XMVector3Length(ik.A - ik.B);

  CalVector cal_bc = bone_c->getCoreBone()->getTranslationAbsolute()
  - bone_b->getCoreBone()->getTranslationAbsolute();
  ik.BC = cal_bc.length();
  XMVECTOR bc2 = XMVector3Length(ik.B - ik.C);

  // Lo anterior tendria que cachearse o inicializarse solo una vez

  float distance_of_c_to_ground = 0; // ground.distanceToGround(ik.C);
  XMVECTOR normal_to_ground = XMVectorSet(0, 1, 0, 0);

  // Raycast para detectar distancia al suelo
  physx::PxRaycastBuffer hit;
  XMVECTOR ray_position = ik.C;
  XMVECTOR ray_dir = XMVectorSet(0, -1, 0, 0);
  Physics.raycast(ray_position, ray_dir, 1, hit);

  if (hit.hasBlock) {
	  physx::PxRaycastHit blockHit = hit.block;
	  distance_of_c_to_ground = blockHit.distance;
	  normal_to_ground = Physics.PxVec3ToXMVECTOR(blockHit.normal);
  }

  // Dado que el pie antes de hacer las modificaciones no esta sobre el plano
  // exactamente sino que los artistas lo han subido un poco...
  TCompTransform* tmx = e->get< TCompTransform >();
  float height_of_e = XMVectorGetY(tmx->position);
  float height_of_c = XMVectorGetY(ik.C);

  float offset_of_over_ground = height_of_c - height_of_e;
  distance_of_c_to_ground -= offset_of_over_ground;

  // Calcular la rotacion que habia del bone c hijo respecto a la normal
  // de los animadores y=1
  bool has_children = !bone_c->getCoreBone()->getListChildId().empty();
  int bone_d_id = -1;
  // Acceder al bone 'd', hijo de c
  XMVECTOR q_correction_of_d;
  if (has_children) {
    bone_d_id = bone_c->getCoreBone()->getListChildId().front();
    CalBone* bone_d = model->getSkeleton()->getBone(bone_d_id);
    
    // La direccion de referencia del bone hijo de c
    XMVECTOR abs_d = Cal2DX(bone_d->getTranslationAbsolute());
    XMVECTOR abs_c = Cal2DX(bone_c->getTranslationAbsolute());
    XMVECTOR dir_c2d = abs_d - abs_c;

    // La direccion de referencia que pusieron los artistas del suel
    XMVECTOR default_ground = XMVectorSet(0, 1, 0, 0);
    
    XMVECTOR q_normal = XMVector3Normalize( XMVector3Cross(default_ground, dir_c2d) );
    XMVECTOR q_angle = XMVector3AngleBetweenVectors(default_ground, dir_c2d);
    q_correction_of_d = XMQuaternionRotationAxis(q_normal, XMVectorGetX(q_angle) * 1.25f);
  }


  //dbg("Delta_y is %f\n", delta_y_over_c);
  //ik.C += XMVectorSet(delta_y_over_c, 0 , 0, 0);

  ik.C += XMVectorSet(0, -distance_of_c_to_ground, 0, 0);


  //ik.normal = XMVector3Cross(ik.C - ik.A, ik.B - ik.A);
  //ik.normal = XMVectorSet(0, -1, 0, 0);
  ik.normal = n;
  ik.solveB();

  if (amount == 0.f)
  return;

  //// Correct A to point to B
  CCoreModel::TBoneCorrector bc;
  bc.bone_id = bone_id_a;
  bc.local_amount = 1.0f;
  bc.local_dir.set(1, 0, 0);
  bc.apply(model, DX2Cal(ik.B), amount);

  //// Correct A to point to B
  bc.bone_id = bone_id_b;
  bc.local_amount = 1.0f;
  bc.local_dir.set(1, 0, 0);
  bc.apply(model, DX2Cal(ik.C), amount);

  // Corregir pie C
  bc.bone_id = bone_id_c;
  bc.local_amount = 1.0f;
  bc.local_dir.set(1, 0, 0);

  XMVECTOR new_normal = normal_to_ground;//ground.normal;  // Fix, compute it!
  XMVECTOR new_dir_of_d = XMVector3Rotate(new_normal, q_correction_of_d);
  XMVECTOR final_d = ik.C + new_dir_of_d;
  bc.apply(model, DX2Cal(final_d), amount);
}

void TCompSkeletonIK::update(float elapsed) {

  for (int i = 0; i < 2; ++i) {
    if (bone_ids_to_adapt[i] != -1)
      solveBone(bone_ids_to_adapt[i], normals[i]);
  }

}
