#include "mcv_platform.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "comp_transform.h"
#include "entity_manager.h"
#include "rope_manager.h"
#include "physics_manager.h"

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

	// Update the first pos
	if (transform_1.isValid()) {
		TCompTransform* trans_1 = transform_1;

		XMVECTOR normal_dir = trans_1->position - pos_1;
		normal_dir = XMVector3Normalize(normal_dir);
		float dist = V3DISTANCE(trans_1->position, pos_1);
		float speed = min(dist / elapsed, 50);

		pos_1 = pos_1 + normal_dir * speed * elapsed;

		//pos_1 = trans_1->position;
	}

	// Update the second pos
	if (transform_2.isValid()) {
		TCompTransform* trans_2 = transform_2;

		XMVECTOR normal_dir = trans_2->position - pos_2;
		normal_dir = XMVector3Normalize(normal_dir);
		float dist = V3DISTANCE(trans_2->position, pos_2);
		float speed = min(dist / elapsed, 50);

		pos_2 = pos_2 + normal_dir * speed * elapsed;

		//pos_2 = trans_2->position;
	}


	// Rope collisions
	CHandle e1 = CHandle();
	CHandle e2 = CHandle();
	CHandle e3 = CEntityManager::get().getByName("Player");

	if (transform_1.isValid()) {
		e1 = transform_1.getOwner();
	}
	if (transform_2.isValid()) {
		e2 = transform_2.getOwner();
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