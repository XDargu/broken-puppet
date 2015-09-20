#include "mcv_platform.h"
#include "handle.h"
#include "components/comp_name.h"

MMsgSubscriptions msg_subscriptions;

const char* CEntity::getName() const {
	const TCompName* cn = get<TCompName>();
	return cn ? cn->name : "<unnamed>";
}

// When cloning an entity
CEntity::CEntity( const CEntity& e ) {

	// Clone each of the components
	for( int i=0; i<CHandle::max_types; ++i ) {
		if( e.components[i].isValid() )
			components[i] = e.components[i].clone();
	}
}

// ----------------------------------------
// The register of all handle managers
CHandleManager::CRegister CHandleManager::the_register;

TMsgID generateUniqueMsgID() {
	static TMsgID global_id = 1;
	global_id++;
	return global_id;
}

