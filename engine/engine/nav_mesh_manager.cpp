#include "mcv_platform.h"
#include "nav_mesh_manager.h"
#include "entity_manager.h"
#include "components\comp_collider_mesh.h"
#include "components\comp_collider_box.h"
#include "components\comp_collider_sphere.h"
#include "components\comp_collider_capsule.h"

static CNav_mesh_manager the_nav_mesh_manager;
CNavmesh* nav_mesh;
CNavmesh 	nav_A;		// temporal 1
CNavmesh 	nav_B;		// temporal 2
std::mutex	generating_navmesh;	// mutex de control
std::mutex	updating_input_navmesh;	// mutex de control
bool		keep_updating_navmesh;
CEntityManager &entity_manager_S = CEntityManager::get();

CNav_mesh_manager& CNav_mesh_manager::get() {
	return the_nav_mesh_manager;
}

/*bool CNav_mesh_manager::build_nav_mesh(){
	nav_mesh->m_input = nav_mesh_input;
	nav_mesh->m_input.computeBoundaries();
	nav_mesh->build();
	return true;
}*/


bool CNav_mesh_manager::build_nav_mesh(){
	first = true;
	nav_A.m_input = nav_mesh_input;
	nav_A.m_input.computeBoundaries();
	nav_B.m_input = nav_mesh_input;
	nav_B.m_input.computeBoundaries();
	nav_A.build();
	nav_mesh = &nav_A;
	keep_updating_navmesh = true;
	//clearMeshManager();
	new std::thread(&CNav_mesh_manager::updateNavmesh, this);
	return true;
}

void CNav_mesh_manager::prepareInputNavMesh(){
	for (int i = 0; i < entity_manager_S.getEntities().size(); ++i){
		TCompColliderMesh* collider_mesh = ((CEntity*)entity_manager_S.getEntities()[i])->get<TCompColliderMesh>();
		TCompColliderBox* collider_box = ((CEntity*)entity_manager_S.getEntities()[i])->get<TCompColliderBox>();
		TCompColliderSphere* collider_sphere = ((CEntity*)entity_manager_S.getEntities()[i])->get<TCompColliderSphere>();
		TCompColliderCapsule* collider_capsule = ((CEntity*)entity_manager_S.getEntities()[i])->get<TCompColliderCapsule>();
		if (collider_mesh){
			//collider_mesh->init();
			collider_mesh->addInputNavMesh();
		}else if (collider_box){
			collider_box->addInputNavMesh();
		}else if (collider_sphere){
			collider_sphere->addInputNavMesh();
		}else if (collider_capsule){
			collider_capsule->addInputNavMesh();
		}
		/*if (i == entity_manager_S.getEntities().size() - 1){
			collider_mesh->clearMeshManager();
		}*/
	}
	nav_A.m_input = nav_mesh_input;
	nav_A.m_input.computeBoundaries();
	nav_B.m_input = nav_mesh_input;
	nav_B.m_input.computeBoundaries();
}

void CNav_mesh_manager::updateNavmesh() {
	while (keep_updating_navmesh) {

		// seleccionamos navmesh a actualizar (las actualizamso alternativamente)
		CNavmesh* updated_nav = nav_mesh == &nav_A ? &nav_B : &nav_A;

		// generamos la navmesh con los datos actualizados
		updated_nav->build();

		// activamos el mutex para asegurarnos de no acceder simultáneamente a una consulta de la IA
		generating_navmesh.lock();

		// hacemos el swap de los datos de la navmesh
		nav_mesh = updated_nav;

		// desactivamos el mutex
		generating_navmesh.unlock();

		// esperamos un poco antes de volver a actulizarla
		std::this_thread::sleep_for(std::chrono::seconds(1));

		nav_mesh_input.clearInput();
		prepareInputNavMesh();
	}
}

