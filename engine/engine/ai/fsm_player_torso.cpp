#include "mcv_platform.h"
#include "fsm_player_torso.h"
#include "io\iostatus.h"
#include "components\all_components.h"
#include "handle\prefabs_manager.h"
#include "components\comp_skeleton.h"
#include "ai\logic_manager.h"
#include "components\comp_skeleton_lookat.h"

FSMPlayerTorso::FSMPlayerTorso()
	: can_move(true)
	, comp_transform(CHandle())
	, entitycount(1)
	, first_actor(nullptr)
	, first_needle(CHandle())
	, first_offset(PxVec3(0, 0, 0))
	, first_position(PxVec3(0, 0, 0))
	, standard_camera_offset(XMVectorZero())
	, first_throw(false)
	, up_animation(false)
	, max_num_string(0)
	, looking_at_pointer(false)
	, can_throw(true)
	, can_cancel(true)
	, can_pull(true)
	, can_tense(true)
{}
FSMPlayerTorso::~FSMPlayerTorso() {}

void FSMPlayerTorso::Init() {
	// Insert all states in the map
	AddState("fbp_ThrowString", (statehandler)&FSMPlayerTorso::ThrowString);
	AddState("fbp_ThrowGoldenNeedle", (statehandler)&FSMPlayerTorso::ThrowGoldenNeedle);
	AddState("fbp_PullString", (statehandler)&FSMPlayerTorso::PullString);
	AddState("fbp_GrabString", (statehandler)&FSMPlayerTorso::GrabString);	
	AddState("fbp_Inactive", (statehandler)&FSMPlayerTorso::Inactive);
	AddState("fbp_ProcessHit", (statehandler)&FSMPlayerTorso::ProcessHit);
	
	// Reset the state
	ChangeState("fbp_Inactive");

	// Get the transform
	comp_transform = ((CEntity*)entity)->get<TCompTransform>();

	// Get the skeleton
	comp_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();

	// Get tge camera
	camera_entity = CEntityManager::get().getByName("PlayerCamera");
	TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
	standard_camera_offset = camera_controller->offset;

	max_num_string = 4;

	can_move = true;
}

void FSMPlayerTorso::ThrowGoldenNeedle(float elapsed){
	CIOStatus& io = CIOStatus::get();

	// Throw the string
	if (on_enter) {

		/* PRUEBA COMP GOLDEN NEEDLE*/

		if (GNLogic.isValid()){
			TCompGNLogic* gn_logic = (TCompGNLogic*)GNLogic;
			gn_logic->throwGoldenNeedle();
		}
		/*--------------------------*/
	}

	// Animation ends
	if (state_time >= 0.1f) {
		ChangeState("fbp_Inactive");
	}
}

