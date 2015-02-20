#include "mcv_platform.h"
#include "entity_inspector.h"
#include <AntTweakBar.h>
#include "handle\handle.h"
#include <locale>
#include <algorithm>

using namespace physx;

CEntityInspector::CEntityInspector() { }

CEntityInspector::~CEntityInspector() { }

TwBar *bar;

XMVECTOR center;
XMVECTOR extents;
XMVECTOR size;

// Rigidbody
PxVec3 linearVelocity;
PxVec3 angularVelocity;


void CEntityInspector::init() {
	// Create a tewak bar
	bar = TwNewBar("Inspector");
	CApp &app = CApp::get();
	
	// AntTweakBar test
	int barSize[2] = { 224, app.yres };
	int varPosition[2] = { app.xres - barSize[0], 0 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Inspector label='Entity inspector' ");
	TwDefine(" Inspector refresh='0.1' ");
	TwDefine(" TW_HELP visible=false ");

}

// ---------------------------- GET AND SET COMPONENTS --------------------------

// TRANSFORM
void TW_CALL GetPitch(void *value, void *clientData)
{
	*static_cast<float *>(value) = rad2deg( getPitchFromVector(static_cast<TTransform *>(clientData)->getFront()) );
}

// COLLIDER
void TW_CALL SetStaticFriction(const void *value, void *clientData)
{
	static_cast<TCollider *>(clientData)->getMaterial()->setStaticFriction(*static_cast<const float *>(value));
}
void TW_CALL GetStaticFriction(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCollider *>(clientData)->getMaterial()->getStaticFriction();
}

void TW_CALL SetDynamicFriction(const void *value, void *clientData)
{
	static_cast<TCollider *>(clientData)->getMaterial()->setDynamicFriction(*static_cast<const float *>(value));
}
void TW_CALL GetDynamicFriction(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCollider *>(clientData)->getMaterial()->getDynamicFriction();
}

void TW_CALL SetRestitution(const void *value, void *clientData)
{
	static_cast<TCollider *>(clientData)->getMaterial()->setRestitution(*static_cast<const float *>(value));
}
void TW_CALL GetRestitution(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCollider *>(clientData)->getMaterial()->getRestitution();
}

// RIGIDBODY
void TW_CALL SetMass(const void *value, void *clientData)
{
	PxRigidBodyExt::setMassAndUpdateInertia(*static_cast<TRigidBody *>(clientData)->rigidBody, *static_cast<const float *>(value), NULL, false);
}
void TW_CALL GetMass(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TRigidBody *>(clientData)->rigidBody->getMass();
}

void TW_CALL SetGravity(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->setUseGravity(*static_cast<const bool *>(value));
}
void TW_CALL GetGravity(void *value, void *clientData)
{
	*static_cast<bool *>(value) = static_cast<TRigidBody *>(clientData)->isUsingGravity();
}

void TW_CALL SetKinematic(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->setKinematic(*static_cast<const bool *>(value));
}
void TW_CALL GetKinematic(void *value, void *clientData)
{
	*static_cast<bool *>(value) = static_cast<TRigidBody *>(clientData)->isKinematic();
}

void TW_CALL SetLinearDamping(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->rigidBody->setLinearDamping(*static_cast<const float *>(value));
}
void TW_CALL GetLinearDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TRigidBody *>(clientData)->rigidBody->getLinearDamping();
}

void TW_CALL SetAngularDamping(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->rigidBody->setAngularDamping(*static_cast<const float *>(value));
}
void TW_CALL GetAngularDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TRigidBody *>(clientData)->rigidBody->getAngularDamping();
}


