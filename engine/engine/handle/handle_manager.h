#ifndef INC_HANDLE_MANAGER_H_
#define INC_HANDLE_MANAGER_H_

// -------------------------------------
class CHandleManager {

protected:
	static const uint32_t max_instances = 1 << CHandle::num_bits_index;
	static const uint32_t max_types = CHandle::max_types;
	static const uint32_t invalid_index = ~0;

	// This table saves is accessed using an external id
	struct TExternData {
		uint32_t internal_index;        // The index in the internal array of real objects
		uint32_t current_age;           // To check if a handle is the current valid handle with that index
		uint32_t next_external_index;   // To link external_data in this array
		CHandle  current_owner;

		TExternData()
			: internal_index(invalid_index)
			, current_age(0)
			, next_external_index(invalid_index)
		{}
	};

	TExternData* external_to_internal;

	// Given an internal index, sometimes we need to get the external_id
	uint32_t*    internal_to_external;

	// Keep track of the number of valid objects by the manager, and 
	// the maximum number of objects allowed. Maybe we don't want to 
	// allocate the full 1<<max_instances objects
	uint32_t     num_objects_in_use;
	uint32_t     max_objects_in_use;

	// Id and name associated to this handle manager
	uint32_t     type;
	const char*  obj_type_name;

	// To quickly find the next free handle when creating objects
	uint32_t     next_free_handle_ext_index;
	uint32_t     last_free_handle_ext_index;

	// Allocate the internal structures
	void init(uint32_t max_objects_allowed) {

		assert(max_objects_allowed > 0);            // Given number should make sense
		assert(external_to_internal == nullptr);    // Do not enter twice here

		// At this point we should have a name
		assert(obj_type_name);
		max_objects_in_use = max_objects_allowed;
		the_register.registerManager(this);

		// Alloc memory for lookup tables
		external_to_internal = new TExternData[max_objects_allowed];
		internal_to_external = new uint32_t[max_objects_allowed];

		// init table contents
		for (uint32_t i = 0; i < max_objects_allowed; ++i) {
			TExternData* ed = external_to_internal + i;
			ed->current_age = 1;
			ed->internal_index = invalid_index;
			// Each handle is linked to the next, except the last one
			if (i != max_objects_allowed - 1)
				ed->next_external_index = i + 1;
			else
				ed->next_external_index = invalid_index;

			internal_to_external[i] = invalid_index;
		}

		next_free_handle_ext_index = 0;
		last_free_handle_ext_index = max_objects_allowed - 1;
	}

public:

	// Everything is invalid at the ctor
	CHandleManager()
		: external_to_internal(nullptr)
		, internal_to_external(nullptr)
		, num_objects_in_use(0)
		, max_objects_in_use(0)
		, type(0)
		, obj_type_name(nullptr)
	{}

	const char* getObjTypeName() const { return obj_type_name; }
	uint32_t    getType() const { return type; }
	void        dumpInternals() const;
	uint32_t    size() const { return num_objects_in_use; }
	uint32_t    capacity() const { return max_objects_in_use; }

	// 
	bool        isValid(CHandle h) const {
		assert(h.getType() == getType());
		if (h.getExternalIndex() >= max_objects_in_use)
			return false;
		TExternData* ed = external_to_internal + h.getExternalIndex();
		return ed->current_age == h.getAge();
	}

	// Owners
	CHandle getOwner(CHandle h) {
		assert(h.getType() == getType());
		uint32_t external_idx = h.getExternalIndex();

		// Comprobar que el external_id is potencialmente valido
		if (external_idx >= max_objects_in_use)
			return CHandle();

		// Acceder a los datos que yo tengo para ese handle
		const TExternData* ed = external_to_internal + external_idx;
		return ed->current_owner;
	}

	bool setOwner(CHandle h, CHandle new_owner) {
		assert(h.getType() == getType());
		uint32_t external_idx = h.getExternalIndex();

		// Comprobar que el external_id is potencialmente valido
		if (external_idx >= max_objects_in_use)
			return false;

		// Acceder a los datos que yo tengo para ese handle
		// Y guardar el nuevo dato que me piden
		TExternData* ed = external_to_internal + external_idx;
		ed->current_owner = new_owner;
		return true;
	}

	// ---------------------------
	// Virtual methods. Can't implement these methods here
	// because I don't know the exact type of objects we are dealing with
	virtual ~CHandleManager() { }
	virtual bool destroyObj(CHandle h) = 0;
	virtual CHandle createEmptyObj() = 0;
	virtual CHandle clone(CHandle h) = 0;
	virtual void loadFromAtts(CHandle who, MKeyValue &atts) = 0;

	// ---------------------------
	class CRegister {

		struct TNameComparator {
			bool operator() (const char* name1, const char * name2) const {
				return strcmp(name1, name2) < 0;
			}
		};
		typedef std::map< const char*, CHandleManager*, TNameComparator > MHandlesByName;

		CHandleManager* all_managers[max_types];
		MHandlesByName  all_managers_by_name;
		// The type 0 is invalid, that's why we start in the 1
		uint32_t        next_type_id;

	public:

		CRegister() : next_type_id(1) { }

		CHandleManager* getByName(const char* name) {
			auto it = all_managers_by_name.find(name);
			if (it != all_managers_by_name.end())
				return it->second;
			//fatal("No manager of objs of type %s is registered\n", name);
			return nullptr;
		}

		CHandleManager* getByType(uint32_t type) {
			assert(type < max_types);
			return all_managers[type];
		}

		void registerManager(CHandleManager* manager) {
			assert(manager->getType() == 0);      // Should not be already registered
			assert(next_type_id < max_types);
			manager->type = next_type_id;

			// Manager should have a name at this point
			const char* name = manager->getObjTypeName();
			assert(name != nullptr);

			// Check if anoter manager already exists with the same name
			assert(all_managers_by_name.find(name) == all_managers_by_name.end());
			all_managers[next_type_id] = manager;   // Register by id
			all_managers_by_name[name] = manager;   // Register by name
			++next_type_id;
		}

	};

	static CRegister the_register;
};

#endif

