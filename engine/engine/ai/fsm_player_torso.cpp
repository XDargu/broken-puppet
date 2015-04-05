#include "mcv_platform.h"
#include "fsm_player_torso.h"
#include "io\iostatus.h"
#include "components\all_components.h"
#include "handle\prefabs_manager.h"

FSMPlayerTorso::FSMPlayerTorso()
	: can_move(true)
	, comp_transform(CHandle())
	, entitycount(1)
	, first_actor(nullptr)
	, first_needle(CHandle())
	, first_offset(PxVec3(0, 0, 0))
	, first_position(PxVec3(0, 0, 0))
	, standard_camera_offset(PxVec3(0, 0, 0))
	, first_throw(false)
	, max_num_string(0)
{}
FSMPlayerTorso::~FSMPlayerTorso() {}

void FSMPlayerTorso::Init() {
	// Insert all states in the map
	AddState("fbp_ThrowString", (statehandler)&FSMPlayerTorso::ThrowString);
	AddState("fbp_PullString", (statehandler)&FSMPlayerTorso::PullString);
	AddState("fbp_GrabString", (statehandler)&FSMPlayerTorso::GrabString);	
	AddState("fbp_Inactive", (statehandler)&FSMPlayerTorso::Inactive);
	AddState("fbp_ProcessHit", (statehandler)&FSMPlayerTorso::ProcessHit);
	
	// Reset the state
	ChangeState("fbp_Inactive");

	// Get the transform
	comp_transform = ((CEntity*)entity)->get<TCompTransform>();

	// Get tge camera
	camera_entity = CEntityManager::get().getByName("PlayerCamera");
	TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
	standard_camera_offset = camera_controller->offset;

	max_num_string = 4;

	can_move = true;
}