// MESH
void TW_CALL ReloadMesh(const void *value, void *clientData)
{
	(static_cast<TMesh *>(clientData))->mesh = mesh_manager.getByName((*(const std::string *)value).c_str());
	TMesh* mesh = static_cast<TMesh *>(clientData);
	std::strcpy(mesh->path, (*(const std::string *)value).c_str());
}
void TW_CALL GetMeshPath(void *value, void *clientData)
{	
	std::string *destPtr = static_cast<std::string *>(value);	
	TwCopyStdStringToLibrary(*destPtr, static_cast<TMesh *>(clientData)->path);
	
}
// ---------------------------- ADD COMPONENT CALLBACKS --------------------------
void TW_CALL AddTransform(void *clientData) {

	TTransform* t = CHandle::create<TTransform>();
	static_cast<CEntity *>(clientData)->add(t);
	CApp::get().entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}

void TW_CALL AddMesh(void *clientData) {

	TMesh* m = CHandle::create<TMesh>();
	std::strcpy(m->path, "teapot");
	m->mesh = mesh_manager.getByName("teapot");
	static_cast<CEntity *>(clientData)->add(m);
	CApp::get().entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}

void CEntityInspector::update() {
	TAABB* e_aabb = target_entity->get<TAABB>();
	TRigidBody* e_rigidbody = target_entity->get<TRigidBody>();

	if (e_aabb) {
		center = e_aabb->getCenter();
		extents = e_aabb->getExtents();
		size = e_aabb->getSize();
	}
	if (e_rigidbody) {
		linearVelocity = e_rigidbody->rigidBody->getLinearVelocity();
		angularVelocity = e_rigidbody->rigidBody->getAngularVelocity();
	}
}

