#include "mcv_platform.h"
#include "entity_manager.h"
#include "callback_physics.h"
#include "physics_manager.h"
#include "components\all_components.h"
#include <PxSimulationEventCallback.h>

CEntityManager entity_manager = CEntityManager::get();

CCallbacks_physx::CCallbacks_physx() : forceLargeImpact(6000), forceMediumImpact(1000) {
}

void CCallbacks_physx::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	//dbg("onContact");

	for (PxU32 i = 0; i < nbPairs; i++){
		//Acceder a los actores pertenecientes al pair
		const PxContactPair& cp = pairs[i];

		if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND){

			PxActor* firstActor = pairHeader.actors[0];
			PxActor* otherActor = pairHeader.actors[1];

			const char* name1 = firstActor->getName();
			const char* name2 = otherActor->getName();

			CEntity* firstActorEntity = CEntityManager::get().getByName(name1);
			CEntity* secondActorEntity = CEntityManager::get().getByName(name2);

			//Colision entre actor y player
			if ((secondActorEntity->hasTag("actor")) && (firstActorEntity->hasTag("player"))){
				TCompRagdoll* p_ragdoll=firstActorEntity->get<TCompRagdoll>();
				TCompPlayerController* p_controller = firstActorEntity->get<TCompPlayerController>();
				if ((!(p_ragdoll->isRagdollActive()) || (p_controller->fsm_player_legs.getCurrentNode()=="fbp_WakeUp"))){
					TCompRigidBody* second_rigid = secondActorEntity->get<TCompRigidBody>();
					PxReal force_threshold = second_rigid->rigidBody->getContactReportThreshold();
					PxReal force = getForce(second_rigid->getMass(), pairs, i);
					float force_float = force;
					if (force_float > force_threshold)
						firstActorEntity->sendMsg(TActorHit(firstActorEntity, force_float));
				}
			} else if((secondActorEntity->hasTag("player")) && (firstActorEntity->hasTag("actor"))){
			//Colision entre player y actor
				TCompRagdoll* p_ragdoll = secondActorEntity->get<TCompRagdoll>();
				TCompPlayerController* p_controller = firstActorEntity->get<TCompPlayerController>();
				if ((!(p_ragdoll->isRagdollActive()) || (p_controller->fsm_player_legs.getCurrentNode() == "fbp_WakeUp"))){
					TCompRigidBody* first_rigid = firstActorEntity->get<TCompRigidBody>();
					PxReal force_threshold = first_rigid->rigidBody->getContactReportThreshold();
					PxReal force = getForce(first_rigid->getMass(), pairs, i);
					float force_float = force;
					if (force_float > force_threshold)
						secondActorEntity->sendMsg(TActorHit(secondActorEntity, force_float));
				}
			}else if ((secondActorEntity->hasTag("actor")) && (firstActorEntity->hasTag("enemy"))){
			//Colision entre actor y enemigo
				TCompRigidBody* second_rigid = secondActorEntity->get<TCompRigidBody>();
				PxReal force_threshold = second_rigid->rigidBody->getContactReportThreshold();
				PxReal force = getForce(second_rigid->getMass(), pairs, i);
				float force_float = force;
				if (force_float >force_threshold)
					firstActorEntity->sendMsg(TActorHit(firstActorEntity, force_float));
			}else if ((secondActorEntity->hasTag("enemy")) && (firstActorEntity->hasTag("actor"))){
			//Colision entre enemigo y actor
				TCompRigidBody* firstActorEntity = secondActorEntity->get<TCompRigidBody>();
				PxReal force_threshold = firstActorEntity->rigidBody->getContactReportThreshold();
				PxReal force = getForce(firstActorEntity->getMass(), pairs, i);
				float force_float = force;
				if (force_float >force_threshold)
					secondActorEntity->sendMsg(TActorHit(secondActorEntity, force_float));
			}else if ((firstActorEntity->hasTag("enemy")) && (secondActorEntity->hasTag("level"))){
			//Colision entre enemigo y escenario
				//TCompTransform* entity_transform = firstActorEntity->get<TCompTransform>();
				//CSoundManager::get().play3DFX("sonar", (TTransform*) entity_transform);
			}
			else if ((firstActorEntity->hasTag("actor")) && (secondActorEntity->hasTag("level"))){
				TCompRigidBody* first_rigid = firstActorEntity->get<TCompRigidBody>();
				PxReal force = getForce(first_rigid->getMass(), pairs, i);
				float force_float = force;
				CSoundManager::get().playImpactFX(force_float, firstActorEntity->get<TCompTransform>());
			}else if ((firstActorEntity->hasTag("level")) && (secondActorEntity->hasTag("actor"))){
				TCompRigidBody* rigid = secondActorEntity->get<TCompRigidBody>();
				PxReal force = getForce(rigid->getMass(), pairs, i);
					float force_float = force;
					CSoundManager::get().playImpactFX(force_float, secondActorEntity->get<TCompTransform>());
			}else if ((firstActorEntity->hasTag("actor")) && (secondActorEntity->hasTag("actor"))){
				TCompRigidBody* rigid = firstActorEntity->get<TCompRigidBody>();
				PxReal force = getForce(rigid->getMass(), pairs, i);
				float force_float = force;
				CSoundManager::get().playImpactFX(force_float, secondActorEntity->get<TCompTransform>());
			}else if ((firstActorEntity->hasTag("enemy")) && (secondActorEntity->hasTag("player"))){
				firstActorEntity->sendMsg(TPlayerTouch(firstActorEntity, true));
			}else if ((firstActorEntity->hasTag("player")) && (secondActorEntity->hasTag("enemy"))){
				secondActorEntity->sendMsg(TPlayerTouch(secondActorEntity, true));
			}
		}
	}
}

