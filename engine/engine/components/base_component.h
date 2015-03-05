#ifndef INC_BASE_COMPONENT_H_
#define INC_BASE_COMPONENT_H_

struct TBaseComponent {
public:
	bool active;

	TBaseComponent() : active(true) {}

	template< class TObj >
	CHandle assertRequiredComponent(CHandle who_requires){		
		CEntity* e = who_requires.getOwner();
		CHandle handle = e->get<TObj>();
		assert(handle.isValid() || fatal("%s %s component requires a %s component\n", e->getName(), CHandleManager::the_register.getByType(who_requires.getType())->getObjTypeName(), getObjManager<TObj>()->getObjTypeName()));
		return handle;
	}

	template< class TObj >
	CHandle getSibling(CHandle who){
		CEntity* e = who.getOwner();
		return  e->get<TObj>();
	}
};

#endif