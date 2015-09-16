#include "mcv_platform.h"
#include "nav_mesh_manager.h"
#include "entity_manager.h"
#include "physics_manager.h"
#include "ai\aimanager.h"
#include "components\comp_recast_aabb.h"

static CNav_mesh_manager the_nav_mesh_manager;
static float max_distance_act_enemies = 30.f;
CNavmesh* nav_mesh;
CNavmesh 	nav_A;		// temporal 1
CNavmesh 	nav_B;		// temporal 2
std::mutex	generating_navmesh;	// mutex de control
static CNavmeshQuery navMeshQuery(nullptr);

CNav_mesh_manager& CNav_mesh_manager::get() {
	return the_nav_mesh_manager;
}

bool CNav_mesh_manager::build_nav_mesh(){
	navMeshQuery = nullptr;
	builded = false;
	first = true;
	player = CEntityManager::get().getByName("Player");
	if (need_navmesh){
		keep_updating_navmesh = true;
	}
	return true;
}

void CNav_mesh_manager::nav_mesh_init(){
	generate_nav_mesh = true;
	nav_A.m_input = nav_mesh_input;
	nav_A.m_input.computeBoundaries();
	nav_B.m_input = nav_mesh_input;
	nav_B.m_input.computeBoundaries();
	nav_A.build();
	nav_mesh = &nav_A;
	AiThread = new std::thread(&CNav_mesh_manager::updateNavmesh, this);
}

void CNav_mesh_manager::prepareInputNavMesh(){

	for (int i = 0; i < colMeshes.size(); ++i){
		((TCompColliderMesh*)colMeshes[i])->addInputNavMesh();
	}

	for (int i = 0; i < colBoxes.size(); ++i){
		((TCompColliderBox*)colBoxes[i])->addInputNavMesh();
	}

	for (int i = 0; i < colSpheres.size(); ++i){
		((TCompColliderSphere*)colSpheres[i])->addInputNavMesh();
	}

	for (int i = 0; i < colCapsules.size(); ++i){
		((TCompColliderCapsule*)colCapsules[i])->addInputNavMesh();
	}

	for (int i = 0; i < colMultiples.size(); ++i){
		((TCompColliderMultiple*)colMultiples[i])->addInputNavMesh();
	}

	for (int i = 0; i < colConvex.size(); ++i){
		((TCompColliderConvex*)colConvex[i])->addInputNavMesh();
	}

	nav_A.m_input = nav_mesh_input;
	nav_A.m_input.computeBoundaries();
	nav_B.m_input = nav_mesh_input;
	nav_B.m_input.computeBoundaries();
}

void CNav_mesh_manager::removeCapsule(CHandle cap){
	auto it = std::find(colCapsules.begin(), colCapsules.end(), cap);
	if (it != colCapsules.end())
		colCapsules.erase(it);
}

void CNav_mesh_manager::removeBox(CHandle cap){
	auto it = std::find(colBoxes.begin(), colBoxes.end(), cap);
	if (it != colBoxes.end())
		colBoxes.erase(it);
}

void CNav_mesh_manager::removeSphere(CHandle cap){
	auto it = std::find(colSpheres.begin(), colSpheres.end(), cap);
	if (it != colSpheres.end())
		colSpheres.erase(it);
}

void CNav_mesh_manager::removeMultiple(CHandle cap){
	auto it = std::find(colMultiples.begin(), colMultiples.end(), cap);
	if (it != colMultiples.end())
		colMultiples.erase(it);
}

void CNav_mesh_manager::removeConvex(CHandle cap){
	auto it = std::find(colConvex.begin(), colConvex.end(), cap);
	if (it != colConvex.end())
		colMultiples.erase(it);
}

