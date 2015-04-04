#ifndef INC_COMP_ROPE_H
#define INC_COMP_ROPE_H

#include "base_component.h"

struct TCompRope : TBaseComponent {
private:
	CHandle transform_1;
	CHandle transform_2;

	CHandle joint;
public:

	XMVECTOR pos_1;
	XMVECTOR pos_2;

	float max_distance;
	float width;

	TCompRope() : max_distance( 20 ) , width ( 0.02f ) {}

	void setPositions(CHandle the_transform_1, XMVECTOR the_pos_2) {

		transform_1 = the_transform_1;
		pos_2 = the_pos_2;
	}

	void setPositions(XMVECTOR the_pos_1, CHandle the_transform_2) {

		pos_1 = the_pos_1;
		transform_2 = the_transform_2;
	}

	// With two transforms, a distance joiny is required
	void setPositions(CHandle the_transform_1, CHandle the_transform_2) {
		joint = assertRequiredComponent<TCompDistanceJoint>(this);

		transform_1 = the_transform_1;
		transform_2 = the_transform_2;
	}

	void loadFromAtts(const std::string& elem, MKeyValue& atts) {
		joint = assertRequiredComponent<TCompDistanceJoint>(this);

		width = atts.getFloat("width", 0.02f);
		max_distance = atts.getFloat("maxDistance", 20);
	}

	void fixedUpdate(float elapsed) {

		// Update the first pos
		if (transform_1.isValid()) {
			TCompTransform* trans_1 = transform_1;
			pos_1 = trans_1->position;
		}

		// Update the second pos
		if (transform_2.isValid()) {
			TCompTransform* trans_2 = transform_2;
			pos_2 = trans_2->position;
		}
		// If the joint distance exceeds the max distance, the destroy the entity
		/*float dist = ((TCompDistanceJoint*)joint)->joint->getDistance();
		if (dist > max_distance * max_distance) {
			CEntityManager::get().remove(CHandle(this).getOwner());
		}

		// Raycast
		TCompDistanceJoint* m_joint = (TCompDistanceJoint*)joint;
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;

		m_joint->joint->getActors(a1, a2);

		XMVECTOR offset_pos1 = Physics.PxVec3ToXMVECTOR(m_joint->joint->getLocalPose(physx::PxJointActorIndex::eACTOR0).p);
		XMVECTOR offset_pos2 = Physics.PxVec3ToXMVECTOR(m_joint->joint->getLocalPose(physx::PxJointActorIndex::eACTOR1).p);

		XMVECTOR pos1 = Physics.PxVec3ToXMVECTOR(a1->getGlobalPose().p);
		XMVECTOR pos2 = Physics.PxVec3ToXMVECTOR(a2->getGlobalPose().p);

		XMVECTOR rot1 = Physics.PxQuatToXMVECTOR(a1->getGlobalPose().q);
		XMVECTOR rot2 = Physics.PxQuatToXMVECTOR(a2->getGlobalPose().q);

		XMVECTOR offset_rotado_1 = XMVector3Rotate(offset_pos1, rot1);
		XMVECTOR offset_rotado_2 = XMVector3Rotate(offset_pos2, rot2);

		//   RECREATE ROPE   
		// Obtener el punto en coordenadas de mundo = Offset * rotaci�n + posici�n
		XMVECTOR initialPos = pos1 + offset_rotado_1;
		XMVECTOR finalPos = pos2 + offset_rotado_2;
		
		physx::PxVec3 dir = Physics.XMVECTORToPxVec3(finalPos - initialPos).getNormalized();

		physx::PxRaycastBuffer buf;

		Physics.raycastAll(Physics.XMVECTORToPxVec3(initialPos), dir, sqrt(dist), buf);
		float force_s = 100;

		for (int i = 0; i < buf.nbTouches; i++)
		{
			if (buf.touches[i].actor != a1 && buf.touches[i].actor != a2) {
				if (buf.touches[i].actor->isRigidBody()) {
					if (((physx::PxRigidBody*)buf.touches[i].actor)->getMass() < 100) {

						physx::PxRigidBody* body = ((physx::PxRigidBody*)buf.touches[i].actor);

						physx::PxVec3 force = (body->getGlobalPose().p - buf.touches[i].position).getNormalized();

						physx::PxVec3 velocity = physx::PxRigidBodyExt::getVelocityAtPos(*body, buf.touches[i].position);
						
						physx::PxRigidBodyExt::addForceAtPos(*body, -velocity, buf.touches[i].position, physx::PxForceMode::eVELOCITY_CHANGE);
						//((physx::PxRigidBody*)buf.touches[i].actor)->addForce(force * force_s, physx::PxForceMode::eIMPULSE);
					}
					else
					{
						CEntityManager::get().remove(CHandle(this).getOwner());
					}
				}
			}
		}*/

	}
};

#endif