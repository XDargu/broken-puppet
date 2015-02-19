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
#include "../entity_manager.h"

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

	// Aim the transform to a position instantly
	void lookAt(XMVECTOR new_target, XMVECTOR new_up_aux) {

		XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
		rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
	}

	// Aim the transform to a position with SLerp
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

	void onExplosion(const TMsgExplosion& msg) {
		dbg("Life recv explosion of %f points when my life is %f\n", msg.damage, life);
		life -= msg.damage;
		if (life < 0)
			life = 0;
	}
	void onDied(const TMsgDied& msg) {
		dbg("Life recv died from %d when my life is %f\n", msg.who, life);
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
	XMVECTOR color;

	TMesh() { mesh = nullptr; color = XMVectorSet(1, 1, 1, 1); }
	TMesh(const CMesh* the_mesh) { mesh = the_mesh; strcpy(path, "unknown"); color = XMVectorSet(1, 1, 1, 1); }
	TMesh(const char* the_name) { mesh = mesh_manager.getByName(the_name); strcpy(path, the_name); color = XMVectorSet(1, 1, 1, 1); }

	void loadFromAtts(MKeyValue &atts) {
		strcpy(path, atts.getString("path", "missing_mesh").c_str());
		mesh = mesh_manager.getByName(path);
		color = atts.getQuat("color");
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

	// Returns the material properties as a vector
	XMVECTOR getMaterialProperties() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return XMVectorSet(
			mat->getStaticFriction(),
			mat->getDynamicFriction(),
			mat->getRestitution(),
			0
			);
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
		, temp_is_kinematic(false)
		, temp_use_gravity(true)
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
		return !rigidBody->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
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
		transform->lookAt(player->position + player->getUp() * 2, player->getUp());
	}

	std::string toString() {
		return "Camera controller";
	}
};

struct TAABB {
private:
	TTransform*		transform;
	XMVECTOR		bbpoints[8];	// Bounding Box with no rotation

	// Used to check if the transform has been moved since the last frame, should be moved to transform component
	XMVECTOR prev_position;
	XMVECTOR prev_rotation;
	XMVECTOR prev_scale;

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
			rotatePoint = XMVector3Rotate(bbpoints[i] * transform->scale, transform->rotation);

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

		setIdentityMinMax(identity_min, identity_max);
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("aabb");
		CEntity* e = hm->getOwner(this);
		transform = e->get<TTransform>();

		assert(transform || fatal("TAABB requieres a TTransform component"));

		prev_position = transform->position;
		prev_rotation = transform->rotation;
		prev_scale = transform->scale;
		recalcMinMax();
	}

	// Updates the min and max variables, if needed
	void update(float elapsed) {
		CHandleManager* hm = CHandleManager::the_register.getByName("aabb");
		CEntity* e = hm->getOwner(this);
		TCompName* name = e->get<TCompName>();

		bool posEqual = XMVectorGetX(XMVectorEqual(prev_position, transform->position)) && XMVectorGetY(XMVectorEqual(prev_position, transform->position)) && XMVectorGetZ(XMVectorEqual(prev_position, transform->position));
		bool rotEqual = XMVectorGetX(XMVectorEqual(prev_rotation, transform->rotation)) && XMVectorGetY(XMVectorEqual(prev_rotation, transform->rotation)) && XMVectorGetZ(XMVectorEqual(prev_rotation, transform->rotation)) && XMVectorGetW(XMVectorEqual(prev_rotation, transform->rotation));
		bool sclEqual = XMVectorGetX(XMVectorEqual(prev_scale, transform->scale)) && XMVectorGetY(XMVectorEqual(prev_scale, transform->scale)) && XMVectorGetZ(XMVectorEqual(prev_scale, transform->scale));

		if (!(posEqual && rotEqual && sclEqual))
			recalcMinMax();

		prev_position = transform->position;
		prev_rotation = transform->rotation;
		prev_scale = transform->scale;
	}

	// Squared distance from a point to the AABB
	float sqrDistance(XMVECTOR point) {
		XMVECTOR nearestPoint = XMVectorZero();
		nearestPoint = XMVectorSetX(nearestPoint, (XMVectorGetX(point) <  XMVectorGetX(min)) ? XMVectorGetX(min) : (XMVectorGetX(point) > XMVectorGetX(max)) ? XMVectorGetX(max) : XMVectorGetX(point));
		nearestPoint = XMVectorSetY(nearestPoint, (XMVectorGetY(point) <  XMVectorGetY(min)) ? XMVectorGetY(min) : (XMVectorGetY(point) > XMVectorGetY(max)) ? XMVectorGetY(max) : XMVectorGetY(point));
		nearestPoint = XMVectorSetZ(nearestPoint, (XMVectorGetZ(point) <  XMVectorGetZ(min)) ? XMVectorGetZ(min) : (XMVectorGetZ(point) > XMVectorGetZ(max)) ? XMVectorGetZ(max) : XMVectorGetZ(point));

		return XMVectorGetX(XMVector3LengthSq(nearestPoint - point));
	}

	// Sets the identity rotation AABB points
	void setIdentityMinMax(XMVECTOR identity_min, XMVECTOR identity_max) {

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

	// Check if the AABB contains a point
	bool containts(XMVECTOR point) {
		return XMVector3InBounds(point - getCenter(), getExtents());
	}

	// Checks if the aabb intersects with another one
	bool intersects(TAABB* aabb) {
		return XMVector3Greater(max, aabb->min) && XMVector3Greater(aabb->max, min);
	}

	std::string toString() {
		return "AABB Min: (" + std::to_string(XMVectorGetX(min)) + ", " + std::to_string(XMVectorGetY(min)) + ", " + std::to_string(XMVectorGetZ(min)) + ")" +
			"\nAABB Max: (" + std::to_string(XMVectorGetX(max)) + ", " + std::to_string(XMVectorGetY(max)) + ", " + std::to_string(XMVectorGetZ(max)) + ")";
	}
};

struct TEnemyWithPhysics {
private:
	TTransform* transform;
public:
	physx::PxRigidDynamic*	 playerRigid; // Kinematic player. Should be changed to Physx Character Controller

	TEnemyWithPhysics()
	{}

	void loadFromAtts(MKeyValue &atts) {

		// Kinematic player creation
		playerRigid = physx::PxCreateDynamic(*Physics.gPhysicsSDK, physx::PxTransform(physx::PxVec3(0, 0, 0)), physx::PxCapsuleGeometry(0.5f, 1.0f),
			*Physics.gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f), 100.0f);
		playerRigid->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		Physics.gScene->addActor(*playerRigid);
	}

	void init() {
		CHandleManager* hm = CHandleManager::the_register.getByName("enemyWithPhysics");
		CEntity* e = hm->getOwner(this);
		transform = e->get<TTransform>();

		assert(transform || fatal("TEnemyWithPhysics requieres a TTransform component"));

		// Teleport the kinematic player to the player position
		physx::PxVec3 position_player = Physics.XMVECTORToPxVec3(transform->position);
		position_player.y *= 0.5f;

		physx::PxQuat rotation_player = Physics.XMVECTORToPxQuat(transform->rotation);
		rotation_player *= physx::PxQuat(deg2rad(90), physx::PxVec3(0, 0, 1));

		playerRigid->setGlobalPose(physx::PxTransform(position_player, rotation_player), true);
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
		return "Enemy with physics controller";
	}
};

#endif


