#include "mcv_platform.h"
#include "comp_collider_mesh.h"
#include "../render/collision_mesh.h"

const CCollision_Mesh* c_m;

void TCompColliderMesh::loadFromAtts(const std::string& elem, MKeyValue &atts) {

    strcpy(path, ("colliders/" + atts.getString("path", "missing_mesh") + "_collider").c_str());
	c_m = mesh_collision_manager.getByName(path);

	physx::PxTriangleMesh* triangle_mesh = c_m->collision_mesh;

	physx::PxTriangleMeshGeometry geom(triangle_mesh);

	collider = Physics.gPhysicsSDK->createShape(geom,
		*Physics.gPhysicsSDK->createMaterial(
		atts.getFloat("staticFriction", 0.5)
		, atts.getFloat("dynamicFriction", 0.5)
		, atts.getFloat("restitution", 0.5))
		,
		true);

	addInputNavMesh();

	//Asignación de mascara al Shape para el filtrado de colisiones
	setupFiltering(collider, FilterGroup::eLEVEL, FilterGroup::eLEVEL);

	//Una vez creado el shape, no necesitamos el triangleMesh
	triangle_mesh->release();

	//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
}

void TCompColliderMesh::init() {
	const CCollision_Mesh* c_nv = mesh_collision_manager.getByName(path);
}

void TCompColliderMesh::addInputNavMesh(){
	TCompAABB* aabb_module = getSibling<TCompAABB>(this);
	TCompTransform* trans = getSibling<TCompTransform>(this);

	TTransform* t = trans;
	//t->transformPoint()
	XMFLOAT3 min;
	XMStoreFloat3(&min, aabb_module->min);
	XMFLOAT3 max;
	XMStoreFloat3(&max, aabb_module->max);

	const CCollision_Mesh* c_m = mesh_collision_manager.getByName(path);

	if (path){
		//const CCollision_Mesh* c_m = mesh_collision_manager.getByName(path);
		if (c_m){
			unsigned n_vertex = c_m->nvertexs;
			float* m_v = new float[n_vertex * 8];
			memcpy(m_v, c_m->vertex_floats, n_vertex * 8 * sizeof(float));

			unsigned n_triangles = c_m->nindices;
			int* t_v = new int[n_triangles];
			memcpy(t_v, c_m->index_int, n_triangles * sizeof(int));

			CNav_mesh_manager::get().nav_mesh_input.addInput(min, max, m_v, t_v, n_vertex, n_triangles, t, CNav_mesh_manager::get().nav_mesh_input.MODULE);
		}
	}
}