void CEntityInspector::inspectEntity(CEntity* the_entity) {
	target_entity = the_entity;

	TTransform* e_transform = target_entity->get<TTransform>();
	TCompName* e_name = target_entity->get<TCompName>();
	TMesh* e_mesh = target_entity->get<TMesh>();
	TAABB* e_aabb = target_entity->get<TAABB>();
	TCollider* e_collider = target_entity->get<TCollider>();
	TRigidBody* e_rigidbody = target_entity->get<TRigidBody>();
	TCamera* e_camera = target_entity->get<TCamera>();
	TPlayerController* e_player_controller = target_entity->get<TPlayerController>();
	TPlayerPivotController* e_player_pivot_controller = target_entity->get<TPlayerPivotController>();
	TCameraPivotController* e_camera_pivot_controller = target_entity->get<TCameraPivotController>();
	TThirdPersonCameraController* e_third_person_camera_controller = target_entity->get<TThirdPersonCameraController>();

	TwRemoveAllVars(bar);
	if (e_name) {
		TwAddVarRW(bar, "NActive", TW_TYPE_BOOL8, &e_name->active, " group=Name label='Active'");
		TwAddVarRW(bar, "CName", TW_TYPE_CSSTRING(sizeof(e_name->name)), e_name->name, " group=Name label='Name'");
		TwAddSeparator(bar, "Name", "");
	}
	if (e_transform) {
		TwAddVarRW(bar, "TActive", TW_TYPE_BOOL8, &e_transform->active, " group=Transform label='Active'");
		TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &e_transform->position, " group=Transform");
		TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &e_transform->rotation, " group=Transform");
		// Yaw Pitch Roll
		TwAddVarCB(bar, "Pich", TW_TYPE_FLOAT, NULL, GetPitch, e_transform, " group=Transform");
		TwAddVarRW(bar, "Scale", TW_TYPE_DIR3F, &e_transform->scale, " group=Transform");
		TwAddSeparator(bar, "Transform", "");
	}
	if (e_mesh) {
		TwAddVarRW(bar, "MActive", TW_TYPE_BOOL8, &e_mesh->active, " group=Mesh label='Active'");
		//TwAddVarRW(bar, "Color", TW_TYPE_COLOR4F, &e_mesh->color, " group=Mesh");
		TwAddVarRW(bar, "LightDir", TW_TYPE_DIR3F, &e_mesh->color, " group=Mesh");
		TwAddVarCB(bar, "Path", TW_TYPE_STDSTRING, ReloadMesh, GetMeshPath, e_mesh, " group=Mesh");
	}
	if (e_collider) {
		TwAddVarRW(bar, "CActive", TW_TYPE_BOOL8, &e_collider->active, " group=Collider label='Active'");
		TwAddVarCB(bar, "Static friction", TW_TYPE_FLOAT, SetStaticFriction, GetStaticFriction, e_collider, " min=0 max=1 step=0.1 group=Collider");
		TwAddVarCB(bar, "Dynamic friction", TW_TYPE_FLOAT, SetDynamicFriction, GetDynamicFriction, e_collider, " min=0 max=1 step=0.1 group=Collider");
		TwAddVarCB(bar, "Restitution", TW_TYPE_FLOAT, SetRestitution, GetRestitution, e_collider, " min=0 max=1 step=0.1 group=Collider");
	}
	if (e_rigidbody) {
		TwAddVarRW(bar, "RActive", TW_TYPE_BOOL8, &e_rigidbody->active, " group=Rigidbody label='Active'");
		TwAddVarCB(bar, "Mass", TW_TYPE_FLOAT, SetMass, GetMass, e_rigidbody, " min=0.1 group=Rigidbody");
		TwAddVarCB(bar, "Damping", TW_TYPE_FLOAT, SetLinearDamping, GetLinearDamping, e_rigidbody, " min=0 group=Rigidbody");
		TwAddVarCB(bar, "Angular Damping", TW_TYPE_FLOAT, SetAngularDamping, GetAngularDamping, e_rigidbody, " min=0 group=Rigidbody");
		TwAddVarCB(bar, "Use Gravity", TW_TYPE_BOOL8, SetGravity, GetGravity, e_rigidbody, " group=Rigidbody");
		TwAddVarCB(bar, "Is Kinematic", TW_TYPE_BOOL8, SetKinematic, GetKinematic, e_rigidbody, " group=Rigidbody");
		TwAddVarRO(bar, "Linear velocity", TW_TYPE_DIR3F, &linearVelocity, " group=Rigidbody");
		TwAddVarRO(bar, "Angular velocity", TW_TYPE_DIR3F, &angularVelocity, " group=Rigidbody");
	}	
	if (e_aabb) {
		TwAddVarRW(bar, "AABBActive", TW_TYPE_BOOL8, &e_aabb->active, " group=AABB label='Active'");
		TwAddVarRW(bar, "Min", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB");
		TwAddVarRW(bar, "Max", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB");
		TwAddVarRO(bar, "Center", TW_TYPE_DIR3F, &center, " group=AABB");
		TwAddVarRO(bar, "Extents", TW_TYPE_DIR3F, &extents, " group=AABB");
		TwAddVarRO(bar, "Size", TW_TYPE_DIR3F, &size, " group=AABB");
	}

	if (e_camera) {
		TwAddVarRW(bar, "CameraActive", TW_TYPE_BOOL8, &e_camera->active, " group=Camera label='Active'");
		TwAddVarRW(bar, "Fov", TW_TYPE_FLOAT, &e_camera->fov_in_radians, " group=Camera");
		TwAddVarRW(bar, "Aspect ratio", TW_TYPE_FLOAT, &e_camera->aspect_ratio, " group=Camera");	
		TwAddVarRW(bar, "Znear", TW_TYPE_FLOAT, &e_camera->znear, " group=Camera");
		TwAddVarRW(bar, "Zfar", TW_TYPE_FLOAT, &e_camera->zfar, " group=Camera");
	}

	if (e_player_controller) {
		TwAddVarRW(bar, "PlayerControllerActive", TW_TYPE_BOOL8, &e_player_controller->active, " group='Player Controller' label='Active'");
		TwAddVarRW(bar, "PlayerControllerMovement velocity", TW_TYPE_FLOAT, &e_player_controller->movement_velocity, " group='Player Controller' label='Movement velocity'");
		TwAddVarRW(bar, "PlayerControllerRotation velocity", TW_TYPE_FLOAT, &e_player_controller->rotation_velocity, " group='Player Controller' label='Rotation velocity'");
	}

	if (e_player_pivot_controller) {
		TwAddVarRW(bar, "PlayerPivotControllerActive", TW_TYPE_BOOL8, &e_player_pivot_controller->active, " group='Player Pivot Controller' label='Active' ");
		TwAddVarRW(bar, "PlayerPivotRotation velocity", TW_TYPE_FLOAT, &e_player_pivot_controller->rotation_velocity, " group='Player Pivot Controller' label='Rotation velocity'");
	}

	if (e_camera_pivot_controller) {
		TwAddVarRW(bar, "CameraPivotControllerActive", TW_TYPE_BOOL8, &e_camera_pivot_controller->active, " group='Camera Pivot Controller' label='Active' ");
		TwAddVarRW(bar, "CameraPivotControllerOffset", TW_TYPE_DIR3F, &e_camera_pivot_controller->offset, " group='Camera Pivot Controller' label='Offset'");
		TwAddVarRW(bar, "Tilt velocity", TW_TYPE_FLOAT, &e_camera_pivot_controller->tilt_velocity, " group='Camera Pivot Controller' label='Tilt velocity'");
		TwAddVarRW(bar, "Min tilt", TW_TYPE_FLOAT, &e_camera_pivot_controller->min_tilt, " group='Camera Pivot Controller' label='Min tilt'");
		TwAddVarRW(bar, "Max tilt", TW_TYPE_FLOAT, &e_camera_pivot_controller->max_tilt, " group='Camera Pivot Controller' label='Max tilt'");
	}

	if (e_third_person_camera_controller) {
		TwAddVarRW(bar, "ThirdPersonCameraControllerActive", TW_TYPE_BOOL8, &e_third_person_camera_controller->active, " group='Third Person Camera Controller' label='Active' ");
		TwAddVarRW(bar, "ThirdPersonCameraControllerOffset", TW_TYPE_DIR3F, &e_third_person_camera_controller->offset, " group='Third Person Camera Controller' label='Offset'");
	}

	TwAddSeparator(bar, "", "");
	TwAddButton(bar, "Add components", NULL, NULL, "");

	if (!e_transform) {
		TwAddButton(bar, "Transform", AddTransform, target_entity, "");
	}
	if (!e_mesh) {
		TwAddButton(bar, "Mesh", AddMesh, target_entity, "");
	}
}


// ----------------------------

TwBar *lister_bar;
std::vector<std::string> entity_names;

CEntityLister::CEntityLister() { searchIn = ""; }

CEntityLister::~CEntityLister() { }

void TW_CALL CallbackInspectEntity(void *clientData) {
	CApp::get().entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
	destinationClientString = sourceLibraryString;
}

size_t LevenshteinDistance(std::string s1, std::string s2)
{
	const size_t m(s1.size());
	const size_t n(s2.size());

	if (m == 0) return n;
	if (n == 0) return m;

	size_t *costs = new size_t[n + 1];

	for (size_t k = 0; k <= n; k++) costs[k] = k;

	size_t i = 0;
	for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
	{
		costs[0] = i + 1;
		size_t corner = i;

		size_t j = 0;
		for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
		{
			size_t upper = costs[j + 1];
			if (*it1 == *it2)
			{
				costs[j + 1] = corner;
			}
			else
			{
				size_t t(upper<corner ? upper : corner);
				costs[j + 1] = (costs[j]<t ? costs[j] : t) + 1;
			}

			corner = upper;
		}
	}

	size_t result = costs[n];
	delete[] costs;

	return result;
}

bool searchCompare(std::string s1, std::string s2) {
	
	std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
	std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);

	if (s1 == s2)
		return true;

	bool contains1 = s1.find(s2) != std::string::npos;
	bool contains2 = s2.find(s1) != std::string::npos;

	if (contains1 || contains2)
		return true;

	int l_dist = LevenshteinDistance(s1, s2);
	if (l_dist < 4)
		return true;
	return false;
}

