#include "mcv_platform.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "comp_transform.h"
#include "entity_manager.h"
#include "rope_manager.h"
#include "physics_manager.h"
#include "comp_needle.h"

TCompRope::~TCompRope() {
	CHandle valid_trans_1 = transform_1_aux.isValid() ? transform_1_aux : transform_1;
	CHandle valid_trans_2 = transform_2_aux.isValid() ? transform_2_aux : transform_2;

	if (valid_trans_1.isValid()) {

		TCompNeedle* needle = ((CEntity*)valid_trans_1.getOwner())->get<TCompNeedle>();
		if (needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

			if (attached_transform->getType() == 95)
				attached_transform->setType(1);
			if (attached_transform->getType() == 90)
				attached_transform->setType(0.8f);
		}
	}

	// Update the second pos
	if (valid_trans_2.isValid()) {
		TCompNeedle* needle = ((CEntity*)valid_trans_2.getOwner())->get<TCompNeedle>();
		if (needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

			if (attached_transform->getType() == 95)
				attached_transform->setType(1);
			if (attached_transform->getType() == 90)
				attached_transform->setType(0.8f);
		}
	}
}

void TCompRope::setPositions(CHandle the_transform_1, XMVECTOR the_pos_2) {

	transform_1 = the_transform_1;
	pos_2 = the_pos_2;
}

void TCompRope::setPositions(XMVECTOR the_pos_1, CHandle the_transform_2) {

	pos_1 = the_pos_1;
	transform_2 = the_transform_2;
}

// With two transforms, a distance joiny is required
void TCompRope::setPositions(CHandle the_transform_1, CHandle the_transform_2) {
	joint = assertRequiredComponent<TCompDistanceJoint>(this);

	transform_1 = the_transform_1;
	transform_2 = the_transform_2;
}

void TCompRope::loadFromAtts(const std::string& elem, MKeyValue& atts) {
	joint = assertRequiredComponent<TCompDistanceJoint>(this);

	width = atts.getFloat("width", 0.02f);
	max_distance = atts.getFloat("maxDistance", 20);
}

void TCompRope::fixedUpdate(float elapsed) {

	CHandle valid_trans_1 = transform_1_aux.isValid() ? transform_1_aux : transform_1;
	CHandle valid_trans_2 = transform_2_aux.isValid() ? transform_2_aux : transform_2;

	// Update the first pos
	if (valid_trans_1.isValid()) {
		TCompTransform* trans_1 = valid_trans_1;

		XMVECTOR normal_dir = trans_1->position - pos_1;
		normal_dir = XMVector3Normalize(normal_dir);
		float dist = V3DISTANCE(trans_1->position, pos_1);
		float speed = min(dist / elapsed, 50);

		pos_1 = pos_1 + normal_dir * speed * elapsed;

		TCompNeedle* needle = ((CEntity*)valid_trans_1.getOwner())->get<TCompNeedle>();
		if (needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

			if (attached_transform->getType() == 80)
				attached_transform->setType(0.9f);
			if (attached_transform->getType() == 100)
				attached_transform->setType(0.95f);
		}

		//pos_1 = trans_1->position;
	}

	// Update the second pos
	if (valid_trans_2.isValid()) {
		TCompTransform* trans_2 = valid_trans_2;

		XMVECTOR normal_dir = trans_2->position - pos_2;
		normal_dir = XMVector3Normalize(normal_dir);
		float dist = V3DISTANCE(trans_2->position, pos_2);
		float speed = min(dist / elapsed, 50);

		pos_2 = pos_2 + normal_dir * speed * elapsed;

		TCompNeedle* needle = ((CEntity*)valid_trans_2.getOwner())->get<TCompNeedle>();
		if (needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

			if (attached_transform->getType() == 80)
				attached_transform->setType(0.9f);
			if (attached_transform->getType() == 100)
				attached_transform->setType(0.95f);
		}

		//pos_2 = trans_2->position;
	}


	// Rope collisions
	
	return;


	CHandle e1 = CHandle();
	CHandle e2 = CHandle();
	CHandle e3 = CEntityManager::get().getByName("Player");

	if (valid_trans_1.isValid()) {
		e1 = valid_trans_1.getOwner();
	}
	if (valid_trans_2.isValid()) {
		e2 = valid_trans_2.getOwner();
	}

	XMVECTOR initialPos = pos_1;
	XMVECTOR dir = XMVector3Normalize(pos_2 - pos_1);
	float length = V3DISTANCE(pos_2, pos_1);

	PxRaycastBuffer buf;

	Physics.raycastAll(initialPos, dir, length, buf);
	float force_s = 100;

	bool colliding = false;
	for (int i = 0; i < buf.nbTouches; i++)
	{
		CHandle e(buf.touches[i].actor->userData);
		if (!(e == e1) && !(e == e2) && !(e == e3)) {
			if (buf.touches[i].actor->isRigidBody()) {
				/*if (((physx::PxRigidBody*)buf.touches[i].actor)->getMass() < 100) {

					physx::PxRigidBody* body = ((physx::PxRigidBody*)buf.touches[i].actor);

					physx::PxVec3 force = (body->getGlobalPose().p - buf.touches[i].position).getNormalized();

					physx::PxVec3 velocity = physx::PxRigidBodyExt::getVelocityAtPos(*body, buf.touches[i].position);

					physx::PxRigidBodyExt::addForceAtPos(*body, -velocity, buf.touches[i].position, physx::PxForceMode::eVELOCITY_CHANGE);
					//((physx::PxRigidBody*)buf.touches[i].actor)->addForce(force * force_s, physx::PxForceMode::eIMPULSE);
				}
				else
				{
					int a = 2;
					//CEntityManager::get().remove(CHandle(this).getOwner());
				}*/
			}
			else
			{
				// Remove the string
				remove_counter += elapsed;
				colliding = true;
				if (remove_counter > 0.5f) {
					CRope_manager::get().removeString(CHandle(this));
					int b = 2;
				}
			}
		}
	}

	if (!colliding)
		remove_counter = 0;

}

bool TCompRope::getStaticPosition(XMVECTOR& position) {
	CHandle valid_trans_1 = transform_1_aux.isValid() ? transform_1_aux : transform_1;
	CHandle valid_trans_2 = transform_2_aux.isValid() ? transform_2_aux : transform_2;

	// Update the first pos
	if (valid_trans_1.isValid()) {
		TCompNeedle* needle = ((CEntity*)valid_trans_1.getOwner())->get<TCompNeedle>();
		if (needle->getAttachedRigid() == nullptr) {
			position = pos_1;
			return true;
		}
		else {
			if (needle->getAttachedRigid()->isRigidStatic()) {
				position = pos_1;
				return true;
			}
		}
	}

	// Update the second pos
	if (valid_trans_2.isValid()) {
		TCompNeedle* needle = ((CEntity*)valid_trans_2.getOwner())->get<TCompNeedle>();
		if (needle->getAttachedRigid() == nullptr) {
			position = pos_2;
			return true;
		}
		else {
			if (needle->getAttachedRigid()->isRigidStatic()) {
				position = pos_2;
				return true;
			}
		}
	}

	return false;
}