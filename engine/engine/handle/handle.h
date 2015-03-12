#ifndef INC_HANDLE_H_
#define INC_HANDLE_H_

#include "mcv_platform.h"
#include "XMLParser.h"
#include "render/render_utils.h"
#include "physics_manager.h"

using namespace DirectX;

class MKeyValue;
struct TMsg;

#define Physics CPhysicsManager::get()

class CHandle {
public:
	static const uint32_t num_bits_type = 7;
	static const uint32_t num_bits_index = 12;
	static const uint32_t num_bits_age = 32 - num_bits_type - num_bits_index;
	static const uint32_t max_types = 1 << num_bits_type;

	// Contructors
	CHandle() : external_index(0), type(0), age(0) { }
	CHandle(uint32_t atype, uint32_t aidx, uint32_t aage)
		: type(atype)
		, external_index(aidx)
		, age(aage)
	{ }

	template< class TObj >
	CHandle(TObj *obj) {
		CObjManager<TObj>* om = getObjManager<TObj>();
		*this = om->getHandleFromObjAddr(obj);
	}

	// Create a object of type TObj, and perfectly forward
	// the given N args (even empty) to the createObj method
	template< class TObj >
	static CHandle create() {
		CObjManager<TObj>* om = getObjManager<TObj>();
		return om->createObj();
	}

	// 
	bool destroy();
	CHandle clone() const;

	// Automatic conversion from Handle to the Type
	template<class TObj>
	operator TObj*() const {
		// std::remove_const<T>::type returns the TObj without const
		// Used when TObj is const*. We want the manager of <TLife> objs
		// not the manager of <const TLife>, so we use the remove_const
		auto om = getObjManager<std::remove_const<TObj>::type>();
		assert(om);
		if (type == 0)
			return nullptr;
		assert(this->type == om->getType()
			|| fatal("Can't convert handle of type %s to %s\n"
			, CHandleManager::the_register.getByType(getType())->getObjTypeName()
			, om->getObjTypeName()));
		return om->getObjByHandle(*this);
	}

	bool operator==(const CHandle handle) const {
		return external_index == handle.external_index
			&& type == handle.type
			&& age == handle.age;
	}

	bool isValid() const;
	uint32_t asUnsigned() const { return *(uint32_t*)this; }

	// Owner
	CHandle getOwner() const;
	bool    setOwner(CHandle new_owner);

	void    loadFromAtts(const std::string& elem, MKeyValue &atts);
	const char* getTypeName() const;

	// Read only 
	uint32_t getType() const { return type; }
	uint32_t getAge() const { return age; }
	uint32_t getExternalIndex() const { return external_index; }

private:
	uint32_t external_index : num_bits_index;
	uint32_t type : num_bits_type;
	uint32_t age : num_bits_age;
};

// -----------------------------------------
#include "handle/handle_manager.h"
#include "handle/objs_manager.h"
#include "handle/msgs.h"
#include "handle/entity.h"





/*
struct TController {    // 2 ...
float velocity, acc;
void update(float elapsed) {
CEntity* e = CHandle(this).getOwner();
dbg("Updating controller of %s\n", e->getName());
}
void loadFromAtts(MKeyValue& atts) {}

std::string toString() {
return "Velocity: " + std::to_string(velocity) + "Acceleration: " + std::to_string(acc);
}
};


struct TThirdPersonCameraController {
private:
TTransform* transform;
public:
TTransform*	player;

TThirdPersonCameraController() {}

void loadFromAtts(MKeyValue &atts) {
}

void init() {
CEntity* e_player = CEntityManager::get().getByName("Player");

assert(e_player || fatal("TFirstPersonCameraController requieres a player entity"));

player = e_player->get<TTransform>();
assert(player || fatal("TFirstPersonCameraController requieres a player entity with a TTransform component"));

CHandleManager* hm = CHandleManager::the_register.getByName("thirdPersonCameraController");
CEntity* e = hm->getOwner(this);
transform = e->get<TTransform>();

assert(transform || fatal("TFirstPersonCameraController requieres a TTransform component"));
}

void update(float elapsed) {
transform->position = player->position - player->getFront() * 3 + player->getUp() * 3;
transform->lookAt(player->position + player->getUp() * 2, player->getUp());
}

std::string toString() {
return "Camera controller";
}
};
*/

#endif


