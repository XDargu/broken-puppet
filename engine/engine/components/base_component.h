#ifndef INC_BASE_COMPONENT_H_
#define INC_BASE_COMPONENT_H_

#include "handle\handle.h"

struct TBaseComponent {
public:
	bool active;

	TBaseComponent() : active(true) {}

	template< class TObj >
	CHandle assertRequiredComponent(CHandle who_requires){
		SET_ERROR_CONTEXT("Getting a required component", getObjManager<TObj>()->getObjTypeName());
		CEntity* e = who_requires.getOwner();
		CHandle handle = e->get<TObj>();
		XASSERT(handle.isValid(), "%s %s component requires a %s component", e->getName(), CHandleManager::the_register.getByType(who_requires.getType())->getObjTypeName(), getObjManager<TObj>()->getObjTypeName());
		return handle;
	}

	// Return component sibling
	template< class TObj >
	CHandle getSibling(CHandle who){
		CEntity* e = who.getOwner();
		return  e->get<TObj>();
	}
};

#endif