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
	mods[0].bone_id = atts.getInt("bone_c0", -1);
	mods[1].bone_id = atts.getInt("bone_c1", -1);
	mods[0].normal = atts.getPoint("normal_c0");
	mods[1].normal = atts.getPoint("normal_c1");
}

void TCompSkeletonIK::solveBone(TBoneMod* bm) {
	int bone_id_c = bm->bone_id;

	// Access to the sibling comp skeleton component
	// where we can access the cal_model instance
	CEntity* e = CHandle(this).getOwner();
	TCompSkeleton *comp_skel = e->get<TCompSkeleton>();
	if (comp_skel == nullptr)
		return;

	CalModel* model = comp_skel->model;
	assert(bone_id_c != -1);
	CalBone* bone_c = model->getSkeleton()->getBone(bone_id_c);

	int bone_id_b = bone_c->getCoreBone()->getParentId();
	assert(bone_id_b != -1);
	CalBone* bone_b = model->getSkeleton()->getBone(bone_id_b);

	int bone_id_a = bone_b->getCoreBone()->getParentId();
	assert(bone_id_a != -1);
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

	// ------------------------------------------------
	// Correct the foot orientation
	// Obtain the current abs front and left directions in local coords
	CalVector abs_front = DX2Cal(tmx->getFront());
	CalVector abs_left = DX2Cal(tmx->getLeft());
	CalQuaternion abs_rot_c = bone_c->getRotationAbsolute();
	CalQuaternion inv_abs_rot_c = abs_rot_c;
	inv_abs_rot_c.invert();
	// These are the local directions that forms the plane of the foot
	CalVector local_front = abs_front;  local_front *= inv_abs_rot_c;
	CalVector local_left = abs_left;   local_left *= inv_abs_rot_c;


	//dbg("Delta_y is %f\n", delta_y_over_c);
	//ik.C += XMVectorSet(delta_y_over_c, 0 , 0, 0);
	ik.C += XMVectorSet(0, -distance_of_c_to_ground, 0, 0);


	//ik.normal = XMVector3Cross(ik.C - ik.A, ik.B - ik.A);
	//ik.normal = XMVectorSet(0, -1, 0, 0);
	ik.normal = bm->normal;
	ik.solveB();

	if (amount == 0.f)
		return;

	//// Correct A to point to B
	CCoreModel::TBoneCorrector bc;
	bc.bone_id = bone_id_a;
	bc.local_dir.set(1, 0, 0);
	bc.apply(model, DX2Cal(ik.B), amount);

	//// Correct A to point to B
	bc.bone_id = bone_id_b;
	bc.local_dir.set(1, 0, 0);
	bc.apply(model, DX2Cal(ik.C), amount);

	// -------------------------------------------------------------------------
	// Now, that c has been modified, fix the foot orientation

	// Raycast de distancias
	float distance_of_cf_to_ground = 0;
	float distance_of_cl_to_ground = 0;
	// Raycast para detectar distancia al suelo
	// Ik.C + front
	ray_position = ik.C + Cal2DX(abs_front);
	ray_dir = XMVectorSet(0, -1, 0, 0);
	Physics.raycast(ray_position, ray_dir, 1, hit);

	if (hit.hasBlock) {
		physx::PxRaycastHit blockHit = hit.block;
		distance_of_cf_to_ground = blockHit.distance;
	}

	ray_position = ik.C + Cal2DX(abs_left);
	ray_dir = XMVectorSet(0, -1, 0, 0);
	Physics.raycast(ray_position, ray_dir, 1, hit);

	if (hit.hasBlock) {
		physx::PxRaycastHit blockHit = hit.block;
		distance_of_cl_to_ground = blockHit.distance;
	}

	// Check which are the new two main directions of the ground, sampling the ground
	// at C, and C + front, and C + left
	float distance_of_c0_to_ground = distance_of_c_to_ground;
	/*float distance_of_cf_to_ground = ground.distanceToGround(ik.C + Cal2DX(abs_front));
	float distance_of_cl_to_ground = ground.distanceToGround(ik.C + Cal2DX(abs_left));*/

	// Fix the front direction, using a bone corrector that will make the old 'local front'
	// look at the new abs front
	CalVector new_abs_front = abs_front;
	new_abs_front.y = distance_of_c0_to_ground - distance_of_cf_to_ground;
	CalVector front_target = bone_c->getTranslationAbsolute() + new_abs_front;
	CCoreModel::TBoneCorrector front_fix(bone_id_c, local_front);
	front_fix.apply(model, front_target, amount);

	// Do the same for the left direction, (which will only apply a roll over the front direction)
	CalVector new_abs_left = abs_left;
	new_abs_left.y = distance_of_c0_to_ground - distance_of_cl_to_ground;
	CalVector left_target = bone_c->getTranslationAbsolute() + new_abs_left;
	CCoreModel::TBoneCorrector left_fix(bone_id_c, local_left);
	left_fix.apply(model, left_target, amount);

}

/*void a() {
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
*/
void TCompSkeletonIK::update(float elapsed) {

	for (int i = 0; i < 2; ++i) {
		if (mods[i].bone_id != -1)
			solveBone(mods + i);
	}

}
