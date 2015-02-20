#ifndef INC_OBJS_MANAGER_H_
#define INC_OBJS_MANAGER_H_

// -----------------------------------------
// Handle manager storing objects of type TObj
template< class TObj >
class CObjManager : public CHandleManager {
	// The real objects...
	TObj* objs;
	bool  updating_objs;
public:

	CObjManager(const char* the_obj_name) : objs(nullptr), updating_objs( false ) {
		obj_type_name = the_obj_name;
	}

	void init(uint32_t max_objects_allowed) {
		CHandleManager::init(max_objects_allowed);
		objs = new TObj[max_objects_allowed];
	}

	// Given an object, return the Handle that represents it
	CHandle getHandleFromObjAddr(TObj* obj) {
		// As we have the objects in linear array, subtracting pointers
		// gives the index in the array
		uintptr_t internal_idx = obj - objs;

		// If the array is invalid, return an invalid handle
		if (internal_idx >= max_objects_in_use)
			return CHandle();

		// Use the int2ext table to get the external index
		uint32_t external_idx = internal_to_external[internal_idx];

		// Get the table to read the current age of that handle
		TExternData* ed = external_to_internal + external_idx;

		// Build the handle
		return CHandle(getType(), external_idx, ed->current_age);
	}


	TObj* getObjByHandle(CHandle h) const {
		assert(h.getType() == getType());
		uint32_t external_idx = h.getExternalIndex();

		// Comprobar que el external_id is potencialmente valido
		if (external_idx >= max_objects_in_use)
			return nullptr;

		// Acceder a los datos que yo tengo para ese handle
		const TExternData* ed = external_to_internal + external_idx;

		// Es un handle antiguo?
		if (ed->current_age != h.getAge())
			return nullptr;

		assert(ed->internal_index != invalid_index);
		return objs + ed->internal_index;
	}

	CHandle createEmptyObj(MKeyValue& atts) {
		CHandle h = createObj();
		TObj* obj = h;
		obj->loadFromAtts(atts);
		return h;
	}

	// ------------------
	// Contruct a new object, using the given args
	CHandle createObj() {

		// Do we have space?
		assert(num_objects_in_use < max_objects_in_use || fatal("No more space for components of type %s\n", getObjTypeName()));

		// The real object goes at the end of the array of real objects
		uint32_t internal_idx = num_objects_in_use;
		++num_objects_in_use;

		// The final object address
		TObj* obj = objs + internal_idx;

		// Call the original ctor using the pointer given by use (obj)
		// and using the given args
		obj = ::new(obj)TObj;

		// Find a free handle to use
		uint32_t external_idx = next_free_handle_ext_index;

		// Update the next_free_handle_ext_index for the next createObj call
		next_free_handle_ext_index = external_to_internal[external_idx].next_external_index;
		if (next_free_handle_ext_index == invalid_index) {
			last_free_handle_ext_index = invalid_index;
			assert(size() == capacity());
		}

		// Update the external data to link to the created object
		TExternData* ed = external_to_internal + external_idx;
		ed->next_external_index = invalid_index;    // Not linked to next anymore
		ed->internal_index = internal_idx;
		ed->current_owner = CHandle();

		// Update the internal to external table
		internal_to_external[internal_idx] = external_idx;

		return CHandle(getType(), external_idx, ed->current_age);
	}

	// --------------------------------------------
	bool destroyObj(CHandle h) {
		assert( !updating_objs || fatal( "No puedes borrar componentes de este tipo mientras estas actualizandolos"));
		// Confirm the handle is mine
		assert(h.getType() == getType());

		// The index is garbage
		if (h.getExternalIndex() >= max_objects_in_use)
			return false;

		uint32_t external_index_removed = h.getExternalIndex();

		TExternData* ed = external_to_internal + external_index_removed;

		// The handle is too old
		if (h.getAge() != ed->current_age)
			return false;

		// Access to the real object to be deleted
		TObj *obj = objs + ed->internal_index;

		// Call the dtor directly
		obj->~TObj();

		// If it's not the last object in the linear array of objects...
		if (ed->internal_index != size() - 1) {

			// Move the last object in the array to replace the deleted object
			// So all objects are still packed
			TObj* last_obj = objs + size() - 1;

			// Do the real move semantics
			*obj = std::move(*last_obj);

			uint32_t external_idx_of_last_obj = internal_to_external[size() - 1];

			// Update the external table, so we updated the new location of the last object
			external_to_internal[external_idx_of_last_obj].internal_index = ed->internal_index;

			// Update the internal table of the last object, because now it's on the deleted
			// position
			internal_to_external[ed->internal_index] = external_idx_of_last_obj;
		}

		// Delete the internal_to_external of the last valid object, it has been moved
		internal_to_external[size() - 1] = invalid_index;

		// We now have one less object
		assert(num_objects_in_use > 0);
		num_objects_in_use--;

		// Update the linked list of free handles
		if (next_free_handle_ext_index != invalid_index) {
			TExternData* ed = external_to_internal + last_free_handle_ext_index;
			ed->next_external_index = external_index_removed;
			// This is the new tail 
			last_free_handle_ext_index = external_index_removed;
		}
		else {
			// Only when all objects have been used, and we free one object
			next_free_handle_ext_index = last_free_handle_ext_index = external_index_removed;
		}

		// Update the external table, because there is no real object associated at this index
		// anymore
		ed->internal_index = invalid_index;

		// This invalidates all existing handles at that index
		ed->current_age++;

		return true;
	}

	// -----------------------------------------
	CHandle clone(CHandle h) {
		// Get access to the real object given the handle
		TObj* orig_obj = getObjByHandle(h);
		// Confirm the handle is valid
		if (orig_obj == nullptr)
			return CHandle();

		// Create a new empty object
		CHandle new_h = createObj();

		// Get the real object
		TObj* new_obj = getObjByHandle(new_h);

		// Call again the ctor of the obj, passing as argument
		// the original object, but the memory to call the ctor
		// is in the new object
		// Warning!!, we are calling twice to the ctor!!!
		::new(new_obj)TObj(*orig_obj) ;

		return new_h;
	}

	// Handler initialization
	void initHandlers() {
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++)
			obj->init();
	}

	// Handler update
	void update(float elapsed) {
		updating_objs = true;
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++) {
			if (obj->active)
				obj->update(elapsed);
		}
		updating_objs = false;
	}

	// Fixed time update, used for physx
	void fixedUpdate(float elapsed) {
		updating_objs = true;
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++) {
			if (obj->active)
				obj->fixedUpdate(elapsed);
		}
		updating_objs = false;
	}

	// Set activation
	void setActiveComponents(bool active) {
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++) {
			obj->active = active;
		}
	}
};


// ----------------------------------------
// Declaracion que dado un tipo de c++
// hay una funcion que devuelve un manager de ese tipo
template< class TObj >
CObjManager<TObj>* getObjManager();


#endif