//Metodo para el calculo de fuerza media en colisiones
PxReal CCallbacks_physx::getForce(PxReal mass, const PxContactPair* pairs, PxU32 index){
	XMVECTOR force = { 0.f, 0.f, 0.f, 0.f };
	const PxU32 bufferSize = 64;
	PxContactPairPoint contacts[bufferSize];
	const PxContactPair& cp = pairs[index];
	PxU32 nbContacts = pairs[index].extractContacts(contacts, bufferSize);
	for (PxU32 j = 0; j < nbContacts; j++)
	{
		XMVECTOR impulse = Physics.PxVec3ToXMVECTOR(contacts[j].impulse);
		XMVECTOR normal = Physics.PxVec3ToXMVECTOR(contacts[j].normal);
		force = force + (XMVector3Dot(normal, impulse));
	}
	force = force / (float)nbContacts;
	PxVec3 forcePhysics = Physics.XMVECTORToPxVec3(force);
	PxReal forceMagnitude = forcePhysics.magnitude();
	return forceMagnitude;
}

PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	//Activación de flags cuando actores entran en contacto o el contactor persiste
	pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eDETECT_CCD_CONTACT;
	//| PxPairFlag::eNOTIFY_TOUCH_FOUND; --> cuando impacto
	//| PxPairFlag::eNOTIFY_TOUCH_PERSISTS --> cuando impacto persiste
	//| PxPairFlag::eNOTIFY_CONTACT_POINTS; --> necesaria para calculos de impacto por punto

	//Compruebo la lista de los involucrados en la colision de entities con las que no colisionan.
	//Si la colision se produce entre dos entities descartadas entre si, se anula dicha colision.
	//Si no es así, se hacen las comprobaciones pertinentes y se deja la colision

	if ((filterData1.word0 == FilterGroup::eNON_COLLISION)||(filterData0.word0 == FilterGroup::eNON_COLLISION))
		return PxFilterFlag::eSUPPRESS;
	else{

		if (filterData0.word0 != FilterGroup::ePARTICLES){
			if (filterData0.word1&filterData1.word0)
				return PxFilterFlag::eSUPPRESS;
			else if (filterData1.word0&filterData0.word1)
				return PxFilterFlag::eSUPPRESS;
			else{
				//Colisiones entre actores (cajas) y enemigos
				if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::eENEMY)){
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eENEMY) && (filterData1.word0 == FilterGroup::eACTOR)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::ePLAYER) && (filterData1.word0 == FilterGroup::eACTOR)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::ePLAYER)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::eLEVEL)){
					//Colisiones entre actores (cajas) y el escenario
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eLEVEL) && (filterData1.word0 == FilterGroup::eACTOR)){
					//Colisiones entre actores (cajas) y el escenario
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::ePLAYER)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::eACTOR)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::eENEMY) && (filterData1.word0 == FilterGroup::ePLAYER)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else if ((filterData0.word0 == FilterGroup::ePLAYER) && (filterData1.word0 == FilterGroup::eENEMY)){
					//Colisiones entre actores (cajas) y enemigos
					pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_CCD_CONTACT;
					return PxFilterFlag::eDEFAULT;
				}
				else{
					return PxFilterFlag::eDEFAULT;
				}
				return PxFilterFlag::eDEFAULT;
			}
			return PxFilterFlag::eDEFAULT;
		}
		return PxFilterFlag::eDEFAULT;
	}
	return PxFilterFlag::eDEFAULT;
}

CFilterCallback::CFilterCallback(){};

PxFilterFlags 	CFilterCallback::pairFound(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, const PxActor *a0, const PxShape *s0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, const PxActor *a1, const PxShape *s1, PxPairFlags &pairFlags){
	return PxFilterFlag::eSUPPRESS;
}