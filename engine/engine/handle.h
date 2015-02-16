#ifndef INC_HANDLE_H_
#define INC_HANDLE_H_

#include "mcv_platform.h"
#include "XMLParser.h"
#include "render\render_utils.h"
#include "entity.h"
#include "physics_manager.h"

using namespace DirectX;

class MKeyValue;

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

	// Read only 
	uint32_t getType() const { return type; }
	uint32_t getAge() const { return age; }
	uint32_t getExternalIndex() const { return external_index; }

private:
	uint32_t external_index : num_bits_index;
	uint32_t type : num_bits_type;
	uint32_t age : num_bits_age;

};



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
	virtual CHandle createEmptyObj(MKeyValue &atts) = 0;
	virtual CHandle clone(CHandle h) = 0;

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

// ----------------------------------------
class CEntity {
	CHandle components[CHandle::max_types];
public:

	~CEntity() {
		for (uint32_t i = 0; i < CHandle::max_types; i++)
			components[i].destroy();
	}

	// Add a handle to this entity
	CHandle add(CHandle h) {
		assert(h.isValid() || fatal("Handle must be valid\n"));
		uint32_t h_type = h.getType();
		assert(!components[h_type].isValid() || fatal("Entity has already a handle of type %d\n", h.getType()));
		assert(!h.getOwner().isValid() || fatal("Handle %08x has already an owner: %08x\n", h.asUnsigned(), h.getOwner().asUnsigned()));
		components[h_type] = h;
		h.setOwner(CHandle(this));
		assert(h.getOwner() == CHandle(this));
		return h;
	}

	// Return true if the entity has a valid component of that type
	template< class TObj>
	bool has() const {
		return get<TObj>().isValid();
	}

	// Get the index given the obj type, and return whatever handle we
	// are storing now
	template< class TObj >
	CHandle get() const {
		uint32_t h_type = getObjManager<TObj>()->getType();
		return components[h_type];
	}

	// Destroy the handle of that type
	template< class TObj >
	bool del() {
		uint32_t h_type = getObjManager<TObj>()->getType();
		return components[h_type].destroy();
	}

	// Helpers
	const char* getName() const;
	void loadFromAtts(MKeyValue& atts) {}

};


// -----------------------------------------
// Handle manager storing objects of type TObj
template< class TObj >
class CObjManager : public CHandleManager {
	// The real objects...
	TObj* objs;
public:

	CObjManager(const char* the_obj_name) : objs(nullptr) {
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

			// Do the real copy
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

		// Copy the old object in the new object
		*new_obj = *orig_obj;

		return new_h;
	}

	void initHandlers() {
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++)
			obj->init();
	}

	void update(float elapsed) {
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++)
			obj->update(elapsed);
	}

	void fixedUpdate(float elapsed) {
		TObj* obj = objs;
		uint32_t num = num_objects_in_use;
		for (; num--; obj++)
			obj->fixedUpdate(elapsed);
	}


};

// ----------------------------------------
struct TTransform {     // 1
	XMVECTOR position;
	XMVECTOR rotation;
	XMVECTOR scale;

	TTransform() : position(XMVectorSet(0.f, 0.f, 0.f, 1.f)), rotation(XMQuaternionIdentity()), scale(XMVectorSet(1, 1, 1, 1)) {}
	TTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : position(np), rotation(nr), scale(ns) {}

	void loadFromAtts(MKeyValue& atts) {
		position = atts.getPoint("position");
		rotation = atts.getQuat("rotation");
		scale = atts.getPoint("scale");
	}

	XMMATRIX getWorld() const {
		XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMMATRIX m = XMMatrixAffineTransformation(scale, zero, rotation, position);
		return m;
	}

	XMVECTOR getFront() const {
		XMMATRIX m = XMMatrixRotationQuaternion(rotation);
		return m.r[2];
	}

	XMVECTOR getLeft() const {
		XMMATRIX m = XMMatrixRotationQuaternion(rotation);
		return m.r[0];
	}