void FSMPlayerTorso::ThrowString(float elapsed) {
	CIOStatus& io = CIOStatus::get();

	// Throw the string
	if (on_enter) {

		CEntityManager &entity_manager = CEntityManager::get();
		CPhysicsManager &physics_manager = CPhysicsManager::get();
		
		TCompTransform* camera_transform = ((CEntity*)camera_entity)->get<TCompTransform>();

		// Raycast detecting the collider the mouse is pointing at
		PxRaycastBuffer hit;
		Physics.raycast(camera_transform->position, camera_transform->getFront(), 1000, hit);

		if (hit.hasBlock) {
			PxRaycastHit blockHit = hit.block;

			// First throw
			if (first_actor == nullptr) {
				first_throw = true;

				first_actor = blockHit.actor;
				first_position = blockHit.position;
				first_offset = first_actor->getGlobalPose().q.rotateInv(blockHit.position - first_actor->getGlobalPose().p);

				unsigned int num_strings = getStringCount();

				// If there are more strings than the maximun available, remove the oldest one
				if (num_strings >= max_num_string){
					CHandle c_rope = strings.front();
					strings.pop_front();
					entity_manager.remove(c_rope.getOwner());
				}

				// Get the needle prefab
				CEntity* new_needle = prefabs_manager.getInstanceByName("Needle");

				// Get the entity of the rigidbody on wich the needle is pierced
				CEntity* rigidbody_e = entity_manager.getByName(blockHit.actor->getName());

				// Rename the needle
				TCompName* new_needle_name = new_needle->get<TCompName>();
				std::strcpy(new_needle_name->name, ("Needle" + to_string(entitycount)).c_str());

				// Set the rotation of the needle according to the camera angle and normal of the surface
				// The final rotation will be a quaternion between those two values, wigthed in one or other direction
				XMVECTOR rotation;
				if (first_position == physics_manager.XMVECTORToPxVec3(camera_transform->position)) {
					XMMATRIX view = XMMatrixLookAtRH(camera_transform->position, camera_transform->position - (physics_manager.PxVec3ToXMVECTOR(first_position + physics_manager.XMVECTORToPxVec3(camera_transform->getFront() * 0.01f)) - camera_transform->position), XMVectorSet(0, 1, 0, 0));
					rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
				}
				else {
					XMMATRIX view = XMMatrixLookAtRH(camera_transform->position, camera_transform->position - (physics_manager.PxVec3ToXMVECTOR(first_position) - camera_transform->position), XMVectorSet(0, 1, 0, 0));
					rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
				}

				XMMATRIX view_normal = XMMatrixLookAtRH(physics_manager.PxVec3ToXMVECTOR(first_position - blockHit.normal), physics_manager.PxVec3ToXMVECTOR(first_position), XMVectorSet(0, 1, 0, 0));
				XMVECTOR normal_rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view_normal));
				XMVECTOR finalQuat = XMQuaternionSlerp(rotation, normal_rotation, 0.35f);

				// Get the needle component and initialize it
				TCompNeedle* new_e_needle = new_needle->get<TCompNeedle>();

				new_e_needle->create(
					first_actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(first_offset) : physics_manager.PxVec3ToXMVECTOR(first_position)
					, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(first_actor->getGlobalPose().q)))
					, rigidbody_e->get<TCompRigidBody>()
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

				// Get the transform of the needle
				TCompTransform* needle_transform = new_needle->get<TCompTransform>();

				// Assing the positions (needle transform + current player position)
				new_e_r->setPositions(needle_transform, p_transform->position);

				// Set the distance joint of the needle as the current one (to move it while grabbing and pulling the string)
				current_rope_entity = new_e;

				// Add the string to the strings vector
				strings.push_back(CHandle(new_e_r));

				entitycount++;
			}
			// Second throw
			else {
				// The string can be thrown
				if ((blockHit.actor != first_actor) && !(blockHit.actor->isRigidStatic() && first_actor->isRigidStatic())) {
					first_throw = false;

					// -------------- Get the offsets
					// Obtener el offset con coordenadas de mundo = (Offset_mundo - posición) * inversa(rotación)
					PxVec3 offset_1 = first_offset;
					PxVec3 offset_2 = blockHit.actor->getGlobalPose().q.rotateInv(blockHit.position - blockHit.actor->getGlobalPose().p);

					// -------------- Whats here so far:
					// - We have an entity with a rope component, pointing to a needle transform and a position
					// - A distance joint may or may not exists in this entity
					CEntity* rope_entity = current_rope_entity;
					CEntity* first_needle_entity = first_needle;

					// -------------- Create the new needle
					// Get the needle prefab
					CEntity* new_needle_2 = prefabs_manager.getInstanceByName("Needle");

					// Get the entity of the rigidbody on wich the needle is pierced
					CEntity* rigidbody_e = entity_manager.getByName(blockHit.actor->getName());

					// Rename the needle
					TCompName* new_e_name2 = new_needle_2->get<TCompName>();
					std::strcpy(new_e_name2->name, ("Needle" + to_string(entitycount)).c_str());

					// Set the rotation of the needle according to the camera angle and normal of the surface
					// The final rotation will be a quaternion between those two values, wigthed in one or other direction
					XMVECTOR rotation;
					if (blockHit.position == physics_manager.XMVECTORToPxVec3(camera_transform->position)) {
						XMMATRIX view = XMMatrixLookAtRH(camera_transform->position, camera_transform->position - (physics_manager.PxVec3ToXMVECTOR(blockHit.position + physics_manager.XMVECTORToPxVec3(camera_transform->getFront() * 0.01f)) - camera_transform->position), XMVectorSet(0, 1, 0, 0));
						rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
					}
					else {
						XMMATRIX view = XMMatrixLookAtRH(camera_transform->position, camera_transform->position - (physics_manager.PxVec3ToXMVECTOR(blockHit.position) - camera_transform->position), XMVectorSet(0, 1, 0, 0));
						rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
					}

					XMMATRIX view_normal = XMMatrixLookAtRH(physics_manager.PxVec3ToXMVECTOR(blockHit.position - blockHit.normal), physics_manager.PxVec3ToXMVECTOR(blockHit.position), XMVectorSet(0, 1, 0, 0));
					XMVECTOR normal_rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view_normal));
					XMVECTOR finalQuat = XMQuaternionSlerp(rotation, normal_rotation, 0.35f);

					// Get the needle component and initialize it
					TCompNeedle* new_e_needle2 = new_needle_2->get<TCompNeedle>();

					new_e_needle2->create(
						blockHit.actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(offset_2) : physics_manager.PxVec3ToXMVECTOR(blockHit.position)
						, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(blockHit.actor->getGlobalPose().q)))
						, rigidbody_e->get<TCompRigidBody>()
					);

					// -------------- If the distance joint doesn't exists, create one

					TCompDistanceJoint* joint = rope_entity->get<TCompDistanceJoint>();
					if (!joint) {
						// Create a new distance joint
						TCompDistanceJoint* new_e_j = CHandle::create<TCompDistanceJoint>();

						new_e_j->create(first_actor, blockHit.actor, 1, first_position, blockHit.position, physx::PxTransform(offset_1), physx::PxTransform(offset_2));

						rope_entity->add(new_e_j);
					}

					// If the distance joint already exists, update it
					else {
						joint->joint->release();
						joint->create(first_actor, blockHit.actor, 1, first_position, blockHit.position, physx::PxTransform(offset_1), physx::PxTransform(offset_2));
					}

					// -------------- Update the rope component
					TCompRope* new_e_r = rope_entity->get<TCompRope>();
					TCompTransform* first_needle_transform = first_needle_entity->get<TCompTransform>();
					
					// Get the transform of the needle
					TCompTransform* second_needle_transform = new_needle_2->get<TCompTransform>();

					new_e_r->setPositions(first_needle_transform, second_needle_transform);
										

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
		}
	}

	// Animation ends
	if (state_time >= 0.1f) {
		if (first_throw)
			ChangeState("fbp_GrabString");
		else
			ChangeState("fbp_Inactive");
	}

}

