#include "mcv_platform.h"
#include "callback_physics.h"
#include "physics_manager.h"
#include <PxSimulationEventCallback.h>

CCallbacks_physx::CCallbacks_physx(){
}

void CCallbacks_physx::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	dbg("onContact");
	const PxU32 bufferSize = 64;
	PxContactPairPoint contacts[bufferSize];
	//pairHeader.actors[0] --> saca actor 1, con [1] sacamos el dos
	for (PxU32 i = 0; i < nbPairs; i++){
		const PxContactPair& cp = pairs[i];
		PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);

		//Acceder a los actores pertenecientes al pair
		PxActor* firstActor = pairHeader.actors[0];
		PxActor* otherActor = pairHeader.actors[1];
		std::string name1 = firstActor->getName();
		std::string name2 = otherActor->getName();

		//NOTA: con su nombre podriamos acceder a ellos para cambiarles el estado?


		for (PxU32 j = 0; j < nbContacts; j++)
		{
			PxVec3 point = contacts[j].position;
			PxVec3 impulse = contacts[j].impulse;
		}
	}
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

	//Activación de flags cuando actores entran en contacto o el contactor persiste
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	//| PxPairFlag::eNOTIFY_TOUCH_FOUND;
	//| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
	//| PxPairFlag::eNOTIFY_CONTACT_POINTS;

	//Colisiones entre actores (cajas)
	if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::eACTOR))
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND;

	//Colisiones entre actores y el nivel
	//if ((filterData0.word0 == FilterGroup::eACTOR) && (filterData1.word0 == FilterGroup::eLEVEL))
		//pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return PxFilterFlag::eDEFAULT;

}