void FSMPlayerTorso::ThrowString(float elapsed) {
	CIOStatus& io = CIOStatus::get();
	TCompSkeleton* skeleton = comp_skeleton;

	// Throw the string
	if (on_enter) {

		CEntityManager &entity_manager = CEntityManager::get();
		CPhysicsManager &physics_manager = CPhysicsManager::get();
		
		TCompTransform* camera_transform = ((CEntity*)camera_entity)->get<TCompTransform>();

		PxActor* m_hit_actor = nullptr;
		PxVec3 hit_position;
		PxVec3 hit_normal;

		getThrowingData(m_hit_actor, hit_position, hit_normal);

		PxRigidActor* hit_actor = (PxRigidActor*)m_hit_actor;

		// Raycast detecting the collider the mouse is pointing at
		// Old single raycast
		/*PxRaycastBuffer hit;
		Physics.raycast(camera_transform->position, camera_transform->getFront(), 1000, hit);

		if (hit.hasBlock) {
			PxRaycastHit blockHit = hit.block;

			if (std::strcmp(blockHit.actor->getName(), "Player") != 0)
			{*/
		if (hit_actor != nullptr) {

			// First throw
			if (first_actor == nullptr) {
				CLogicManager::get().stringThrown();
				first_throw = true;

				first_actor = hit_actor;
				first_position = hit_position;
				first_offset = first_actor->getGlobalPose().q.rotateInv(hit_position - first_actor->getGlobalPose().p);

				/*unsigned int num_strings = getStringCount();

				// If there are more strings than the maximun available, remove the oldest one
				if (num_strings >= max_num_string){
				CHandle c_rope = strings.front();
				strings.pop_front();
				entity_manager.remove(c_rope.getOwner());
				}*/

				// Get the needle prefab
				Citem_manager::get().checkAndRemoveFirstNeedle();
				CEntity* new_needle = prefabs_manager.getInstanceByName("Needle");

				// Get the entity of the rigidbody on wich the needle is pierced
				//CEntity* rigidbody_e = entity_manager.getByName(blockHit.actor->getName()); //CHandle(blockHit.actor->userData);

				// Rename the needle
				TCompName* new_needle_name = new_needle->get<TCompName>();
				std::strcpy(new_needle_name->name, ("Needle" + to_string(entitycount)).c_str());

				// Set the rotation of the needle according to the camera angle and normal of the surface
				// The final rotation will be a quaternion between those two values, wigthed in one or other direction
				TTransform rotation_aux;
				rotation_aux.position = camera_transform->position;

				if (first_position == physics_manager.XMVECTORToPxVec3(camera_transform->position))
					rotation_aux.lookAt(physics_manager.PxVec3ToXMVECTOR(first_position) + camera_transform->getFront() * 0.1f, XMVectorSet(0, 1, 0, 0));
				else
					rotation_aux.lookAt(physics_manager.PxVec3ToXMVECTOR(first_position), XMVectorSet(0, 1, 0, 0));

				TTransform normal_aux;
				normal_aux.position = physics_manager.PxVec3ToXMVECTOR(first_position);
				normal_aux.lookAt(physics_manager.PxVec3ToXMVECTOR(first_position - hit_normal), XMVectorSet(0, 1, 0, 0));

				XMVECTOR finalQuat = XMQuaternionSlerp(rotation_aux.rotation, normal_aux.rotation, 0.35f);

				if (!first_actor->isRigidStatic()) {
					finalQuat = XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(first_actor->getGlobalPose().q)));
				}

				// Get the needle component and initialize it
				TCompNeedle* new_e_needle = new_needle->get<TCompNeedle>();

				/*new_e_needle->create(
					first_actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(first_offset) : physics_manager.PxVec3ToXMVECTOR(first_position)
					, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(first_actor->getGlobalPose().q)))
					, rigidbody_e->get<TCompRigidBody>()
					);*/

				new_e_needle->create(
					first_actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(first_offset) : physics_manager.PxVec3ToXMVECTOR(first_position)
					, finalQuat
					, hit_actor
					);

				first_needle = new_needle;

				// Create the rope, between the player and the target
				CEntity* new_e = entity_manager.createEmptyEntity();

				// Rename it
				TCompName* new_e_name = CHandle::create<TCompName>();
				std::strcpy(new_e_name->name, ("Joint" + to_string(entitycount)).c_str());
				new_e->add(new_e_name);

				// Get the player transform
				TCompTransform* p_transform = comp_transform;

				// Only add a distance joint in the first throw if the first actor is a rigidbody
				if (first_actor->isRigidDynamic()) {
					TCompDistanceJoint* new_e_j = CHandle::create<TCompDistanceJoint>();

					// The first position is the actor position with offset in world coords
					PxVec3 pos = first_actor->getGlobalPose().q.rotate(first_offset) + first_actor->getGlobalPose().p;

					// The second position is the player
					PxVec3 pos2 = physics_manager.XMVECTORToPxVec3(p_transform->position);

					// Offsets
					PxVec3 offset_1 = first_offset;
					PxVec3 offset_2 = PxVec3(0, 0, 0);

					new_e_j->create(first_actor, NULL, 1, first_position, pos2, PxTransform(offset_1), physx::PxTransform(offset_2));

					new_e->add(new_e_j);
				}

				// Add the rope component
				TCompRope* new_e_r = CHandle::create<TCompRope>();
				new_e->add(new_e_r);

				//Add the needle and the rope to the vector from item_manager
				Citem_manager::get().addNeedleToVector(new_e_needle, new_e);

				// Get the transform of the needle
				TCompTransform* needle_transform = new_needle->get<TCompTransform>();
				needle_transform->init();

				// Assing the positions (needle transform + current player position)
				new_e_r->setPositions(needle_transform, p_transform->position);
				new_e_r->pos_1 = skeleton->getPositionOfBone(29);

				// Set the distance joint of the needle as the current one (to move it while grabbing and pulling the string)
				current_rope_entity = new_e;

				// Add the string to the strings vector
				//strings.push_back(CHandle(new_e_r));
				CRope_manager::get().addString(new_e_r);

				entitycount++;

				//Checking if enemy tied
				CEntity* firstActorEntity = CHandle(first_actor->userData);

				//aicontroller::types kind=bot_ai->m_ai_controller->getType();
				if (firstActorEntity->hasTag("enemy")){

					TCompBtSoldier* bot_ai = firstActorEntity->get<TCompBtSoldier>();
					if (bot_ai){
						Citem_manager::get().addNeedle(CHandle(new_e_needle), CHandle(new_e_r));
					}

					TCompSensorTied* tied_sensor = firstActorEntity->get<TCompSensorTied>();
					if (tied_sensor) {
						tied_sensor->changeTiedState(true, CHandle(new_e_r));
					}

					// Get the ragdoll
					TCompRagdoll* ragdoll = firstActorEntity->get<TCompRagdoll>();
					if (ragdoll) {
						// Get the bone 
						PxRigidDynamic* bone = ragdoll->getBoneRigidRaycast(Physics.PxVec3ToXMVECTOR(first_position), camera_transform->getFront());
						if (bone != nullptr)
						{
							// ************ Change needle target ***********
							new_e_needle->create(
								XMVectorSet(0, 0, 0, 0)
								, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(bone->getGlobalPose().q)))
								, bone
								);

							// ************ Create aux entity ************
							CEntity* aux_e = entity_manager.createEmptyEntity();

							TCompName* new_e_name = CHandle::create<TCompName>();
							std::strcpy(new_e_name->name, ("JointAux" + to_string(entitycount)).c_str());
							aux_e->add(new_e_name);

							// ************ Create aux joint ************
							TCompDistanceJoint* aux_joint = CHandle::create<TCompDistanceJoint>();

							// The first position is the actor position with offset in world coords
							// TODO: Get the real position, to get a valid offset. Temporally, no offset.
							PxVec3 aux_pos = bone->getGlobalPose().p;
							PxVec3 aux_offset = bone->getGlobalPose().q.rotateInv(aux_pos - bone->getGlobalPose().p);

							PxVec3 pos = bone->getGlobalPose().q.rotate(aux_offset) + bone->getGlobalPose().p;

							// The second position is the player
							PxVec3 pos2 = physics_manager.XMVECTORToPxVec3(p_transform->position);

							// Offsets
							PxVec3 offset_1 = aux_offset;
							PxVec3 offset_2 = PxVec3(0, 0, 0);

							aux_joint->create(bone, NULL, 1, bone->getGlobalPose().p, pos2, PxTransform(offset_1), physx::PxTransform(offset_2));

							aux_e->add(aux_joint);

							new_e_r->joint_aux = CHandle(aux_joint);
						}
					}

				}
				else{
					//adding needle and rope to item manager
					Citem_manager::get().addNeedle(CHandle(new_e_needle), CHandle(new_e_r));
				}
			}
			// Second throw
			else {
				// The string can be thrown
				if ((hit_actor != first_actor) && !(hit_actor->isRigidStatic() && first_actor->isRigidStatic())) {
					CLogicManager::get().stringThrown();
					first_throw = false;

					// -------------- Get the offsets
					// Obtener el offset con coordenadas de mundo = (Offset_mundo - posición) * inversa(rotación)
					PxVec3 offset_1 = first_offset;
					PxVec3 offset_2 = hit_actor->getGlobalPose().q.rotateInv(hit_position - hit_actor->getGlobalPose().p);

					// -------------- Whats here so far:
					// - We have an entity with a rope component, pointing to a needle transform and a position
					// - A distance joint may or may not exists in this entity
					CEntity* rope_entity = current_rope_entity;
					CEntity* first_needle_entity = first_needle;

					// -------------- Create the new needle
					// Get the needle prefab
					Citem_manager::get().checkAndRemoveFirstNeedle();
					CEntity* new_needle_2 = prefabs_manager.getInstanceByName("Needle");

					// Get the entity of the rigidbody on wich the needle is pierced
					//CEntity* rigidbody_e = entity_manager.getByName(blockHit.actor->getName());

					// Rename the needle
					TCompName* new_e_name2 = new_needle_2->get<TCompName>();
					std::strcpy(new_e_name2->name, ("Needle" + to_string(entitycount)).c_str());

					// Set the rotation of the needle according to the camera angle and normal of the surface
					// The final rotation will be a quaternion between those two values, wigthed in one or other direction
					TTransform rotation_aux;
					rotation_aux.position = camera_transform->position;

					if (hit_position == physics_manager.XMVECTORToPxVec3(camera_transform->position))
						rotation_aux.lookAt(physics_manager.PxVec3ToXMVECTOR(hit_position) + camera_transform->getFront() * 0.1f, XMVectorSet(0, 1, 0, 0));
					else
						rotation_aux.lookAt(physics_manager.PxVec3ToXMVECTOR(hit_position), XMVectorSet(0, 1, 0, 0));

					TTransform normal_aux;
					normal_aux.position = physics_manager.PxVec3ToXMVECTOR(hit_position);
					normal_aux.lookAt(physics_manager.PxVec3ToXMVECTOR(hit_position - hit_normal), XMVectorSet(0, 1, 0, 0));

					XMVECTOR finalQuat = XMQuaternionSlerp(rotation_aux.rotation, normal_aux.rotation, 0.35f);

					if (!first_actor->isRigidStatic()) {
						finalQuat = XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(first_actor->getGlobalPose().q)));
					}

					// Get the needle component and initialize it
					TCompNeedle* new_e_needle2 = new_needle_2->get<TCompNeedle>();

					/*new_e_needle2->create(
						blockHit.actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(offset_2) : physics_manager.PxVec3ToXMVECTOR(blockHit.position)
						, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(blockHit.actor->getGlobalPose().q)))
						, rigidbody_e->get<TCompRigidBody>()
						);*/

					new_e_needle2->create(
						hit_actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(offset_2) : physics_manager.PxVec3ToXMVECTOR(hit_position)
						, finalQuat
						, hit_actor
						);

					// -------------- If the distance joint doesn't exists, create one

					TCompDistanceJoint* joint = rope_entity->get<TCompDistanceJoint>();
					if (!joint) {
						// Create a new distance joint
						TCompDistanceJoint* new_e_j = CHandle::create<TCompDistanceJoint>();

						new_e_j->create(first_actor, hit_actor, 1, first_position, hit_position, physx::PxTransform(offset_1), physx::PxTransform(offset_2));

						rope_entity->add(new_e_j);
					}

					// If the distance joint already exists, update it
					else {
						joint->joint->release();
						joint->create(first_actor, hit_actor, 1, first_position, hit_position, physx::PxTransform(offset_1), physx::PxTransform(offset_2));
					}

					// -------------- Update the rope component
					TCompRope* new_e_r = rope_entity->get<TCompRope>();

					//Add the needle and the rope to the vector from item_manager
					Citem_manager::get().addNeedleToVector(new_e_needle2, new_e_r);

					TCompTransform* first_needle_transform = first_needle_entity->get<TCompTransform>();

					// Get the transform of the needle
					TCompTransform* second_needle_transform = new_needle_2->get<TCompTransform>();
					second_needle_transform->init();

					new_e_r->setPositions(first_needle_transform, second_needle_transform);

					bool second_is_enemy = false;
					//Checking if enemy tied
					PxRigidActor* second_actor = hit_actor;
					CEntity* secondActorEntity = CHandle(second_actor->userData);
					if (secondActorEntity->hasTag("enemy")){
						TCompSensorTied* tied_sensor = secondActorEntity->get<TCompSensorTied>();
						if (tied_sensor) {
							tied_sensor->changeTiedState(true, CHandle(new_e_r));
						}
						second_is_enemy = true;
					}
					else{
						//adding needle to item manager
						Citem_manager::get().addNeedle(CHandle(new_e_needle2), CHandle(new_e_r));
					}

					// ****************** Aux joint ******************
					TCompDistanceJoint* aux_joint = new_e_r->joint_aux;
					if (aux_joint) {
						PxRigidActor* a1 = nullptr;
						PxRigidActor* a2 = nullptr;

						aux_joint->joint->getActors(a1, a2);

						// Second actor is a wall
						if (!second_is_enemy) {
							aux_joint->joint->release();
							aux_joint->create(a1, hit_actor, 1, a1->getGlobalPose().p, hit_position, physx::PxTransform(PxVec3(0, 0, 0), PxQuat(0, 0, 0, 1)), physx::PxTransform(offset_2));
						}
						// Second actor is an enemy
						else {

							// Get the ragdoll
							TCompRagdoll* ragdoll = secondActorEntity->get<TCompRagdoll>();
							if (ragdoll) {
								// Get the bone 
								PxRigidDynamic* bone = ragdoll->getBoneRigidRaycast(Physics.PxVec3ToXMVECTOR(first_position), camera_transform->getFront());
								if (bone != nullptr)
								{
									// ************ Change needle target ***********
									new_e_needle2->create(
										XMVectorSet(0, 0, 0, 0)
										, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(bone->getGlobalPose().q)))
										, bone
										);

									// ************ Change existing joint ***********
									aux_joint->joint->release();
									if (a1 != bone) {
										aux_joint->create(a1, bone, 1, a1->getGlobalPose().p, bone->getGlobalPose().p, physx::PxTransform(PxVec3(0, 0, 0), PxQuat(0, 0, 0, 1)), physx::PxTransform(PxVec3(0, 0, 0), PxQuat(0, 0, 0, 1)));
									}
									else {
										CEntityManager::get().remove(CHandle(aux_joint).getOwner());
										aux_joint->joint = nullptr;
										aux_joint = CHandle();
										new_e_r->joint_aux = CHandle();
									}
								}
							}
						}
					}
					else
					{
						// Second actor is an enemy, but first one is a wall
						// Get the ragdoll
						TCompRagdoll* ragdoll = secondActorEntity->get<TCompRagdoll>();
						if (ragdoll) {
							// Get the bone 
							PxRigidDynamic* bone = ragdoll->getBoneRigidRaycast(Physics.PxVec3ToXMVECTOR(first_position), camera_transform->getFront());
							if (bone != nullptr)
							{
								// ************ Change needle target ***********
								new_e_needle2->create(
									XMVectorSet(0, 0, 0, 0)
									, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(bone->getGlobalPose().q)))
									, bone
									);

								// ************ Create aux entity ************
								CEntity* aux_e = entity_manager.createEmptyEntity();

								TCompName* new_e_name = CHandle::create<TCompName>();
								std::strcpy(new_e_name->name, ("JointAux" + to_string(entitycount)).c_str());
								aux_e->add(new_e_name);

								// ************ Create aux joint ************
								TCompDistanceJoint* aux_joint = CHandle::create<TCompDistanceJoint>();

								// The first position is the actor position with offset in world coords
								// TODO: Get the real position, to get a valid offset. Temporally, no offset.
								PxVec3 aux_pos = bone->getGlobalPose().p;
								PxVec3 aux_offset = bone->getGlobalPose().q.rotateInv(aux_pos - bone->getGlobalPose().p);

								PxVec3 pos = bone->getGlobalPose().q.rotate(aux_offset) + bone->getGlobalPose().p;

								if (hit_actor != bone) {
									aux_joint->create(bone, hit_actor, 1, bone->getGlobalPose().p, hit_position, physx::PxTransform(aux_offset), physx::PxTransform(offset_2));
									aux_e->add(aux_joint);
								}
								else {
									CEntityManager::get().remove(aux_e);
									aux_joint->joint = nullptr;
									aux_joint = CHandle();
								}

								if (aux_joint == CHandle())
									new_e_r->joint_aux = CHandle();
								else
									new_e_r->joint_aux = CHandle(aux_joint);
							}
						}


					}

					// Set the current rope entity as an invalid Handle
					current_rope_entity = CHandle();

					first_actor = nullptr;
					first_needle = CHandle();
					entitycount++;

				}
				// The string can't be thrown (same actor or two static bodies)
				else {
					// Visual effects when the string can't be thrown
				}
			}
			//}
		}

	}

	else{
		// Not on enter
		// Make the distance joint from the rope follow the player
		CEntity* rope_entity = current_rope_entity;
		if (rope_entity) {
			TCompRope* rope = rope_entity->get<TCompRope>();
			if (rope) {
				rope->pos_2 = skeleton->getPositionOfBone(29);
			}
		}
	}


	// Animation ends
	if (state_time >= 0.1f) {
		if (current_rope_entity == CHandle())
			ChangeState("fbp_Inactive"); 
		else
			ChangeState("fbp_GrabString");
	}

}