	XMVECTOR getUp() const {
		XMMATRIX m = XMMatrixRotationQuaternion(rotation);
		return m.r[1];
	}

	// Returns true if the point is in the positive part of my front
	bool isInFront(XMVECTOR loc) const {
		return XMVectorGetX(XMVector3Dot(getFront(), loc - position)) > 0.f;
	}

	bool isInLeft(XMVECTOR loc) const {
		return XMVectorGetX(XMVector3Dot(getLeft(), loc - position)) > 0.f;
	}

	bool isInFov(XMVECTOR loc, float fov_in_rad) const {
		XMVECTOR unit_delta = XMVector3Normalize(loc - position);
		float cos_angle = XMVectorGetX(XMVector3Dot(getFront(), unit_delta));
		return(cos_angle < cos(fov_in_rad * 0.5f));
	}

	void lookAt(XMVECTOR new_target, XMVECTOR new_up_aux) {

		XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
		rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
	}

	void aimAt(XMVECTOR new_target, XMVECTOR new_up_aux, float t) {
		XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
		rotation = XMQuaternionSlerp(rotation, XMQuaternionInverse(XMQuaternionRotationMatrix(view)), t);
	}

	std::string toString() {
		return "Position: (" + std::to_string(XMVectorGetX(position)) + ", " + std::to_string(XMVectorGetY(position)) + ", " + std::to_string(XMVectorGetZ(position)) + ")\n" +
			"Rotation: (" + std::to_string(XMVectorGetX(rotation)) + ", " + std::to_string(XMVectorGetY(rotation)) + ", " + std::to_string(XMVectorGetZ(rotation)) + ", " + std::to_string(XMVectorGetW(rotation)) + ")\n" +
			"Scale: (" + std::to_string(XMVectorGetX(scale)) + ", " + std::to_string(XMVectorGetY(scale)) + ", " + std::to_string(XMVectorGetZ(scale)) + ")";
	}
};
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
struct TLife {    // 2 ...
	float life;

	TLife() : life(1.0f) { }
	TLife(float alife) : life(alife) { }

	void loadFromAtts(MKeyValue &atts) {
		life = atts.getFloat("life", 0.f);
	}

	std::string toString() {
		return "Life: " + std::to_string(life);
	}
};
struct TCompName {     // 1
	char name[32];
	TCompName() { name[0] = 0x00; }
	TCompName(const char *aname) { strcpy(name, aname); }
	void loadFromAtts(MKeyValue &atts) {
		strcpy(name, atts["name"].c_str());
	}

	std::string toString() {
		return name;
	}
};
struct TMesh {
	const CMesh* mesh;
	char path[32];

	TMesh() { mesh = nullptr; }
	TMesh(const CMesh* the_mesh) { mesh = the_mesh; strcpy(path, "unknown"); }
	TMesh(const char* the_name) { mesh = mesh_manager.getByName(the_name); strcpy(path, the_name); }

	void loadFromAtts(MKeyValue &atts) {
		strcpy(path, atts.getString("path", "missing_mesh").c_str());
		mesh = mesh_manager.getByName(path);
	}

	std::string toString() {
		return "Mesh: " + std::string(path);
	}
};
struct TCamera {
private:
	TTransform* transform;
public:

	XMMATRIX       view;            // Where is and where is looking at
	XMMATRIX       projection;      // Prespective info
	XMMATRIX       view_projection;

	XMVECTOR       target;          // Where we are looking at

	float          fov_in_radians;  // Field of view in radians
	float          aspect_ratio;
	float          znear, zfar;

	D3D11_VIEWPORT viewport;

	TCamera()
		: fov_in_radians(deg2rad(60.f))
		, znear(1.0f)
		, zfar(1000.f)
	{
		projection = XMMatrixIdentity();
	}

