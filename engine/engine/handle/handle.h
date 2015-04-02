#ifndef INC_HANDLE_H_
#define INC_HANDLE_H_

#include "mcv_platform.h"
#include "XMLParser.h"

class MKeyValue;
struct TMsg;

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
		XASSERT(om, "Invalid object manager");
		if (type == 0)
			return nullptr;
		XASSERT(this->type == om->getType()
			, "Can't convert handle of type %s to %s"
			, CHandleManager::the_register.getByType(getType())->getObjTypeName()
			, om->getObjTypeName()
			);
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

	template <class TObj>
	bool isTypeOf() {
		auto om = getObjManager<std::remove_const<TObj>::type>();
		XASSERT(om, "Invalid object manager");
		return om->getType() == this->type;
	}

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

#endif