void FSMPlayerTorso::PullString(float elapsed) {
	if (!legs->canThrow)
		return;

	CIOStatus& io = CIOStatus::get();

	if (on_enter) {
		// -------------- Moves the camera to the shoulder
		/*TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
		camera_controller->offset = PxVec3(0.56f, -0.22f, 1.07f);*/
		
		TCompSkeleton* skeleton = comp_skeleton;		
		skeleton->playAnimation(17);

		float distance=0.f;
		CEntity* rope_entity = current_rope_entity;
		if (rope_entity){
			TCompRope* rope = rope_entity->get<TCompRope>();
			if (rope){
				distance = V3DISTANCE(rope->pos_1, rope->pos_2);
			}
		}

		CSoundManager::SoundParameter params[] = {
			{ "LongCuerda", distance }
		};

		CSoundManager::get().playEvent("STRING_GRAB", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
		
	}

	TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
	camera_controller->offset = XMVectorSet(0.56f, -0.22f, 1.07f, 0);// XMVectorLerp(camera_controller->offset, XMVectorSet(0.56f, -0.22f, 1.07f, 0), 0.05f);

	if (current_rope_entity.isValid()) {
		CEntity* rope_entity = current_rope_entity;
		TCompDistanceJoint* joint = rope_entity->get<TCompDistanceJoint>();
		TCompRope* rope = rope_entity->get<TCompRope>();
		TCompSkeleton* skeleton = comp_skeleton;

		rope->pos_2 = skeleton->getPositionOfBone(89);
		CLogicManager::get().stringPulled();

		if (joint) {
			// -------------- Shorten the distance joint			
			float oldDistance = sqrt(joint->joint->getDistance());
			if (oldDistance > 0.1f) {
				joint->joint->setMaxDistance(oldDistance - 1000.f * elapsed);
				rope->max_distance = oldDistance - 1000.f * elapsed;
			}
			else {
				joint->joint->setMaxDistance(0);
				rope->max_distance = 0;
			}

			joint->awakeActors();
			
			if (rope->joint_aux.isValid()) {
				TCompDistanceJoint* joint2 = rope->joint_aux;
				float oldDistance = sqrt(joint2->joint->getDistance());
				joint2->joint->setMaxDistance(oldDistance - 100.f * elapsed);
				joint2->awakeActors();
			}
			
			// -- Send tensed MSG (to enter in ragdoll)
			PxRigidActor* a1 = nullptr;
			PxRigidActor* a2 = nullptr;

			joint->joint->getActors(a1, a2);
			// Wake up the actors, if dynamic
			if (a1 && a1->isRigidDynamic()) {
				((CEntity*)CHandle(a1->userData))->sendMsg(TMsgRopeTensed(0));
			}
			if (a2 && a2->isRigidDynamic()) {
				((CEntity*)CHandle(a2->userData))->sendMsg(TMsgRopeTensed(0));
			}

			// Se the joints position
			if (joint) {
				joint->joint->setLocalPose(PxJointActorIndex::eACTOR1, PxTransform(Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(29))));
				joint->awakeActors();
			}

			if (rope->joint_aux.isValid()) {
				TCompDistanceJoint* joint2 = rope->joint_aux;
				joint2->joint->setLocalPose(PxJointActorIndex::eACTOR1, PxTransform(Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(29))));
				joint2->awakeActors();
			}
		}
	}

	// Animation ends
	if (io.isReleased(CIOStatus::PULL_STRING)) {	
		TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
		camera_controller->offset = standard_camera_offset;

		// Remove the rope
		/*CHandle c_rope = strings.back();
		strings.pop_front();
		CEntityManager::get().remove(c_rope.getOwner());*/
		if (first_throw)
			CRope_manager::get().removeBackString();

		// Reset the variables
		current_rope_entity = CHandle();
		first_actor = nullptr;
		first_needle = CHandle();
		entitycount++;

		ChangeState("fbp_Inactive");		
	}
}