	void loadFromAtts(MKeyValue &atts) {
		target = atts.getPoint("target");
		zfar = atts.getFloat("zfar", 1000);
		znear = atts.getFloat("znear", 1);
		fov_in_radians = deg2rad(atts.getFloat("fov", 60));
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("camera");
		CEntity* e = hm->getOwner(this);
		transform = e->get<TTransform>();

		assert(transform || fatal("TCamera requieres a TTransform component"));

		transform->lookAt(target, transform->getUp());
		setViewport(0, 0, 512, 512);    // Will update projection matrix
	}

	void update(float elapsed) {
		// Update matrix
		updateViewProjection();
	}

	// -----------------------------------------------
	void setPerspective(float new_fov_in_rad, float new_znear, float new_zfar) {

		projection = XMMatrixPerspectiveFovRH(new_fov_in_rad, aspect_ratio, new_znear, new_zfar);

		fov_in_radians = new_fov_in_rad;
		znear = new_znear;
		zfar = new_zfar;

		updateViewProjection();
	}

	// -----------------------------------------------
	void updateViewProjection() {

		view = XMMatrixLookAtRH(transform->position, transform->position + transform->getFront(), XMVectorSet(0, 1, 0, 1));
		view_projection = view * projection;
	}


	// -----------------------------------------------
	void setViewport(float x0, float y0, float width, float height) {
		viewport.TopLeftX = x0;
		viewport.TopLeftY = y0;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;

		aspect_ratio = width / height;

		setPerspective(fov_in_radians, znear, zfar);
	}


	// -----------------------------------------------
	bool getScreenCoords(XMVECTOR world_coord, float *x, float *y) const {
		XMVECTOR homo_coords = XMVector3TransformCoord(world_coord, view_projection);
		float sx = XMVectorGetX(homo_coords);
		float sy = XMVectorGetY(homo_coords);
		float sz = XMVectorGetZ(homo_coords);

		if (sz < 0.f || sz > 1.f)
			return false;

		*x = viewport.TopLeftX + (sx * 0.5f + 0.5f) * viewport.Width;
		*y = viewport.TopLeftY + (-sy * 0.5f + 0.5f) * viewport.Height;

		return true;
	}

	std::string toString() {
		return "Target: (" + std::to_string(XMVectorGetX(target)) + ", " + std::to_string(XMVectorGetY(target)) + ", " + std::to_string(XMVectorGetZ(target)) + ")";
	}
};

struct TCollider {

	physx::PxShape* collider;

	TCollider() { }

	void loadFromAtts(MKeyValue &atts) {

		collider = Physics.gPhysicsSDK->createShape(
			physx::PxBoxGeometry(
				physx::PxReal(atts.getFloat("boxX", 0.5))
				, physx::PxReal(atts.getFloat("boxY", 0.5))
				, physx::PxReal(atts.getFloat("boxZ", 0.5))
			),
			*Physics.gPhysicsSDK->createMaterial(
				atts.getFloat("staticFriction", 0.5)
				, atts.getFloat("dynamicFriction", 0.5)
				, atts.getFloat("restitution", 0.5))
			,
			true);
	}

	void init() {
	}

	physx::PxMaterial* getMaterial() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
			return mat;
	}

	std::string toString() {
		return "Static friction: " + std::to_string(getMaterial()->getStaticFriction()) +
			"\nDynamic friction: " + std::to_string(getMaterial()->getDynamicFriction()) +
			"\nRestitution: " + std::to_string(getMaterial()->getRestitution());
	}
};

struct TRigidBody {
private:
	// Just for rigidbody creation
	float temp_density; 
	bool temp_is_kinematic;
	bool temp_use_gravity;
public:
	physx::PxRigidDynamic* rigidBody;

	TRigidBody() : 
		rigidBody(nullptr)
		, temp_density(1)
		, temp_is_kinematic( false )
		, temp_use_gravity( true ) 
	{}