bool CNav_mesh_manager::checkIfUpdatedNavMesh(){
	int i = 0;
	bool updatedChecked = false;
	while ((!updatedChecked) && (i < colBoxes.size())){
		updatedChecked = ((TCompColliderBox*)colBoxes[i])->getIfUpdated();
		i++;
	}
	
	if (!updatedChecked){
		i = 0;
		while ((!updatedChecked) && (i < colSpheres.size())){
			updatedChecked = ((TCompColliderSphere*)colSpheres[i])->getIfUpdated();
			i++;
		}
	}

	if (!updatedChecked){
		i = 0;
		while ((!updatedChecked) && (i < colCapsules.size())){
			updatedChecked = ((TCompColliderCapsule*)colCapsules[i])->getIfUpdated();
			i++;
		}
	}

	if (!updatedChecked){
		i = 0;
		while ((!updatedChecked) && (i < colMultiples.size())){
			updatedChecked = ((TCompColliderMultiple*)colMultiples[i])->getIfUpdated();
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
	while (generate_nav_mesh){
		if (keep_updating_navmesh) {
			if (need_update){

				// seleccionamos navmesh a actualizar (las actualizamso alternativamente)
				CNavmesh* updated_nav = nav_mesh == &nav_A ? &nav_B : &nav_A;

				// generamos la navmesh con los datos actualizados
				updated_nav->build();

				lock = true;

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

				lock = false;
				builded = true;

			}
		}else{
			lock = false;
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
		navMeshQuery.data=nav_mesh;
		if (nav_mesh){
			const dtNavMesh* navmesh = nav_mesh->m_navMesh;
			if (navmesh){
				navMeshQuery.resetTools();
				navMeshQuery.updatePos(pst_src, pst_dst);
				navMeshQuery.setTool(CNavmeshQuery::ETool::FIND_PATH);
				navMeshQuery.findStraightPath();
				num = navMeshQuery.numPointsStraightPath;
				polys = navMeshQuery.straightPath;
				straightPath.clear();
				XMFLOAT3* aux_array = new XMFLOAT3[navMeshQuery.numPointsStraightPath];
				for (int i = 0; i < navMeshQuery.numPointsStraightPath; ++i){
					XMVECTOR point = XMVectorSet(navMeshQuery.straightPath[i * 3], navMeshQuery.straightPath[i * 3 + 1] + 0.10f, navMeshQuery.straightPath[i * 3 + 2], 1);
					straightPath.push_back(point);
				}
				delete[] aux_array;
			}
		}
	}
}

void CNav_mesh_manager::pathRender(){
	if (polys) {
		for (int i = 0; i <num; ++i){
			if (i>0){
				XMVECTOR p1 = XMVectorSet(polys[(i - 1) * 3], polys[((i - 1) * 3) + 1] + 0.10f, polys[((i - 1) * 3) + 2], 1);
				XMVECTOR p2 = XMVectorSet(polys[i * 3], polys[i * 3 + 1] + 0.10f, polys[i * 3 + 2], 1);
				drawLine(p1, p2);
			}
		}
	}
}

void CNav_mesh_manager::clearNavMesh(){
	navMeshQuery = nullptr;
	recastAABBs.clear();
	colBoxes.clear();
	colSpheres.clear();
	colMeshes.clear();
	colConvex.clear();
	colCapsules.clear();
	colMultiples.clear();
	keep_updating_navmesh = false;
	need_navmesh = false;
	nav_mesh_input.clearInput();
	nav_mesh = nullptr;
	/*if (AiThread != nullptr){
		TerminateThread(AiThread, 0);
	}*/
}

XMVECTOR CNav_mesh_manager::getRandomNavMeshPoint(XMVECTOR center, float radius, XMVECTOR current_pos){

	navMeshQuery.data = nav_mesh;
	if (keep_updating_navmesh){
		if (nav_mesh){
			const dtNavMesh* navmesh = nav_mesh->m_navMesh;
			if (navmesh){
				return navMeshQuery.getRandomPoint(center, radius, current_pos);
			}
			else{
				XMVECTOR no_point = XMVectorSet(0.f, 0.f, 0.f, 0.f);
				return no_point;
			}
		}
		else{
			XMVECTOR no_point = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			return no_point;
		}
	}
	else{
		XMVECTOR no_point = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		return no_point;
	}
}

bool CNav_mesh_manager::rayCastHit(XMVECTOR pos, XMVECTOR wanted_pos){
	bool hit = false;
	navMeshQuery.data=nav_mesh;
	navMeshQuery.resetTools();
	navMeshQuery.updatePos(pos, wanted_pos);
	navMeshQuery.setTool(CNavmeshQuery::ETool::RAYCAST);
	return navMeshQuery.m_hitResult;
}

int CNav_mesh_manager::getLastRecastAABBIndex(){
	return recast_aabb_index;
}

void CNav_mesh_manager::registerRecastAABB(CHandle recastAABB){
	recastAABBs.push_back(recastAABB);
	recast_aabb_index++;
}

void CNav_mesh_manager::unregisterRecastAABB(CHandle recastAABB){
	auto it = std::find(recastAABBs.begin(), recastAABBs.end(), recastAABB);
	if (it != recastAABBs.end())
		recastAABBs.erase(it);
}

void CNav_mesh_manager::checkDistaceToEnemies(){
	if ((nav_mesh) && (need_navmesh)){
		for (int i = 0; i < recastAABBs.size(); ++i){
			TCompRecastAABB* aux_recast_aabb = (TCompRecastAABB*)recastAABBs[i];
			int ind = aux_recast_aabb->getIndex();
			AABB aabb_struct = *((TCompAABB*)aux_recast_aabb->m_aabb);
			if (player.isValid()){
				CHandle p_transform = ((CEntity*)player)->get<TCompTransform>();
				TCompTransform* player_transform = (TCompTransform*)p_transform;
				float distance = aabb_struct.sqrDistance(player_transform->position);
				if (distance < max_distance_act_enemies*max_distance_act_enemies) {
					if (!aux_recast_aabb->getActive()){
						aimanager::get().recastAABBActivate(ind);
						aux_recast_aabb->setActive(true);
					}
				}
				else{
					if (aux_recast_aabb->getActive()){
						aimanager::get().recastAABBDesactivate(ind);
						aux_recast_aabb->setActive(false);
					}
				}
			}
		}
	}
}

int CNav_mesh_manager::getIndexMyRecastAABB(CHandle my_aabb){
	for (int i = 0; i < recastAABBs.size(); ++i){
		TCompRecastAABB* aux_recast_aabb = (TCompRecastAABB*)recastAABBs[i];
		int ind = aux_recast_aabb->getIndex();
		AABB aabb_struct = *((TCompAABB*)aux_recast_aabb->m_aabb);
		TCompAABB* m_aabb = (TCompAABB*)my_aabb;
		if (aabb_struct.intersects(m_aabb)){
			return aux_recast_aabb->getIndex();
		}
	}
	return -1;
}

void CNav_mesh_manager::setNeedNavMesh(bool need){
	need_navmesh = need;
}

bool CNav_mesh_manager::getNeedNavMesh(){
	return need_navmesh;
}

CNav_mesh_manager::CNav_mesh_manager()
{
	recast_aabb_index = 0;
	need_navmesh=false;
	builded = false;
	AiThread = nullptr;
	lock = false;
}


CNav_mesh_manager::~CNav_mesh_manager()
{
}

bool CNav_mesh_manager::getLock(){
	return lock;
}
