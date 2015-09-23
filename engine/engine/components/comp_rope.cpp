#include "mcv_platform.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "comp_transform.h"
#include "entity_manager.h"
#include "rope_manager.h"
#include "physics_manager.h"
#include "comp_needle.h"
#include "audio\sound_manager.h"
#include "ai\logic_manager.h"
#include "comp_particle_group.h"

TCompRope::~TCompRope() {
	CHandle valid_trans_1 = transform_1_aux.isValid() ? transform_1_aux : transform_1;
	CHandle valid_trans_2 = transform_2_aux.isValid() ? transform_2_aux : transform_2;

	if (valid_trans_1.isValid()) {

		TCompNeedle* needle = ((CEntity*)valid_trans_1.getOwner())->get<TCompNeedle>();
		if (needle && needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			if (attached_entity.isValid()) {
				TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

				if (attached_transform) {
					if (attached_transform->getType() == 95)
						attached_transform->setType(1);
					if (attached_transform->getType() == 90)
						attached_transform->setType(0.8f);
				}
			}
		}
	}

	// Update the second pos
	if (valid_trans_2.isValid()) {
		TCompNeedle* needle = ((CEntity*)valid_trans_2.getOwner())->get<TCompNeedle>();
		if (needle && needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			if (attached_entity.isValid()) {
				TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

				if (attached_transform) {
					if (attached_transform->getType() == 95)
						attached_transform->setType(1);
					if (attached_transform->getType() == 90)
						attached_transform->setType(0.8f);
				}
			}
		}
	}

	// Create particle system
	XMVECTOR position = XMVectorSet(0, 0, 0, 0);
	XMVECTOR rotation = XMQuaternionIdentity(); //XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), deg2rad(0));
	CHandle particle_entity = CLogicManager::get().instantiateParticleGroup("ps_rope_destroy2", position, rotation);
	
	std::vector<XMFLOAT3> positions;

	XMVECTOR init = pos_1;
	XMVECTOR end = pos_2;
	XMFLOAT3 value;
	float pos = 0;
	for (int i = 0; i < 50; i++) {
		if (i > 0) {
			pos = i / 50.0f;
		}		
		XMStoreFloat3(&value, XMVectorLerp(pos_1, pos_2, pos));
		positions.push_back(value);
	}


	// ************** HORRIBLE PRUEBA PARA QUE SIGA AL A CUERDA ********************
	float dist = XMVectorGetX(XMVector3Length(pos_2 - pos_1));

	const int epsilon = 50;
	const int sizes = 4;
	//float width = 0.1;
	XMFLOAT3 ropeReferences[epsilon];
	CVertexPosUVNormal ropeVertices[epsilon * sizes];
	CMesh::TIndex ropeIndices[(epsilon - 1) * sizes * 6];
	float y = 0;
	float pow_r = 4;	// Suavidad de la onda
	int velocity = 10;	// Velocidad de movimiento
	float wave_freq = 0.5f;	// frecuencia del ruido de la onda
	float amplitude = 0.0f;	// amplitud del ruido de la onda
	float elapsed = CApp::get().total_time;

	for (int i = 0; i < epsilon; i++)
	{
		XMVECTOR midPos = XMVectorLerp(pos_1, pos_2, (float)i / (epsilon - 1));

		if (i < epsilon / 2)
			y = (float)(pow(epsilon - i, pow_r) / pow(epsilon, pow_r));
		else
			y = (float)(pow(i, pow_r) / pow(epsilon, pow_r));

		float noise = (sin(elapsed*velocity + i*wave_freq) + 1) * amplitude;
		if (i != 0 && i != epsilon - 1)
			y += noise;

		y -= 1;

		PxRaycastBuffer hit;
		float y_offset = 1;
		Physics.raycast(midPos + XMVectorSet(0, y_offset, 0, 0), XMVectorSet(0, -1, 0, 0), 2, hit);
		CEntity* entity;

		if (hit.hasBlock) {
			PxRaycastHit blockHit = hit.block;
			entity = CHandle(hit.block.actor->userData);

			if ((!entity->hasTag("enemy")) && (!entity->hasTag("player"))) {
				float m_dist = blockHit.distance - y_offset;
				if (m_dist < abs(y))
					y = -m_dist;
			}

		}

		y *= tensed ? 0 : 1;
		// Guardar la referencia de posición de la cuerda
		ropeReferences[i] = XMFLOAT3(XMVectorGetX(midPos), XMVectorGetY(midPos) + y, XMVectorGetZ(midPos));
	}
	// *****************************************************************************

	if (particle_entity.isValid()) {
		TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
		pg->destroy_on_death = true;
		if (pg->particle_systems->size() > 0)
		{
			(*pg->particle_systems)[0].restart();
			(*pg->particle_systems)[0].setParticlePositions(ropeReferences, epsilon);
		}
		CLogicManager::get().p_group_counter++;
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

	TCompDistanceJoint* m_joint = joint;

	PxRigidActor* a1 = nullptr;
	PxRigidActor* a2 = nullptr;

	m_joint->joint->getActors(a1, a2);

	CEntity* e1 = CHandle(a1->userData);
	CEntity* e2 = CHandle(a2->userData);

	transform_1 = e1->get<TCompTransform>();
	transform_2 = e2->get<TCompTransform>();

	XMVECTOR initialPos = XMVectorZero();
	XMVECTOR finalPos = XMVectorZero();

	XMVECTOR rot1 = XMQuaternionIdentity();
	XMVECTOR rot2 = XMQuaternionIdentity();

	if (a1) {
		XMVECTOR offset_pos1 = Physics.PxVec3ToXMVECTOR(m_joint->joint->getLocalPose(PxJointActorIndex::eACTOR0).p);

		XMVECTOR pos1 = Physics.PxVec3ToXMVECTOR(a1->getGlobalPose().p);
		rot1 = Physics.PxQuatToXMVECTOR(a1->getGlobalPose().q);

		XMVECTOR offset_rotado_1 = XMVector3Rotate(offset_pos1, rot1);

		//   RECREATE ROPE
		// Obtener el punto en coordenadas de mundo = Offset * rotación + posición
		initialPos = pos1 + offset_rotado_1;
	}
	else {
		initialPos = Physics.PxVec3ToXMVECTOR(m_joint->joint->getLocalPose(PxJointActorIndex::eACTOR0).p);
	}
	if (a2) {
		XMVECTOR offset_pos2 = Physics.PxVec3ToXMVECTOR(m_joint->joint->getLocalPose(PxJointActorIndex::eACTOR1).p);

		XMVECTOR pos2 = Physics.PxVec3ToXMVECTOR(a2->getGlobalPose().p);
		rot2 = Physics.PxQuatToXMVECTOR(a2->getGlobalPose().q);

		XMVECTOR offset_rotado_2 = XMVector3Rotate(offset_pos2, rot2);

		//   RECREATE ROPE
		// Obtener el punto en coordenadas de mundo = Offset * rotación + posición
		finalPos = pos2 + offset_rotado_2;
	}
	else {
		finalPos = Physics.PxVec3ToXMVECTOR(m_joint->joint->getLocalPose(PxJointActorIndex::eACTOR1).p);
	}

	// TODO: Va al centro, no a donde debería ¿?
	pos_1 = initialPos;
	pos_2 = finalPos;

	width = atts.getFloat("width", 0.02f);
	max_distance = atts.getFloat("maxDistance", 20);
}

void TCompRope::fixedUpdate(float elapsed) {

	CHandle valid_trans_1 = transform_1_aux.isValid() ? transform_1_aux : transform_1;
	CHandle valid_trans_2 = transform_2_aux.isValid() ? transform_2_aux : transform_2;

	// Update the first pos
	if (valid_trans_1.isValid()) {
		TCompTransform* trans_1 = valid_trans_1;

		float dist = V3DISTANCE(trans_1->position, pos_1);

		if (dist > 0) {
			XMVECTOR normal_dir = trans_1->position - pos_1;
			normal_dir = XMVector3Normalize(normal_dir);
			float speed = min(dist / elapsed, 50);

			pos_1 = pos_1 + normal_dir * speed * elapsed;

			dist = V3DISTANCE(trans_1->position, pos_1);

			if (dist <= 0.f) {
				if (!sound_1_played){					

					CEntity* entity_1 = transform_1.getOwner();
					if (entity_1) {

						TCompNeedle* needle = entity_1->get<TCompNeedle>();
						if (needle) {

							CEntity* target_entity = CHandle(needle->getAttachedRigid()->userData);
							if (target_entity) {

								XMMATRIX view = XMMatrixLookAtRH(pos_1, pos_1 + normal_dir, XMVectorSet(0, 1, 0, 0));
								XMVECTOR rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));

								std::string name = target_entity->getName();
								XDEBUG("Needle shot to %s", name);
								std::string material = target_entity->material_tag;
								std::string particle_name = "ps_prota_jump_ring";

								if (material == "metal") {
									particle_name = "ps_metal_hit";
								}

								// Final test (underwater)
								if (XMVectorGetY(pos_1) < CApp::get().water_level) {
									particle_name = "ps_bubble_one_shot";
								}

								CHandle particle_entity = CLogicManager::get().instantiateParticleGroup(particle_name, pos_1 + normal_dir * 0.1f, trans_1->rotation);

								if (particle_entity.isValid()) {
									TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
									pg->kind = TCompParticleGroup::flag::IMPACT;
									pg->destroy_on_death = true;
									if (pg->particle_systems->size() > 0)
									{
										(*pg->particle_systems)[0].emitter_generation->inner_radius = 1.0f / 2.f;
										(*pg->particle_systems)[0].emitter_generation->radius = 1.0f;
									}
									CLogicManager::get().p_group_counter++;
								}
							}
						}
					}
					CSoundManager::get().playEvent("event:/Strings/needle_hit");
					sound_1_played=true;
				}
			}
		}
		

		TCompNeedle* needle = ((CEntity*)valid_trans_1.getOwner())->get<TCompNeedle>();
		if (needle && needle->getAttachedRigid() != nullptr) {
			CHandle attached_entity(needle->getAttachedRigid()->userData);
			if (attached_entity.isValid()) {
				TCompTransform* attached_transform = ((CEntity*)attached_entity)->get<TCompTransform>();

				if (attached_transform->getType() == 80)
					attached_transform->setType(0.9f);
				if (attached_transform->getType() == 100)
					attached_transform->setType(0.95f);
			}
		}

		//pos_1 = trans_1->position;
	}

	// Update the second pos
	if (valid_trans_2.isValid()) {
		TCompTransform* trans_2 = valid_trans_2;

		float dist = V3DISTANCE(trans_2->position, pos_2);

		if (dist > 0) {
			XMVECTOR normal_dir = trans_2->position - pos_2;
			normal_dir = XMVector3Normalize(normal_dir);
			float speed = min(dist / elapsed, 50);

			pos_2 = pos_2 + normal_dir * speed * elapsed;

			dist = V3DISTANCE(trans_2->position, pos_2);

			if (dist <= 0.f) {
				if (!sound_2_played){

					CEntity* entity_2 = transform_2.getOwner();
					if (entity_2) {

						TCompNeedle* needle = entity_2->get<TCompNeedle>();
						if (needle) {

							CEntity* target_entity = CHandle(needle->getAttachedRigid()->userData);
							if (target_entity) {

								XMMATRIX view = XMMatrixLookAtRH(pos_2, pos_2 + normal_dir, XMVectorSet(0, 1, 0, 0));
								XMVECTOR rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));

								std::string name = target_entity->getName();
								XDEBUG("Needle shot to %s", name.c_str());
								std::string material = target_entity->material_tag;
								std::string particle_name = "ps_prota_jump_ring";

								if (material == "metal") {
									particle_name = "ps_metal_hit";
								}

								// Final test (underwater)
								if (XMVectorGetY(pos_2) < CApp::get().water_level) {
									particle_name = "ps_bubble_one_shot";
								}

								CHandle particle_entity = CLogicManager::get().instantiateParticleGroup(particle_name, pos_2 + normal_dir * 0.1f, trans_2->rotation);

								if (particle_entity.isValid()) {
									TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
									pg->kind = TCompParticleGroup::flag::IMPACT;
									pg->destroy_on_death = true;
									if (pg->particle_systems->size() > 0)
									{
										(*pg->particle_systems)[0].emitter_generation->inner_radius = 1.0f / 2.f;
										(*pg->particle_systems)[0].emitter_generation->radius = 1.0f;
									}
									CLogicManager::get().p_group_counter++;
								}
							}
						}
					}

					CSoundManager::get().playEvent("event:/Strings/needle_hit");
					sound_2_played = true;
				}
			}
		}

		TCompNeedle* needle = ((CEntity*)valid_trans_2.getOwner())->get<TCompNeedle>();
		if (needle && needle->getAttachedRigid() != nullptr) {
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
	for (unsigned int i = 0; i < buf.nbTouches; i++)
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

void TCompRope::tenseRope() {

}