void CEntityLister::init() {
	// Create a tewak bar
	lister_bar = TwNewBar("Lister");
	TwCopyStdStringToClientFunc(CopyStdStringToClient);
	
	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, app.yres - 120 };
	int varPosition[2] = { 0, 0 };
	TwSetParam(lister_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(lister_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Lister label='Entity list' ");
	TwDefine(" Lister refresh='0.3' ");
	TwDefine(" TW_HELP visible=false ");

	TwAddButton(lister_bar, "Search", NULL, NULL, "");
	TwAddVarRW(lister_bar, "EntitySearchInput", TW_TYPE_STDSTRING, &searchIn, "label='Search entities'");
	TwAddSeparator(lister_bar, "", "");
	TwAddButton(lister_bar, "Entities", NULL, NULL, "");

	m_entity_event_count = -1;
	prevSearch = "";
}

void CEntityLister::update() {

	// if the entities has changed
	if ((m_entity_event_count != CEntityManager::get().getEntityEventCount()) || (prevSearch != searchIn)) {
		m_entity_event_count = CEntityManager::get().getEntityEventCount();
		// Get the entities currently alive
		std::vector< CHandle > entities = CEntityManager::get().getEntities();

		// Loop the current listed entities vector and remove the bars with those names
		for (int n = 0; n < entity_names.size(); ++n) {
			// Remove the var
			TwRemoveVar(lister_bar, entity_names[n].c_str());
		}

		for (int i = 0; i < entities.size(); ++i) {
			// Get the entity names
			TCompName* e_name = ((CEntity*)entities[i])->get<TCompName>();
			if (e_name) {
				std::string a = std::string(searchIn);
				if (a.empty() || searchCompare(a, e_name->name)) {
					// Add to the listed entities vector
					entity_names.push_back(e_name->name);
					TwAddButton(lister_bar, e_name->name, CallbackInspectEntity, (CEntity*)entities[i], "");
					
				}
			}
		}

		prevSearch = searchIn;
	}	
}

// ----------------------------

TwBar *actioner_bar;

CEntityActioner::CEntityActioner() {}

CEntityActioner::~CEntityActioner() { }

// AntTweakBar button test
int entityCounter = 0;
void TW_CALL CallbackCreateEntity(void *clientData)
{

	// Create a new entity with some components
	CEntity* e = CEntityManager::get().createEmptyEntity();

	TCompName* n = getObjManager<TCompName>()->createObj();
	std::strcpy(n->name, ("Nueva entidad" + std::to_string(entityCounter)).c_str());
	e->add(n);

	entityCounter++;
}

void CEntityActioner::init() {
	// Create a tewak bar
	actioner_bar = TwNewBar("Actioner");

	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, 224 };
	int varPosition[2] = { 0, app.yres - 120 };
	TwSetParam(actioner_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(actioner_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Lister label='Entity actions' ");
	TwDefine(" Lister refresh='0.3' ");
}

void CEntityActioner::update() {

	TwRemoveAllVars(actioner_bar);

	std::vector< CHandle > entities = CEntityManager::get().getEntities();

	TwAddButton(actioner_bar, "New entity", CallbackCreateEntity, NULL, "");


}


// ------------------------------------------------------
TwBar *debug_bar;

CDebugOptioner::CDebugOptioner()
{}

CDebugOptioner::~CDebugOptioner() { }

void CDebugOptioner::init() {
	// Create a tewak bar
	debug_bar = TwNewBar("DebugOptioner");

	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, 120 };
	int varPosition[2] = { 240, 20 };
	TwSetParam(debug_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(debug_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" DebugOptioner label='Debug options' ");
	TwDefine(" DebugOptioner refresh='2' ");

	TwAddVarRW(debug_bar, "Draw grid", TW_TYPE_BOOL8, &app.renderGrid, "");
	TwAddVarRW(debug_bar, "Draw axis", TW_TYPE_BOOL8, &app.renderAxis, "");
	TwAddVarRW(debug_bar, "Draw names", TW_TYPE_BOOL8, &app.renderNames, "");
	TwAddVarRW(debug_bar, "Draw AABBs", TW_TYPE_BOOL8, &app.renderAABB, "");
}