void FSMPlayerTorso::GrabString(float elapsed) {

	CIOStatus& io = CIOStatus::get();

	/**/
	if (!current_rope_entity.isValid()) {

		// Remove the current string
		//CHandle c_rope = strings.back();
		//strings.pop_back();
		//CEntityManager::get().remove(c_rope.getOwner());
		CRope_manager::get().removeString(current_rope_entity);

		// Reset the variables
		current_rope_entity = CHandle();
		first_actor = nullptr;
		first_needle = CHandle();
		entitycount++;

		up_animation = false;
		ChangeState("fbp_Inactive");
		return;
	}

	if (on_enter)
	{
		//CSoundManager::get().playFX("string_grab_8");
	}
	up_animation = true;

	TCompSkeleton* skeleton = comp_skeleton;

	// Make the distance joint from the rope follow the player
	CEntity* rope_entity = current_rope_entity;
	TCompRope* rope = rope_entity->get<TCompRope>();
	TCompDistanceJoint* joint = rope_entity->get<TCompDistanceJoint>();

	// Get the player transform
	TCompTransform* p_transform = comp_transform;

	// Set the rope position
	rope->pos_2 = skeleton->getPositionOfBone(29);

	// Se the joints position
	if (joint) {
		joint->joint->setLocalPose(PxJointActorIndex::eACTOR1, PxTransform(Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(29))));
		joint->awakeActors();
	}

	if (rope->joint_aux.isValid()) {
		TCompDistanceJoint* joint2 = rope->joint_aux;
		joint2->joint->setLocalPose(PxJointActorIndex::eACTOR1, PxTransform(Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(29))));
		joint2->awakeActors();
	}

	

	// Cancel
	

	if (io.becomesReleased(CIOStatus::CANCEL_STRING) && can_cancel) {

		// Remove the current string
		/*CHandle c_rope = strings.back();
		strings.pop_back();
		CEntityManager::get().remove(c_rope.getOwner());*/
		CRope_manager::get().removeBackString();

		skeleton->playAnimation(31);

		// Reset the variables
		current_rope_entity = CHandle();
		first_actor = nullptr;
		first_needle = CHandle();
		first_throw = false;
		entitycount++;
		
		up_animation = false;
		ChangeState("fbp_Inactive");		
	}

	// Throw the second needle
	if (io.becomesReleased(CIOStatus::THROW_STRING) && canThrow() && can_throw) {
		up_animation = false;
		ChangeState("fbp_ThrowString");
	}

	if (legs->getCurrentNode() != "fbp_Idle")
		return;

	// Pull
	if (io.isPressed(CIOStatus::PULL_STRING) && can_pull) {
		up_animation = false;
		ChangeState("fbp_PullString");
	}
	
}