	void loadFromAtts(MKeyValue &atts) {
		temp_density = atts.getFloat("density", 1);
		temp_is_kinematic = atts.getBool("kinematic", false);
		temp_use_gravity = atts.getBool("gravity", true);
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("rigidbody");
		CEntity* e = hm->getOwner(this);
		TTransform* t = e->get<TTransform>();
		TCollider* c = e->get<TCollider>();

		assert(t || fatal("TRigidBody requieres a TTransform component"));
		assert(c || fatal("TRigidBody requieres a TCollider component"));

		rigidBody = physx::PxCreateDynamic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
				Physics.XMVECTORToPxVec3(t->position),
				Physics.XMVECTORToPxQuat(t->rotation))
			, *c->collider
			, temp_density);
		Physics.gScene->addActor(*rigidBody);
		setKinematic(temp_is_kinematic);
		setUseGravity(temp_use_gravity);
	}

	void fixedUpdate(float elapsed) {
		CHandleManager* hm = CHandleManager::the_register.getByName("rigidbody");
		CEntity* e = hm->getOwner(this);
		TTransform* t = e->get<TTransform>();

		t->position = Physics.PxVec3ToXMVECTOR(rigidBody->getGlobalPose().p);
		t->rotation = Physics.PxQuatToXMVECTOR(rigidBody->getGlobalPose().q);		
	}

	void setKinematic(bool is_kinematic) {
		rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, is_kinematic);
	}

	bool isKinematic() {
		return rigidBody->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
	}

	void setUseGravity(bool use_gravity) {
		rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !use_gravity);
	}

	bool isUsingGravity() {
		return rigidBody->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
	}

	std::string toString() {
		return "Mass: " + std::to_string(rigidBody->getMass()) +
			"\nLinear velocity: " + Physics.toString(rigidBody->getLinearVelocity()) +
			"\nAngular velocity: " + Physics.toString(rigidBody->getAngularVelocity());
	}
};

struct TStaticBody {
public:
	physx::PxRigidStatic* staticBody;

	TStaticBody() : staticBody(nullptr) {}

	void loadFromAtts(MKeyValue &atts) {
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("staticbody");
		CEntity* e = hm->getOwner(this);
		TTransform* t = e->get<TTransform>();
		TCollider* c = e->get<TCollider>();

		assert(t || fatal("TStaticBody requieres a TTransform component"));
		assert(c || fatal("TStaticBody requieres a TCollider component"));

		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
				Physics.XMVECTORToPxVec3(t->position),
				Physics.XMVECTORToPxQuat(t->rotation))
			, *c->collider
			);
		
		Physics.gScene->addActor(*staticBody);
	}

	std::string toString() {
		return "Bounds: [" + Physics.toString(staticBody->getWorldBounds().minimum) + " ; " + Physics.toString(staticBody->getWorldBounds().maximum) + "]";
	}
};

struct TPlayerDoomController {
private:
	TTransform* transform;
public:

	physx::PxRigidDynamic*	 playerRigid; // Kinematic player. Should be changed to Physx Character Controller
	float movement_velocity;
	float rotation_velocity;

	TPlayerDoomController()
		: movement_velocity(5.0f)
		, rotation_velocity(deg2rad(90.f))
	{}

	void loadFromAtts(MKeyValue &atts) {
		movement_velocity = atts.getFloat("movementVelocity", 5);
		rotation_velocity = deg2rad(atts.getFloat("rotationVelocity", 90));

		// Kinematic player creation
		playerRigid = physx::PxCreateDynamic(*Physics.gPhysicsSDK, physx::PxTransform(physx::PxVec3(0, 0, 0)), physx::PxCapsuleGeometry(0.5f, 1.0f),
			*Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f), 100.0f);
		playerRigid->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		Physics.gScene->addActor(*playerRigid);
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("playerDoomController");
		CEntity* e = hm->getOwner(this);
		transform = e->get<TTransform>();

		assert(transform || fatal("TPlayerDoomController requieres a TTransform component"));