void FSMPlayerTorso::PullString(float elapsed) {
	CIOStatus& io = CIOStatus::get();

	if (on_enter) {
		// -------------- Moves the camera to the shoulder
		TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
		camera_controller->offset = PxVec3(0.56, -0.22f, 1.07f);	
	}

	CEntity* rope_entity = current_rope_entity;
	TCompDistanceJoint* joint = rope_entity->get<TCompDistanceJoint>();
	TCompRope* rope = rope_entity->get<TCompRope>();

	if (joint) {
		// -------------- Shorten the distance joint
		float oldDistance = sqrt(joint->joint->getDistance());
		if (oldDistance > 0.1f) {
			joint->joint->setMaxDistance(oldDistance - 10.f * elapsed);
			rope->max_distance = oldDistance - 10.f * elapsed;
		}
		else {
			joint->joint->setMaxDistance(0);
			rope->max_distance = 0;
		}

		joint->awakeActors();
	}

	// Animation ends
	if (io.isReleased(CIOStatus::PULL_STRING)) {
		TCompThirdPersonCameraController* camera_controller = ((CEntity*)camera_entity)->get<TCompThirdPersonCameraController>();
		camera_controller->offset = standard_camera_offset;

		// Remove the rope
		CHandle c_rope = strings.front();
		strings.pop_front();
		CEntityManager::get().remove(c_rope.getOwner());

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

	// Make the distance joint from the rope follow the player
	CEntity* rope_entity = current_rope_entity;
	TCompRope* rope = rope_entity->get<TCompRope>();
	TCompDistanceJoint* joint = rope_entity->get<TCompDistanceJoint>();

	// Get the player transform
	TCompTransform* p_transform = comp_transform;

	if (joint) {
		joint->joint->setLocalPose(PxJointActorIndex::eACTOR1, PxTransform(Physics.XMVECTORToPxVec3(p_transform->position + XMVectorSet(0, 2, 0, 0))));
		joint->awakeActors();
	}

	rope->pos_2 = p_transform->position + XMVectorSet(0, 2, 0, 0);

	// Cancel
	if (io.becomesReleased(CIOStatus::CANCEL_STRING)) {

		// Remove the current string
		CHandle c_rope = strings.back();
		strings.pop_back();
		CEntityManager::get().remove(c_rope.getOwner());

		// Reset the variables
		current_rope_entity = CHandle();
		first_actor = nullptr;
		first_needle = CHandle();
		entitycount++;

		ChangeState("fbp_Inactive");		
	}

	// Pull
	if (io.isPressed(CIOStatus::PULL_STRING)) {
		ChangeState("fbp_PullString");
	}

	// Throw the second needle
	if (io.becomesReleased(CIOStatus::THROW_STRING)) {

		ChangeState("fbp_ThrowString");
	}
	
}

void FSMPlayerTorso::Inactive(float elapsed) {

	if (!can_move)
		return;

	CIOStatus& io = CIOStatus::get();
	CEntityManager &entity_manager = CEntityManager::get();

	//Calculate the current number of strings
	unsigned int num_strings = getStringCount();

	// Mimic the legs animation

	// Cancel previous strings
	// Simple cancel
	if (io.becomesReleased(CIOStatus::CANCEL_STRING)) {

		if (io.getTimePressed(CIOStatus::CANCEL_STRING) < .5f  && num_strings > 0) {
			CHandle c_rope = strings.back();
			strings.pop_back();
			entity_manager.remove(c_rope.getOwner());
		}
	}

	// Multiple cancel
	if (io.isPressed(CIOStatus::CANCEL_STRING)) {
		if (io.getTimePressed(CIOStatus::CANCEL_STRING) >= .5f && num_strings > 0) {
			strings.clear();
			for (int i = 0; i < entity_manager.getEntities().size(); ++i)
			{
				TCompRope* rope = ((CEntity*)entity_manager.getEntities()[i])->get<TCompRope>();

				if (rope) {
					entity_manager.remove(CHandle(rope).getOwner());
				}
			}
		}
	}

	// Tense the string
	if (io.becomesPressed(CIOStatus::TENSE_STRING)) {
		for (int i = 0; i < entity_manager.getEntities().size(); ++i)
		{
			TCompDistanceJoint* djoint = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceJoint>();

			if (djoint) {
				djoint->joint->setMaxDistance(0.1f);
				PxRigidActor* a1 = nullptr;
				PxRigidActor* a2 = nullptr;

				djoint->joint->getActors(a1, a2);
				// Wake up the actors, if dynamic
				if (a1 && a1->isRigidDynamic()) {
					((physx::PxRigidDynamic*)a1)->wakeUp();
					((CEntity*)entity_manager.getByName(a1->getName()))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
				}
				if (a2 && a2->isRigidDynamic()) {
					((physx::PxRigidDynamic*)a2)->wakeUp();
					((CEntity*)entity_manager.getByName(a2->getName()))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
				}
			}
		}
	}

	// Waits for the player to throw
	if (io.isPressed(CIOStatus::THROW_STRING)) {

		ChangeState("fbp_ThrowString");
	}
}

void FSMPlayerTorso::ProcessHit(float elapsed) {
	CIOStatus& io = CIOStatus::get();

}

unsigned int FSMPlayerTorso::getStringCount() {
	unsigned int num_strings = 0;
	for (int i = 0; i < CEntityManager::get().getEntities().size(); ++i){
		TCompRope* c_rope = ((CEntity*)CEntityManager::get().getEntities()[i])->get<TCompRope>();
		if (c_rope){
			num_strings++;
		}
	}
	return num_strings;
}