void FSMPlayerTorso::Inactive(float elapsed) {

	if (!legs->canThrow)
		return;

	CIOStatus& io = CIOStatus::get();
	CEntityManager &entity_manager = CEntityManager::get();
	TCompSkeleton* skeleton = comp_skeleton;

	//Calculate the current number of strings
	//unsigned int num_strings = getStringCount();

	// Mimic the legs animation

	// Cancel previous strings
	// Simple cancel
	if (io.becomesReleased(CIOStatus::CANCEL_STRING) && can_cancel) {
		float time_prueba = io.getTimePressed(CIOStatus::CANCEL_STRING);
		if (io.getTimePressed(CIOStatus::CANCEL_STRING) < .5f && CRope_manager::get().getRopeCount() > 0){ //&& num_strings > 0) {
			/*CHandle c_rope = strings.back();
			strings.pop_back();
			entity_manager.remove(c_rope.getOwner());*/
			CRope_manager::get().removeBackString();
			CLogicManager::get().stringCancelled();
			skeleton->playAnimation(31);			
		}
	}

	// Multiple cancel
	if (io.isPressed(CIOStatus::CANCEL_STRING) && can_cancel) {
		if (io.getTimePressed(CIOStatus::CANCEL_STRING) >= .5f && CRope_manager::get().getRopeCount() > 0){ //&& num_strings > 0) {
			CRope_manager::get().clearStrings();
			CLogicManager::get().stringAllCancelled();
			skeleton->playAnimation(31);
			/*strings.clear();
			for (int i = 0; i < entity_manager.getEntities().size(); ++i)
			{
				TCompRope* rope = ((CEntity*)entity_manager.getEntities()[i])->get<TCompRope>();

				if (rope) {
					entity_manager.remove(CHandle(rope).getOwner());
				}
			}*/
		}
	}

	// Tense the string
	if (io.becomesPressed(CIOStatus::TENSE_STRING) && can_tense) {

		CLogicManager::get().stringsTensed();

		// TODO: ¡Se están tensado TODOS los distance joint, no los que dependan de ropes!
		skeleton->playAnimation(32);
		
		for (int i = 0; i < entity_manager.getEntities().size(); ++i)
		{
			TCompRope* rope = ((CEntity*)entity_manager.getEntities()[i])->get<TCompRope>();			
			TCompDistanceJoint* djoint = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceJoint>();

			if (rope && djoint) {
				if (!rope->tensed) {
					//CSoundManager::SoundParameter params[] = {
						//{ "type", 1 }
					//};

					//CSoundManager::get().playEvent("event:/Strings/stringEvents", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
					//Sound with distance --------------------------------------------------------------------------------------------------------
					float distance = 0.f;
					if (rope){
						distance = V3DISTANCE(rope->pos_1, rope->pos_2);
					}

					CSoundManager::SoundParameter params[] = {
						{ "LongCuerda", distance }
					};

					CSoundManager::get().playEvent("STRING_TENSE", params, sizeof(params) / sizeof(CSoundManager::SoundParameter));
					//----------------------------------------------------------------------------------------------------------------------------
					
					rope->tensed = true;
					djoint->joint->setMaxDistance(0.1f);
					PxRigidActor* a1 = nullptr;
					PxRigidActor* a2 = nullptr;

					if (rope->joint_aux.isValid()) {
						TCompDistanceJoint* joint2 = rope->joint_aux;
						joint2->joint->setMaxDistance(0.1f);
						joint2->awakeActors();
					}

					djoint->joint->getActors(a1, a2);
					// Wake up the actors, if dynamic
					if (a1 && a1->isRigidDynamic()) {
						if (!((physx::PxRigidDynamic*)a1)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))  {
							((physx::PxRigidDynamic*)a1)->wakeUp();
						}

						((CEntity*)CHandle(a1->userData))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));

						//((CEntity*)entity_manager.getByName(a1->getName()))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
					}
					if (a2 && a2->isRigidDynamic()) {
						if (!((physx::PxRigidDynamic*)a2)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))  {
							((physx::PxRigidDynamic*)a2)->wakeUp();
						}
						((CEntity*)CHandle(a2->userData))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
						//((CEntity*)entity_manager.getByName(a2->getName()))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
					}
				}
			}
		}
	}

	// Waits for the player to throw
	if (io.isPressed(CIOStatus::THROW_STRING) && canThrow() && can_throw) {
		ChangeState("fbp_ThrowString");
	}

	/*if (io.isPressed(CIOStatus::CLUE_BUTTON)) {
		XMVECTOR& point = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		bool inside = CLogicManager::get().playerInsideGNZone(point, GNLogic);
		if ((inside) || (!first_throw)) {
			ChangeState("fbp_ThrowGoldenNeedle");
			golden_needle_point = point;
		}
	}*/
}

