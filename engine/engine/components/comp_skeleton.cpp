#include "mcv_platform.h"
#include "comp_skeleton.h"
#include "skeletons/skeleton_manager.h"
#include "physics_manager.h"
#include "render/render_utils.h"
#include "handle/handle.h"
#include "components/comp_transform.h"
#include "components/comp_rigid_body.h"
#include "comp_unity_character_controller.h"
#include "font/font.h"
#include "render/render_manager.h"
#include "comp_ragdoll.h"

extern CMesh        axis;
#include "render/ctes/shader_ctes.h"

extern CShaderCte<TCtesBones> ctes_bones;

CalVector DX2Cal(XMVECTOR v) {
  return CalVector(
    XMVectorGetX(v)
    , XMVectorGetY(v)
    , XMVectorGetZ(v)
    );
}
XMVECTOR Cal2DX(CalVector q) {
  return XMVectorSet(q.x, q.y, q.z, 1.0f);
}
CalQuaternion DX2CalQuat(XMVECTOR v) {
  return CalQuaternion(
    XMVectorGetX(v)
    , XMVectorGetY(v)
    , XMVectorGetZ(v)
    , -XMVectorGetW(v)
    );
}
XMVECTOR Cal2DX(CalQuaternion q) {
  return XMVectorSet(q.x, q.y, q.z, -q.w);
}

CalQuaternion getRotationFromAToB(CalVector a, CalVector b, float unit_amount) {
  XMVECTOR da = Cal2DX(a);
  XMVECTOR db = Cal2DX(b);
  XMVECTOR normal = XMVector3Cross(da, db);
  XMVECTOR angle = XMVector3AngleBetweenVectors(da, db);
  XMVECTOR q = XMQuaternionRotationAxis(normal, XMVectorGetX(angle));
  return DX2CalQuat(q);
}

struct CalTransform {
	CalVector pos;
	CalQuaternion rot;	

	CalTransform()
	{		
		pos = CalVector(0, 0, 0);
		rot = CalQuaternion(0, 0, 0, 1);
	}

	CalTransform(CalVector p, CalQuaternion q) {
		pos = p;
		rot = q;
	}
};

void CCoreModel::TBoneCorrector::apply(CalModel* model, CalVector world_pos, float amount) {
  CalBone* bone = model->getSkeleton()->getBone(bone_id);
  assert(bone);
  
  // My coords in world coords
  CalVector bone_abs_pos = bone->getTranslationAbsolute();

  // The direction in world coords
  CalVector dir_abs = world_pos - bone_abs_pos;

  // Convert it to local coords of my bone
  CalQuaternion abs_to_local_rot = bone->getRotationAbsolute();
  abs_to_local_rot.invert();

  // Convert the world dir to local dir
  CalVector dir_local_to_target = dir_abs;
  dir_local_to_target *= abs_to_local_rot;

  CalQuaternion correction_rot = getRotationFromAToB(local_dir, dir_local_to_target, 1.0f);

  // The current bone rotation wrt my parent bone
  CalQuaternion bone_local_rotation = bone->getRotation();

  // Apply the correction and set it to the bone
  correction_rot *= bone_local_rotation;
  bone->setRotation(correction_rot);

  // Compute new abs rotation and update abs of my children bones
  bone->calculateState();
}

void TCompSkeleton::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	h_transform = assertRequiredComponent<TCompTransform>(this);


  std::string skel_name = atts["name"];
  CDBGTimer tm;
  core_model = (CCoreModel*) skeleton_manager.getByName(skel_name.c_str());
  dbg("Load skeleton %s took %g seconds. Seconds since scene load: %g\n", skel_name, tm.seconds(), CApp::get().load_timer.seconds());
  CApp::get().load_skel_time += tm.seconds();
  model = new CalModel(core_model);
  model->getMixer()->blendCycle(0, 1.0, 0.f);

  // Register a key to render the mesh
  const CMaterial* mat = material_manager.getByName(("skin_" + skel_name).c_str());
  bool a = mat->castsShadows();
  const CMesh* mesh = core_model->getMesh();

  for (int i = 0; i < mesh->getNGroups(); ++i )
    render_manager.addKey(mesh, mat, i, CHandle(this), &active);

  // Guardar estado inicial del esqueleto
  model->update(0.05f);
  CalSkeleton* skel = model->getSkeleton();
  auto& cal_bones = skel->getVectorBone();

  int size = cal_bones.size();
  bone_ragdoll_transforms = new CalTransform[size];

  for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
	  CalBone* bone = cal_bones[bone_idx];

	  bone_ragdoll_transforms[bone_idx] = CalTransform(
		  bone->getTranslationBoneSpace()
		  , bone->getRotationBoneSpace()
		  );
  }

  time_since_last_ragdoll = 5000;

}

