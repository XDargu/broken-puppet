#include "mcv_platform.h"
#include "entity_manager.h"
#include "callback_physics.h"
#include "physics_manager.h"
#include "components\all_components.h"
#include <PxSimulationEventCallback.h>

CCallbacks_physx::CCallbacks_physx() : forceLargeImpact (1000) {
}

void CCallbacks_physx::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	dbg("onContact");

	for (PxU32 i = 0; i < nbPairs; i++){
		//Acceder a los actores pertenecientes al pair
		PxActor* firstActor = pairHeader.actors[0];
		PxActor* otherActor = pairHeader.actors[1];

		const char* name1 = firstActor->getName();
		const char* name2 = otherActor->getName();

		CEntity* firstActorEntity=CEntityManager::get().getByName(name1);
		CEntity* secondActorEntity=CEntityManager::get().getByName(name2);

		TCompRigidBody* first_rigid = firstActorEntity->get<TCompRigidBody>();
		TCompUnityCharacterController* first_controller = firstActorEntity->get<TCompUnityCharacterController>();
		TCompRigidBody* second_rigid = secondActorEntity->get<TCompRigidBody>();
		TCompUnityCharacterController* second_controller = secondActorEntity->get<TCompUnityCharacterController>();

		//Colision entre actor y enemigo
		if ((second_rigid) && ((first_controller))){
			PxReal force = getForce(second_rigid->getMass(), pairs, i);
			if (force > forceLargeImpact){
				firstActorEntity->destroyComponents();
			}
		}

		//Colision entre enemigo y actor
		if ((first_rigid) && ((second_controller))){
			PxReal force = getForce(second_controller->getMass(), pairs, i);
			if (force > forceLargeImpact){
				//TO DO: Cambiar por el metodo de destruccion de enemigos pertinente
				//secondActorEntity->~CEntity();
				//secondActorEntity->destroyComponents();
				CEntityManager::get().remove(secondActorEntity);
			}
		}

		//Colision entre enemigo y escenario
		if ((first_controller) && ((!second_rigid))){
			PxReal force = getForce(first_controller->getMass(), pairs, i);
			if (force > forceLargeImpact){
				bool prueba = true; 
				//firstActorEntity->~CEntity();
				CEntityManager::get().remove(firstActorEntity);
			}
		}

	}
}

//Metodo para el calculo de fuerza media en colisiones
PxReal CCallbacks_physx::getForce(PxReal mass, const PxContactPair* pairs, PxU32 index){
	XMVECTOR force = {0.f,0.f,0.f,0.f};
	const PxU32 bufferSize = 64;
	PxContactPairPoint contacts[bufferSize];
	const PxContactPair& cp = pairs[index];
	PxU32 nbContacts = pairs[index].extractContacts(contacts, bufferSize);
	for (PxU32 j = 0; j < nbContacts; j++)
	{
		XMVECTOR impulse = Physics.PxVec3ToXMVECTOR(contacts[j].impulse);
		XMVECTOR normal = Physics.PxVec3ToXMVECTOR(contacts[j].normal);
		force = force+(XMVector3Dot(normal, impulse)*mass);
	}
	force = force / nbContacts;
	PxVec3 forcePhysics = Physics.XMVECTORToPxVec3(force);
	PxReal forceMagnitude = forcePhysics.magnitude();
	return forceMagnitude;
}

void CCallbacks_physx::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	dbg("onTrigger");
}

PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{

	CCallbacks_physx prueba1;

	//Activación de flags cuando actores entran en contacto o el contactor persiste
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	//| PxPairFlag::eNOTIFY_TOUCH_FOUND; --> cuando impacto
	//| PxPairFlag::eNOTIFY_TOUCH_PERSISTS --> cuando impacto persiste
	//| PxPairFlag::eNOTIFY_CONTACT_POINTS; --> necesaria para calculos de impacto por punto

	//Colisiones entre actores (cajas) y enemigos
	if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::eENEMY))
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS;

	//Colisiones entre actores (cajas) y enemigos
	if ((filterData0.word0 == FilterGroup::eENEMY) && (filterData1.word0 == FilterGroup::eACTOR)){
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

	//Colisiones entre actores y el nivel
	if ((filterData0.word0 == FilterGroup::eLEVEL) && (filterData1.word0 == FilterGroup::eENEMY)){
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

	return PxFilterFlag::eDEFAULT;

}