void FSMPlayerTorso::ProcessHit(float elapsed) {
	CIOStatus& io = CIOStatus::get();

}

void FSMPlayerTorso::getThrowingData(PxActor* &the_hit_actor, PxVec3 &the_actor_position, PxVec3 &the_actor_normal) {
	TCompTransform* camera_transform = ((CEntity*)camera_entity)->get<TCompTransform>();

	// Raycast all
	PxRaycastBuffer buf;	
	Physics.raycastAll(camera_transform->position, camera_transform->getFront(), 1000, buf);

	float max_dist = 1000000;
	PxActor* hit_actor = nullptr;
	PxVec3 actor_position = PxVec3(0, 0, 0);
	PxVec3 actor_normal = PxVec3(0, 0, 0);
	bool priority_entity = false;

	// Loop variables
	CEntity* entity = nullptr;
	float dist;
	bool is_priority_entity = false;

	for (int i = 0; i < (int)buf.nbTouches; i++)
	{
		if (std::strcmp(buf.touches[i].actor->getName(), "Player") != 0) {
			// Reset variables
			is_priority_entity = false;
			entity = nullptr;

			// Check distance
			dist = V3DISTANCE(Physics.PxVec3ToXMVECTOR(buf.touches[i].position), camera_transform->position);			

			// Check if the entity has priority
			entity = CHandle(buf.touches[i].actor->userData);
			if (entity) {
				is_priority_entity = entity->hasCollisionTag("Boss_Part");
			}				

			// If is priority and isn't the first one or is not priority and there isn't a priority entity, act as usually
			if (is_priority_entity == priority_entity) {
				if (dist < max_dist) {
					actor_position = buf.touches[i].position;
					actor_normal = buf.touches[i].normal;
					hit_actor = buf.touches[i].actor;
					max_dist = dist;
				}
				if (is_priority_entity) { priority_entity = true; }
			}
			else if (is_priority_entity) {
				// Else, if the current entity is priority, override the current actor
				actor_position = buf.touches[i].position;
				actor_normal = buf.touches[i].normal;
				hit_actor = buf.touches[i].actor;
				max_dist = dist;
				priority_entity = true;
			}

			// Old code
			/*if (dist < max_dist) {
				actor_position = buf.touches[i].position;
				actor_normal = buf.touches[i].normal;
				hit_actor = buf.touches[i].actor;
				max_dist = dist;
			}*/
		}
	}

	the_hit_actor = hit_actor;
	the_actor_position = actor_position;
	the_actor_normal = actor_normal;
}