void TCompSkeleton::init() {
	h_ragdoll = getSibling<TCompRagdoll>(this);
	h_rigidbody = getSibling<TCompRigidBody>(this);

}


void TCompSkeleton::update(float elapsed) {

  /*if (isKeyPressed(VK_SHIFT)) {
    elapsed *= 0.05f;
  }*/

  /*if (isKeyPressed('o') || isKeyPressed('O')) {
    model->getMixer()->executeAction(3, 0.0f, 0.3f, 1.0f, false);
	//model->getMixer()->blendCycle(1, 1.0, 0.3);
  }
  if (isKeyPressed('P')) {
    model->getMixer()->removeAction(1, 0.f);
  }
  if (isKeyPressed('L')) {
    model->getMixer()->removeAction(1, 0.3f);
  }*/

  //if (hasKeyBecomedPressed('o')) {
  //  model->getMixer()->blendCycle(0, 1.0, 0.3);
  //}
  //if (hasKeyBecomedPressed('p')) {
  //  model->getMixer()->executeAction(1, 1.0, 0.3);
  //}
  //if (hasKeyBecomedPressed('O')) {
  //  model->getMixer()->clearCycle(0, 0.3);
  //}
  //if (hasKeyBecomedPressed('P')) {
  //  model->getMixer()->clearCycle(1, 0.3);
  //}

  // Get transform of the entity
  TCompTransform *t = h_transform;
  TCompRigidBody *r = h_rigidbody;
  TCompUnityCharacterController *u = getSibling<TCompUnityCharacterController>(this);


  bool draw_ragdoll = h_ragdoll.isValid();
  if (draw_ragdoll) {
	  draw_ragdoll = ((TCompRagdoll*)h_ragdoll)->isRagdollActive();
  }
  if (draw_ragdoll) {

	  TCompRagdoll* ragdoll = h_ragdoll;

	  CalSkeleton* skel = model->getSkeleton();
	  auto& cal_bones = skel->getVectorBone();
	  for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {

		  PxRigidDynamic* rigid_bone = ragdoll->getBoneRigid(bone_idx);

		  if (rigid_bone) {
			  CalBone* bone = cal_bones[bone_idx];

			  CalVector parent_abs_pos = CalVector(0, 0, 0);
			  CalQuaternion parent_abs_rot = CalQuaternion(0, 0, 0, 1);

			  // Parent abs coords
			  int parent_id = bone->getCoreBone()->getParentId();
			  if (parent_id != -1) {
				  parent_abs_pos = cal_bones[parent_id]->getTranslationAbsolute();
				  parent_abs_rot = cal_bones[parent_id]->getRotationAbsolute();
			  }
			  else {
				  parent_abs_pos = bone->getTranslationAbsolute();
				  parent_abs_rot = bone->getRotationAbsolute();
			  }

			  CalQuaternion cal_abs_rot = bone->getRotationAbsolute();  // quat
			  CalVector  cal_abs_pos = bone->getTranslationAbsolute(); // vec3

			  // Physics abs coords
			  CalVector rigid_abs_pos = DX2Cal(Physics.PxVec3ToXMVECTOR(rigid_bone->getGlobalPose().p));
			  CalQuaternion rigid_abs_rot = DX2CalQuat(Physics.PxQuatToXMVECTOR(rigid_bone->getGlobalPose().q));

			  CalQuaternion parent_to_local = parent_abs_rot;
			  parent_to_local.invert();

			  CalQuaternion new_local_rotation = rigid_abs_rot;
			  new_local_rotation *= parent_to_local;

			  bone->setRotation(new_local_rotation);

			  // T_abs = (T * p_rot_abs) + p_T_abs
			  // (T_abs - p_T_abs) = T * p_rot_abs
			  // T_abs - p_T_abs * p_rot_abs_in = T
			  CalVector delta_abs_pos = rigid_abs_pos - parent_abs_pos;
			  CalQuaternion my_rotation_to_local = parent_abs_rot;
			  my_rotation_to_local.invert();

			  CalVector delta_in_local = delta_abs_pos;
			  delta_in_local *= my_rotation_to_local;

			  TTransform tt = TTransform(Cal2DX(delta_in_local), Cal2DX(new_local_rotation), XMVectorSet(1,1,1,1));

			  CalVector right = DX2Cal(tt.getLeft());
			  float half_height = 0;
			  
			  PxShape* collider;
			  rigid_bone->getShapes(&collider, 1, 0);

			  PxCapsuleGeometry geom;
			  collider->getCapsuleGeometry(geom);
			  half_height = geom.halfHeight;

			  bone->setTranslation(delta_in_local);			  
			  
			  bone->calculateState();
		  }
	  }
  }
  else {

	  model->getMixer()->setRootTranslation(DX2Cal(t->position));
	  model->getMixer()->setRootRotation(DX2CalQuat(t->rotation));
	  model->update(elapsed);

	  if (h_ragdoll.isValid()) {
		  time_since_last_ragdoll += elapsed;

		  // If the ragdoll exists and has been deactivated recently, interpolate the bones
		  if (time_since_last_ragdoll < 0.5f) {
			  CalSkeleton* skel = model->getSkeleton();
			  auto& cal_bones = skel->getVectorBone();

			  int size = cal_bones.size();

			  for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
				  CalBone* bone = cal_bones[bone_idx];

				  CalVector cal_pos = bone->getTranslation();
				  CalQuaternion cal_rot = bone->getRotation();

				  XMVECTOR dx_cal_pos = Cal2DX(cal_pos);
				  XMVECTOR dx_cal_rot = Cal2DX(cal_rot);

				  XMVECTOR ragdoll_pos = Cal2DX(bone_ragdoll_transforms[bone_idx].pos);
				  XMVECTOR ragdoll_rot = Cal2DX(bone_ragdoll_transforms[bone_idx].rot);

				  XMVECTOR final_pos = XMVectorLerp(ragdoll_pos, dx_cal_pos, time_since_last_ragdoll / .5f);
				  XMVECTOR final_rot = XMQuaternionSlerp(ragdoll_rot, dx_cal_rot, time_since_last_ragdoll / .5f);

				  //bone->setTranslation(DX2Cal(final_pos));
				  bone->setRotation(DX2CalQuat(final_rot));

				  bone->calculateState();
			  }
		  }
	  }

	  CalVector delta_logic_trans = model->getMixer()->getAndClearLogicTranslation();

	  if (delta_logic_trans.length() > 0) {
		  /*if (u) {
			  PxTransform u_transform = u->enemy_rigidbody->getGlobalPose();
			  u_transform.p += Physics.XMVECTORToPxVec3(Cal2DX(delta_logic_trans));
			  u->enemy_rigidbody->setGlobalPose(u_transform);
			  }
			  else if (r) {
			  PxTransform r_transform = r->rigidBody->getGlobalPose();
			  r_transform.p += Physics.XMVECTORToPxVec3(Cal2DX(delta_logic_trans));
			  r->rigidBody->setGlobalPose(r_transform);
			  }
			  else {
			  t->position += Cal2DX(delta_logic_trans);
			  }*/
	  }
  }
}