		// Teleport the kinematic player to the player position
		physx::PxVec3 position_player = Physics.XMVECTORToPxVec3(transform->position);
		position_player.y *= 0.5f;

		physx::PxQuat rotation_player = Physics.XMVECTORToPxQuat(transform->rotation);
		rotation_player *= physx::PxQuat(deg2rad(90), physx::PxVec3(0, 0, 1));

		playerRigid->setGlobalPose(physx::PxTransform(position_player, rotation_player), true);
	}

	void update(float elapsed) {
		XMVECTOR delta_pos = XMVectorZero();
		XMVECTOR delta_q = XMQuaternionIdentity();

		// Que teclas se pulsan -> que cambios hacer
		if (isKeyPressed('W'))
			delta_pos += elapsed * movement_velocity * transform->getFront();
		else if (isKeyPressed('S'))
			delta_pos -= elapsed * movement_velocity * transform->getFront();
		if (isKeyPressed('A'))
			delta_pos += elapsed * movement_velocity * transform->getLeft();
		else if (isKeyPressed('D'))
			delta_pos -= elapsed * movement_velocity * transform->getLeft();

		if (isKeyPressed('Q'))
			delta_q = XMQuaternionRotationAxis(transform->getUp(), elapsed * rotation_velocity);
		else if (isKeyPressed('E'))
			delta_q = XMQuaternionRotationAxis(transform->getUp(), -elapsed * rotation_velocity);

		// Actualizar la posicion/rotacion
		transform->position += delta_pos;
		transform->rotation = XMQuaternionMultiply(transform->rotation, delta_q);
	}

	void fixedUpdate(float elapsed) {
		// Kinematic player update
		// Rotate the capsule
		physx::PxVec3 position_player = Physics.XMVECTORToPxVec3(transform->position);
		position_player.y *= 0.5f;

		physx::PxQuat rotation_player = Physics.XMVECTORToPxQuat(transform->rotation);
		rotation_player *= physx::PxQuat(deg2rad(90), physx::PxVec3(0, 0, 1));		

		playerRigid->setKinematicTarget(physx::PxTransform(position_player, rotation_player));
	}


	std::string toString() {
		return "Player controller";
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
		transform->rotation = player->rotation;
		transform->lookAt(player->position + player->getUp() * 2, player->getUp());
	}

	std::string toString() {
		return "Camera controller";
	}
};

struct TAABB {
private:
	TTransform*		transform;
	XMVECTOR		last_position;	// Used to check if the transform has moved since the last frame
	XMVECTOR		bbpoints[8];	// Bounding Box with no rotation

	void recalcMinMax() {
		// Recalcultate AABB:
		// - Take the non rotated AABB min and max position and calculate the 8 bounding box points
		// - Apply a rotation to those points
		// - Find the min and max values for the points
		// - Set min and max new values		

		XMVECTOR rotatePoint;
		XMVECTOR minValue = XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, 1);
		XMVECTOR maxValue = -minValue;
		for (int i = 0; i < 8; ++i) {
			rotatePoint = XMVector3Rotate(bbpoints[i], transform->rotation);

			if (XMVectorGetX(rotatePoint) < XMVectorGetX(minValue))
				minValue = XMVectorSetX(minValue, XMVectorGetX(rotatePoint));
			if (XMVectorGetY(rotatePoint) < XMVectorGetY(minValue))
				minValue = XMVectorSetY(minValue, XMVectorGetY(rotatePoint));
			if (XMVectorGetZ(rotatePoint) < XMVectorGetZ(minValue))
				minValue = XMVectorSetZ(minValue, XMVectorGetZ(rotatePoint));

			if (XMVectorGetX(rotatePoint) > XMVectorGetX(maxValue))
				maxValue = XMVectorSetX(maxValue, XMVectorGetX(rotatePoint));
			if (XMVectorGetY(rotatePoint) > XMVectorGetY(maxValue))
				maxValue = XMVectorSetY(maxValue, XMVectorGetY(rotatePoint));
			if (XMVectorGetZ(rotatePoint) > XMVectorGetZ(maxValue))
				maxValue = XMVectorSetZ(maxValue, XMVectorGetZ(rotatePoint));
		}