bool FSMPlayerTorso::canThrow() {
	TCompTransform* camera_transform = ((CEntity*)camera_entity)->get<TCompTransform>();
	TCompTransform* comp_trans = ((CEntity*)entity)->get<TCompTransform>();
	TCompSkeletonLookAt* comp_lookat = ((CEntity*)entity)->get<TCompSkeletonLookAt>();

	TTransform head = *comp_trans;
	float player_y = XMVectorGetY(comp_trans->position);
	XMVectorSetY(head.position, player_y + 1.7f);

	XMVECTOR head_front = head.position + head.getFront() + head.getUp() * 1.6f;

	PxActor* hit_actor = nullptr;
	PxVec3 actor_position;
	PxVec3 actor_normal;

	getThrowingData(hit_actor, actor_position, actor_normal);

	if (hit_actor == nullptr) {
		comp_lookat->target = XMVectorLerp(comp_lookat->target, head_front, 0.05f);
		looking_at_pointer = false;
		return false;
	}

	XMVECTOR pos = Physics.PxVec3ToXMVECTOR(actor_position);
	float pos_y = XMVectorGetY(pos);
	XMVECTOR pos_aux = XMVectorSetY(pos, XMVectorGetY(head.position));
	XMVECTOR target = comp_lookat->target;

	bool inFovBig = head.isInFov(pos_aux, deg2rad(170));
	bool inFovSmall = head.isInFov(pos_aux, deg2rad(110));

	if ((inFovSmall) || (looking_at_pointer && inFovBig)) {
		TCompSkeletonLookAt* comp_lookat = ((CEntity*)entity)->get<TCompSkeletonLookAt>();
		//comp_lookat->target = XMVectorLerp(comp_lookat->target, pos, 0.03f);
		target = pos;
		looking_at_pointer = true;
	}
	else {
		//comp_lookat->target = XMVectorLerp(comp_lookat->target, head_front, 0.03f);
		target = head_front;
		looking_at_pointer = false;
	}

	comp_lookat->target = XMVectorLerp(comp_lookat->target, target, 0.03f);

	if (legs->getState() == "fbp_Idle")
		comp_lookat->active = true;
	else
		comp_lookat->active = false;

	// First throw
	if (first_actor == nullptr) {
		return true;
	}
	else {
		// The string can be thrown
		if ((hit_actor != first_actor) && !(hit_actor->isRigidStatic() && first_actor->isRigidStatic())) {
			return true;
		}
	}
		
	return false;
}

void FSMPlayerTorso::CancelGrabString() {

	if (first_throw)
		CRope_manager::get().removeBackString();

	// Reset the variables
	current_rope_entity = CHandle();
	first_actor = nullptr;
	first_needle = CHandle();
	first_throw = false;
	entitycount++;

	up_animation = false;
}