void CNav_mesh_manager::render_nav_mesh(){
	//nav_mesh.render();
	if (nav_mesh){
		const dtNavMesh* navmesh = nav_mesh->m_navMesh;
		if (navmesh){
			int totalTiles = navmesh->getMaxTiles();
			for (int i = 0; i < totalTiles; i++){
				render_tile(navmesh->getTile(i));
			}
		}
	}
	//nav_mesh.m_draw_mode = CNavmesh::EDrawMode::NAVMESH_DRAW_COUNTOURS;
	//nav_mesh.render(true);
}

void CNav_mesh_manager::render_tile(const dtMeshTile* tile){
	
	int numPolys=tile->header->polyCount;
	int pairs = 0;
	int indx = 0;
	std::vector<XMVECTOR>vtxs;
	for (int i = 0; i < numPolys; i++){
		dtPoly current_poly = tile->polys[i];
		XMVECTOR last=XMVectorSet(0.f,0.f,0.f,0.f);
		XMVECTOR current=XMVectorSet(0.f, 0.f, 0.f, 0.f);
		bool first = true;
		for (int j = 0; j < current_poly.vertCount; j++){
			unsigned short indx = current_poly.verts[j];
			if (!first){
				unsigned short indx_last = current_poly.verts[j-1];
				current = XMVectorSet(tile->verts[indx * 3], tile->verts[(indx * 3) + 1], tile->verts[(indx * 3) + 2], 1);
				last = XMVectorSet(tile->verts[indx_last * 3], tile->verts[(indx_last * 3) + 1], tile->verts[(indx_last * 3) + 2], 1);
				drawLine(last, current);
				if (j == current_poly.vertCount - 1){
					unsigned short first_indx = current_poly.verts[0];
					XMVECTOR first_vtx = XMVectorSet(tile->verts[first_indx * 3], tile->verts[(first_indx * 3) + 1], tile->verts[(first_indx * 3) + 2], 1);
					drawLine(current, first_vtx);
				}
			}
			first = false;
		}
	}
}

void CNav_mesh_manager::findPath(XMVECTOR pst_src, XMVECTOR pst_dst, XMVECTOR *straightPath, int &numPoints){
	CNavmeshQuery navMeshQuery(nav_mesh);
	if (nav_mesh){
		const dtNavMesh* navmesh = nav_mesh->m_navMesh;
		if (navmesh){
			navMeshQuery.resetTools();
			navMeshQuery.updatePos(pst_src, pst_dst);
			navMeshQuery.setTool(CNavmeshQuery::ETool::FIND_PATH);
			navMeshQuery.findStraightPath();
			num = navMeshQuery.numPointsStraightPath;
			prue = navMeshQuery.straightPath;
			int ind = 0;
			XMVECTOR* aux_array = new XMVECTOR[navMeshQuery.numPointsStraightPath];
			for (int i = 0; i <navMeshQuery.numPointsStraightPath; ++i){
				XMVECTOR point = XMVectorSet(navMeshQuery.straightPath[i * 3], navMeshQuery.straightPath[i * 3 + 1] + 0.10f, navMeshQuery.straightPath[i * 3 + 2], 1);
				aux_array[ind] = point;
				ind++;
			}
			memcpy(straightPath, aux_array, navMeshQuery.numPointsStraightPath*sizeof(XMVECTOR));
			numPoints = navMeshQuery.numPointsStraightPath;
		}
	}
}

void CNav_mesh_manager::pathRender(){
	if (prue) {
		for (int i = 0; i <num; ++i){
			if (i>0){
				XMVECTOR p1 = XMVectorSet(prue[(i - 1) * 3], prue[((i - 1) * 3) + 1]+0.10f, prue[((i - 1) * 3) + 2], 1);
				XMVECTOR p2 = XMVectorSet(prue[i * 3], prue[i * 3 + 1] + 0.10f, prue[i * 3 + 2], 1);
				drawLine(p1, p2);
			}
		}
	}
}

CNav_mesh_manager::CNav_mesh_manager()
{
}


CNav_mesh_manager::~CNav_mesh_manager()
{
}