		min = transform->position + minValue;
		max = transform->position + maxValue;
	}
public:

	XMVECTOR min;
	XMVECTOR max;
	XMVECTOR getCenter() { return (min + max) / 2; };
	XMVECTOR getExtents() { return (max - min) / 2; };
	XMVECTOR getSize() { return max - min; };

	TAABB() {}

	void loadFromAtts(MKeyValue &atts) {
		XMVECTOR identity_min = atts.getPoint("min");
		XMVECTOR identity_max = atts.getPoint("max");

		// Initialize bbpoints		
		bbpoints[0] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_min), XMVectorGetZ(identity_min), 1);
		bbpoints[1] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_min), XMVectorGetZ(identity_max), 1);
		bbpoints[2] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_max), XMVectorGetZ(identity_min), 1);
		bbpoints[3] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_max), XMVectorGetZ(identity_max), 1);
		bbpoints[4] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_min), XMVectorGetZ(identity_min), 1);
		bbpoints[5] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_min), XMVectorGetZ(identity_max), 1);
		bbpoints[6] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_max), XMVectorGetZ(identity_min), 1);
		bbpoints[7] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_max), XMVectorGetZ(identity_max), 1);
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("aabb");
		CEntity* e = hm->getOwner(this);
		transform = e->get<TTransform>();

		assert(transform || fatal("TAABB requieres a TTransform component"));

		last_position = transform->position;
		recalcMinMax();
	}

	void update(float elapsed) {
		if (!XMVectorGetX(XMVectorEqual(last_position, transform->position)))
			recalcMinMax();
	}

	float sqrDistance(XMVECTOR point) {
		XMVECTOR nearestPoint = XMVectorZero();
		nearestPoint = XMVectorSetX(nearestPoint, (XMVectorGetX(point) <  XMVectorGetX(min)) ? XMVectorGetX(min) : (XMVectorGetX(point) > XMVectorGetX(max)) ? XMVectorGetX(max) : XMVectorGetX(point));
		nearestPoint = XMVectorSetY(nearestPoint, (XMVectorGetY(point) <  XMVectorGetY(min)) ? XMVectorGetY(min) : (XMVectorGetY(point) > XMVectorGetY(max)) ? XMVectorGetY(max) : XMVectorGetY(point));
		nearestPoint = XMVectorSetZ(nearestPoint, (XMVectorGetZ(point) <  XMVectorGetZ(min)) ? XMVectorGetZ(min) : (XMVectorGetZ(point) > XMVectorGetZ(max)) ? XMVectorGetZ(max) : XMVectorGetZ(point));

		return XMVectorGetX(XMVector3LengthSq(nearestPoint - point));
	}

	void setMinMax(XMVECTOR n_min, XMVECTOR n_max) {
		min = n_min;
		max = n_max;
	}

	bool containts(XMVECTOR point) {
		return XMVector3InBounds(point - getCenter(), getExtents());
	}

	bool intersects(TAABB* aabb) {
		return XMVector3Greater(max, aabb->min) && XMVector3Greater(aabb->max, min);
	}

	std::string toString() {
		return "AABB Min: (" + std::to_string(XMVectorGetX(min)) + ", " + std::to_string(XMVectorGetY(min)) + ", " + std::to_string(XMVectorGetZ(min)) + ")" +
			   "\nAABB Max: (" + std::to_string(XMVectorGetX(max)) + ", " + std::to_string(XMVectorGetY(max)) + ", " + std::to_string(XMVectorGetZ(max)) + ")";
	}
};

// ----------------------------------------
// Declaracion que dado un tipo de c++
// hay una funcion que devuelve un manager de ese tipo
template< class TObj >
CObjManager<TObj>* getObjManager();

#endif