void TCompSkeleton::renderBoneAxis(int bone_id) const {
  auto bones = model->getSkeleton()->getVectorBone();
  CalBone* bone = bones[bone_id];
  TTransform tmx;
  tmx.rotation = Cal2DX(bone->getRotationAbsolute());
  tmx.position = Cal2DX(bone->getTranslationAbsolute());
  XMMATRIX world = tmx.getWorld();
  world = XMMatrixScaling(10.f, 10.f, 10.f) * world;
  setWorldMatrix(world);
  axis.activateAndRender();
}

void TCompSkeleton::renderDebug3D() const {
	/*return;
	int id_bones[] = { 10, 23, 21, 8 };
	CalSkeleton* skel = model->getSkeleton();
	auto& cal_bones = skel->getVectorBone();

	

	for (int id : id_bones) {
		CalBone* bone = cal_bones[id];
		XMVECTOR bone_pos = Cal2DX(bone->getTranslationAbsolute());

		drawLine(bone_pos, bone_pos + XMVectorSet(0, 10, 0, 0));
	}*/
	

	//return;
	/*bool draw_ragdoll = h_ragdoll.isValid();

	if (draw_ragdoll) {
		TCompRagdoll* ragdoll = h_ragdoll;
		CalSkeleton* skel = model->getSkeleton();
		auto& cal_bones = skel->getVectorBone();
		for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {

			PxRigidDynamic* rigid_bone = ragdoll->getBoneRigid(bone_idx);

			if (rigid_bone) {
				CalBone* bone = cal_bones[bone_idx];

				// Physics abs coords
				XMVECTOR rigid_abs_pos = Physics.PxVec3ToXMVECTOR(rigid_bone->getGlobalPose().p);
				XMVECTOR rigid_abs_rot = Physics.PxQuatToXMVECTOR(rigid_bone->getGlobalPose().q);

				TTransform bone_tt = TTransform(rigid_abs_pos, rigid_abs_rot, XMVectorSet(1, 1, 1, 1));

				XMVECTOR front = XMVector3Normalize(bone_tt.getFront());
				XMVECTOR up = XMVector3Normalize(bone_tt.getUp());
				XMVECTOR right = XMVector3Normalize(-bone_tt.getLeft());
				

				drawLine(rigid_abs_pos, rigid_abs_pos + front);
				drawLine(rigid_abs_pos, rigid_abs_pos + up);
				drawLine(rigid_abs_pos, rigid_abs_pos + right);
			}
		}
	}
	*/

  /*CCoreModel *core = (CCoreModel*) model->getCoreModel();
  for (auto bc : core->bone_ids_to_debug) {
    renderBoneAxis(bc);
  }

  // Render the bone lines
  auto bones = model->getSkeleton()->getVectorBone();
  for (auto it : bones) {
    auto bone_pos = it->getTranslationAbsolute();

    int parentId = it->getCoreBone()->getParentId();

    // If I have a parent
    if (parentId != -1) {
      CalBone* pParent = bones[parentId];
      auto parent_pos = pParent->getTranslationAbsolute();

      XMVECTOR src = DirectX::XMVectorSet(bone_pos.x, bone_pos.y, bone_pos.z, 1);
      XMVECTOR dst = DirectX::XMVectorSet(parent_pos.x, parent_pos.y, parent_pos.z, 1);
      drawLine(src, dst);
    }
  }*/

  // 	
  TCompRigidBody* rigid = h_rigidbody;
  CEntity* e = CHandle(rigid->rigidBody->userData);
  if (!e->hasTag("player")) { return; }
	
  auto actions = model->getMixer()->getAnimationActionList();
  float x0 = 20.f;
  float y0 = 20.f;
  y0 += XMVectorGetW(font.print(x0, y0, "Player Mixer:"));
  for (auto a : actions) {
    y0 += font.printf(x0, y0, "Action: %s Weigth: %1.3f Time:%1.3f/%1.3f"
		, a->getCoreAnimation()->getName().c_str()
      , a->getWeight()
      , a->getTime(), a->getCoreAnimation()->getDuration()
      );
  }
  auto cycles = model->getMixer()->getAnimationCycle();
  for (auto a : cycles) {
    y0 += font.printf(x0, y0, "Cycle: %s  Weigth: %1.3f Time:%1.3f/%1.3f"
		, a->getCoreAnimation()->getName().c_str()
      , a->getWeight()
      , a->getTime(), a->getCoreAnimation()->getDuration()
      );
  }

}

