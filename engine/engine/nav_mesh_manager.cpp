#include "mcv_platform.h"
#include "nav_mesh_manager.h"
#include "entity_manager.h"
#include "physics_manager.h"

static CNav_mesh_manager the_nav_mesh_manager;
CNavmesh* nav_mesh;
CNavmesh 	nav_A;		// temporal 1
CNavmesh 	nav_B;		// temporal 2
std::mutex	generating_navmesh;	// mutex de control

CNav_mesh_manager& CNav_mesh_manager::get() {
	return the_nav_mesh_manager;
}

bool CNav_mesh_manager::build_nav_mesh(){
	first = true;
	nav_A.m_input = nav_mesh_input;
	nav_A.m_input.computeBoundaries();
	nav_B.m_input = nav_mesh_input;
	nav_B.m_input.computeBoundaries();
	nav_A.build();
	nav_mesh = &nav_A;
	keep_updating_navmesh = true;
	new std::thread(&CNav_mesh_manager::updateNavmesh, this);
	return true;
}

void CNav_mesh_manager::prepareInputNavMesh(){

	for (int i = 0; i < colMeshes.size(); ++i){
		colMeshes[i]->addInputNavMesh();
	}

	for (int i = 0; i < colBoxes.size(); ++i){
		colBoxes[i]->addInputNavMesh();
	}

	for (int i = 0; i < colSpheres.size(); ++i){
		colSpheres[i]->addInputNavMesh();
	}

	/*for (int i = 0; i < colCapsules.size(); ++i){
		colCapsules[i]->addInputNavMesh();
	}*/

	nav_A.m_input = nav_mesh_input;
	nav_A.m_input.computeBoundaries();
	nav_B.m_input = nav_mesh_input;
	nav_B.m_input.computeBoundaries();
}

bool CNav_mesh_manager::checkIfUpdatedNavMesh(){
	int i = 0;
	bool updatedChecked = false;
	while ((!updatedChecked) && (i < colBoxes.size())){
		updatedChecked = colBoxes[i]->getIfUpdated();
		i++;
	}
	
	if (!updatedChecked){
		i = 0;
		while ((!updatedChecked) && (i < colSpheres.size())){
			updatedChecked = colSpheres[i]->getIfUpdated();
			i++;
		}
	}

	if (updatedChecked)
		return true;
	else
		return false;
}

void CNav_mesh_manager::checkUpdates(){
	need_update = checkIfUpdatedNavMesh();
}

void CNav_mesh_manager::updateNavmesh() {
	while (keep_updating_navmesh) {
		bool lock = false;
		if (need_update){

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
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			nav_mesh_input.clearInput();
			prepareInputNavMesh();
		}
	}
}

void CNav_mesh_manager::render_nav_mesh(){
	if (keep_updating_navmesh){
		if (nav_mesh){
			const dtNavMesh* navmesh = nav_mesh->m_navMesh;
			if (navmesh){
				int totalTiles = navmesh->getMaxTiles();
				for (int i = 0; i < totalTiles; i++){
					render_tile(navmesh->getTile(i));
				}
			}
		}
	}
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

void CNav_mesh_manager::findPath(XMVECTOR pst_src, XMVECTOR pst_dst, std::vector<XMVECTOR> &straightPath){
	if (keep_updating_navmesh){
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
				straightPath.clear();
				XMFLOAT3* aux_array = new XMFLOAT3[navMeshQuery.numPointsStraightPath];
				for (int i = 0; i < navMeshQuery.numPointsStraightPath; ++i){
					XMVECTOR point = XMVectorSet(navMeshQuery.straightPath[i * 3], navMeshQuery.straightPath[i * 3 + 1] + 0.10f, navMeshQuery.straightPath[i * 3 + 2], 1);
					straightPath.push_back(point);
				}
			}
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

void CNav_mesh_manager::clearNavMesh(){
	keep_updating_navmesh = false;
	nav_mesh_input.clearInput();
}

XMVECTOR CNav_mesh_manager::getRandomNavMeshPoint(XMVECTOR center, float radius, XMVECTOR current_pos){
	CNavmeshQuery navMeshQuery(nav_mesh);
	return navMeshQuery.getRandomPoint(center, radius, current_pos);
}

bool CNav_mesh_manager::rayCastHit(XMVECTOR pos, XMVECTOR wanted_pos){
	bool hit = false;
	CNavmeshQuery navMeshQuery(nav_mesh);
	navMeshQuery.resetTools();
	navMeshQuery.updatePos(pos, wanted_pos);
	navMeshQuery.setTool(CNavmeshQuery::ETool::RAYCAST);
	return navMeshQuery.m_hitResult;
}

CNav_mesh_manager::CNav_mesh_manager()
{
}


CNav_mesh_manager::~CNav_mesh_manager()
{
}
