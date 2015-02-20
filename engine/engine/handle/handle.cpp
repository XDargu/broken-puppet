#include "mcv_platform.h"
#include "handle.h"
#include "XMLParser.h"

// -----------------------------------------
bool CHandle::destroy() {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->destroyObj(*this);
	return false;
}
// -----------------------------------------
CHandle CHandle::clone() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->clone(*this);
	return CHandle();
}
// -----------------------------------------
bool CHandle::isValid() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->isValid(*this);
	return false;
}
// -----------------------------------------
CHandle CHandle::getOwner() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->getOwner(*this);
	return CHandle();
}

bool CHandle::setOwner(CHandle new_owner) {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->setOwner(*this, new_owner);
	return false;
}


// -----------------------------------------
void CHandleManager::dumpInternals() const {
	dbg(" IDX       Int      Next      Age      External.  NextFree:%d LastFree:%d Used %d/%d\n"
		, next_free_handle_ext_index, last_free_handle_ext_index, size(), capacity());
	for (uint32_t i = 0; i < max_objects_in_use; ++i) {
		const TExternData* ed = external_to_internal + i;
		uint32_t internal_idx = internal_to_external[i];
		dbg("%4d  %08x  %08x %08x      %08x\n"
			, i
			, ed->internal_index
			, ed->next_external_index
			, ed->current_age
			, internal_idx);
	}
}





// ----------------------------------------
// The register of all handle managers
CHandleManager::CRegister CHandleManager::the_register;

// 
#define DECL_OBJ_MANAGER(TObj,obj_name) \
  CObjManager<TObj> om_##TObj( obj_name ); \
  template<> CObjManager<TObj>* getObjManager<TObj>() { return &om_##TObj; }

const char* CEntity::getName() const {
	const TCompName* cn = get<TCompName>();
	return cn ? cn->name : "<unnamed>";
}

CEntity::CEntity( const CEntity& e ) {

	// Clone each of the components
	for( int i=0; i<CHandle::max_types; ++i ) {
		if( e.components[i].isValid() )
			components[i] = e.components[i].clone();
	}
}



MMsgSubscriptions msg_subscriptions;

TMsgID generateUniqueMsgID() {
	static TMsgID global_id = 1;
	global_id++;
	return global_id;
}


DECL_OBJ_MANAGER(CEntity, "entity");
DECL_OBJ_MANAGER(TTransform, "transform");
DECL_OBJ_MANAGER(TController, "controller");
DECL_OBJ_MANAGER(TLife, "life");
DECL_OBJ_MANAGER(TCompName, "name");
DECL_OBJ_MANAGER(TMesh, "mesh");
DECL_OBJ_MANAGER(TCamera, "camera");
DECL_OBJ_MANAGER(TCollider, "collider");
DECL_OBJ_MANAGER(TRigidBody, "rigidbody");
DECL_OBJ_MANAGER(TStaticBody, "staticbody");
DECL_OBJ_MANAGER(TPlayerController, "playerController");
DECL_OBJ_MANAGER(TCameraPivotController, "cameraPivotController");
DECL_OBJ_MANAGER(TPlayerPivotController, "playerPivotController");
DECL_OBJ_MANAGER(TThirdPersonCameraController, "thirdPersonCameraController");
DECL_OBJ_MANAGER(TEnemyWithPhysics, "enemyWithPhysics");
DECL_OBJ_MANAGER(TAABB, "aabb");
DECL_OBJ_MANAGER(TDistanceJoint, "distanceJoint");


/*
void createManagers() {
	getObjManager<CEntity>()->init(1024);
	getObjManager<TTransform>()->init(32);
	getObjManager<TController>()->init(32);
	getObjManager<TLife>()->init(4);
	getObjManager<TCompName>()->init(1024);
}

CHandle last_entity;

class CMyParser : public CXMLParser {
	CHandle current_entity;
public:
	void onStartElement(const std::string &elem, MKeyValue &atts) {
		dbg("onStart %s\n", elem.c_str());

		// Check if there is a handle manager with that name
		CHandleManager* hm = CHandleManager::the_register.getByName(elem.c_str());
		if (!hm) {
			dbg("unknown tag %s\n", elem.c_str());
			return;
		}

		CHandle h = hm->createEmptyObj(atts);

		if (elem != "entity") {
			assert(current_entity.isValid());
			CEntity* e = current_entity;
			e->add(h);
		}
		else {
			current_entity = h;
		}

	}

	// Cuando se acaba el tag de xml
	void onEndElement(const std::string &elem) {
		dbg("onEnd %s\n", elem.c_str());
		if (elem == "entity") {
			last_entity = current_entity;
			current_entity = CHandle();
		}
	}
};


void testComponents() {

	createManagers();
	CMyParser p;
	p.xmlParseFile("my_file.xml");

	CEntity* e = last_entity;
	if (e) {
		TLife *life = e->get<TLife>();
		dbg("last is %s nad has life %f\n", e->getName(), life->life);
	}

}
*/