void TCompSkeleton::uploadBonesToGPU() const {

  TCtesBones* cpu_bones = ctes_bones.get();
  float*      fout = (float*) cpu_bones->bones;

  CalSkeleton* skel = model->getSkeleton();
  auto& cal_bones = skel->getVectorBone();
  for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
    CalBone* bone = cal_bones[bone_idx];

    const CalMatrix& cal_mtx = bone->getTransformMatrix();    // 3x3
    const CalVector  cal_pos = bone->getTranslationBoneSpace(); // vec3	

    *fout++ = cal_mtx.dxdx;
    *fout++ = cal_mtx.dydx;
    *fout++ = cal_mtx.dzdx;
    *fout++ = 0.f;
    *fout++ = cal_mtx.dxdy;
    *fout++ = cal_mtx.dydy;
    *fout++ = cal_mtx.dzdy;
    *fout++ = 0.f;
    *fout++ = cal_mtx.dxdz;
    *fout++ = cal_mtx.dydz;
    *fout++ = cal_mtx.dzdz;
    *fout++ = 0.f;
    *fout++ = cal_pos.x;
    *fout++ = cal_pos.y;
    *fout++ = cal_pos.z;
    *fout++ = 1.0f;
  }

  ctes_bones.uploadToGPU();
  ctes_bones.activateInVS(3);     // fixed in shader_ctes.h TCtesBones SHADER_REGISTER(b3)
}

void TCompSkeleton::stopAnimation(int id) {
	if (id >= 0) {
		float blend = core_model->animation_blend_out_times[id];
		model->getMixer()->clearCycle(id, blend);
		model->getMixer()->removeAction(id, blend);
	}
}

void TCompSkeleton::cancelAnimation(int id) {
	if (id >= 0) {
		float blend = 0.f;
		model->getMixer()->clearCycle(id, blend);
		model->getMixer()->removeAction(id, blend);
	}
}

void TCompSkeleton::loopAnimation(int id) {
	if (id >= 0) {
		float blend = core_model->animation_blend_times[id];
		model->getMixer()->blendCycle(id, 1.0, blend);
	}
}

void TCompSkeleton::playAnimation(int id) {
	if (id >= 0) {
		float blend = core_model->animation_blend_times[id];
		float blend_out = core_model->animation_blend_out_times[id];
		model->getMixer()->executeAction(id, blend, blend_out, 1.0f, false);
	}
}

void TCompSkeleton::resetAnimationTime() {
	model->getMixer()->setAnimationTime(0);
}

float TCompSkeleton::getAnimationDuration(int id) {
	return model->getMixer()->getAnimationDuration();
}

XMVECTOR TCompSkeleton::getPositionOfBone(int id) {
	CalSkeleton* skel = model->getSkeleton();
	auto& cal_bones = skel->getVectorBone();

	CalBone* bone = cal_bones[id];
	return Cal2DX(bone->getTranslationAbsolute());
}

XMVECTOR TCompSkeleton::getRotationOfBone(int id) {
	CalSkeleton* skel = model->getSkeleton();
	auto& cal_bones = skel->getVectorBone();

	CalBone* bone = cal_bones[id];
	return Cal2DX(bone->getRotationAbsolute());
}

void TCompSkeleton::ragdollUnactive() {
	time_since_last_ragdoll = 0;
	CalSkeleton* skel = model->getSkeleton();
	auto& cal_bones = skel->getVectorBone();

	int size = cal_bones.size();

	for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
		CalBone* bone = cal_bones[bone_idx];

		bone_ragdoll_transforms[bone_idx] = CalTransform(
			bone->getTranslation()
			, bone->getRotation()
			);
	}
}

float TCompSkeleton::getCancelTime(int id) { return core_model->animation_cancel_times[id]; }