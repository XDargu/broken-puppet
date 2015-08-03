#include <stdio.h>
#include <stdlib.h>
#include "mcv_platform.h"
#include "navmesh.h"
#include "DetourNavMeshBuilder.h"
#include "RecastDump.h"
#include "DetourDebugDraw.h"
#include "render/render_utils.h"

CNavmesh::CNavmesh()
	: m_navMesh(nullptr)
{
	m_navQuery = dtAllocNavMeshQuery();
	m_ctx = &m_context;
}

void CNavmesh::build() {
	destroy();
	m_ctx->resetLog();

	rcConfig config;
	/*const float x_min = m_input.aabb_min.x;
	const float x_max = m_input.aabb_max.x;
	rcVcopy(config.bmin, &x_min);
	rcVcopy(config.bmax, &x_max);*/
	config.bmin[0] = m_input.aabb_min.x;
	config.bmin[1] = m_input.aabb_min.y;
	config.bmin[2] = m_input.aabb_min.z;
	config.bmax[0] = m_input.aabb_max.x;
	config.bmax[1] = m_input.aabb_max.y;
	config.bmax[2] = m_input.aabb_max.z;
	config.tileSize = 32;
	config.cs = 0.3f;
	config.ch = 0.1f;

	// TILES ---------------------------
	m_cellsize = config.cs;
	m_cellheight = config.ch;
	//----------------------------------

	config.walkableHeight = 2;
	config.walkableRadius = 2;
	config.walkableClimb = 1;
	config.walkableSlopeAngle = 45.0f;
	config.minRegionArea = 1;
	config.mergeRegionArea = 1;
	config.maxEdgeLen = 10;
	config.maxSimplificationError = 2;
	config.maxVertsPerPoly = 6;
	config.detailSampleDist = 1.0f;
	config.detailSampleMaxError = 0.1f;

	m_navMesh = create(config);
	if (m_navMesh)
		prepareQueries();

	dumpLog();
}

dtNavMesh* CNavmesh::create(const rcConfig& cfg) {
	// WARNING: We will admit animated meshes, but the first snapshot will be the used to generate the navmesh
	//assert( mesh->header.nsnapshots == 1 ); // must be a static mesh!

		dtNavMesh* m_nav = nullptr;

		bool m_keepInterResults = true;
		bool m_monotonePartitioning = false;

		// -------------------------------------------
		// Step 1. Initialize build config.
		// -------------------------------------------

		// Init build configuration from GUI
		memset(&m_cfg, 0, sizeof(m_cfg));
		m_cfg.cs = cfg.cs;
		m_cfg.ch = cfg.ch;
		m_cfg.walkableSlopeAngle = cfg.walkableSlopeAngle;
		m_cfg.walkableHeight = (int)ceilf(cfg.walkableHeight / m_cfg.ch);
		m_cfg.walkableClimb = (int)floorf(cfg.walkableClimb / m_cfg.ch * 0.3f);
		m_cfg.walkableRadius = (int)ceilf(cfg.walkableRadius / m_cfg.cs * 0.2f);
		m_cfg.maxEdgeLen = (int)(cfg.maxEdgeLen / cfg.cs);
		m_cfg.maxSimplificationError = cfg.maxSimplificationError;
		m_cfg.minRegionArea = (int)rcSqr(cfg.minRegionArea);		// Note: area = size*size
		m_cfg.mergeRegionArea = (int)rcSqr(cfg.mergeRegionArea);	// Note: area = size*size
		m_cfg.maxVertsPerPoly = (int)cfg.maxVertsPerPoly;
		m_cfg.detailSampleDist = cfg.detailSampleDist < 0.9f ? 0 : cfg.cs * cfg.detailSampleDist;
		m_cfg.detailSampleMaxError = cfg.ch * cfg.detailSampleMaxError;

		// Set the area where the navigation will be build.
		// Here the bounds of the input mesh are used, but the
		// area could be specified by an user defined box, etc.
		rcVcopy(m_cfg.bmin, cfg.bmin);
		rcVcopy(m_cfg.bmax, cfg.bmax);
		rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

		// Reset build times gathering.
		m_ctx->resetTimers();

		// Start the build process.	
		m_ctx->startTimer(RC_TIMER_TOTAL);

		m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
		m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
		m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", m_input.nverts / 1000.0f, m_input.ntris / 1000.0f);

		// -------------------------------------------
		// Step 2. Rasterize input polygon soup.
		// -------------------------------------------

		// Allocate voxel heightfield where we rasterize our input data to.
		m_solid = rcAllocHeightfield();
		if (!m_solid) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
			return nullptr;
		}
		if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch)) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
			return nullptr;
		}

		// Allocate array that can hold triangle area types.
		// If you have multiple meshes you need to process, allocate
		// and array which can hold the max number of triangles you need to process.
		m_triareas = new unsigned char[m_input.ntris_total];
		if (!m_triareas) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", m_input.ntris_total);
			return nullptr;
		}

		// Find triangles which are walkable based on their slope and rasterize them.
		// If your input data is multiple meshes, you can transform them here, calculate
		// the are type for each of the meshes and rasterize them.
		memset(m_triareas, 0, m_input.ntris*sizeof(unsigned char));

		for (size_t i = 0; i < m_input.inputs.size(); ++i) {
			m_input.prepareInput(m_input.inputs[i]);

			rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, m_input.verts, m_input.nverts, m_input.tris, m_input.ntris, m_triareas);
			rcRasterizeTriangles(m_ctx, m_input.verts, m_input.nverts, m_input.tris, m_triareas, m_input.ntris, *m_solid, m_cfg.walkableClimb);

			m_input.unprepareInput();
		}

		if (!m_keepInterResults) {
			delete[] m_triareas;
			m_triareas = 0;
		}

		// -------------------------------------------
		// Step 3. Filter walkables surfaces.
		// -------------------------------------------

		// Once all geoemtry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
		rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
		rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);


		// -------------------------------------------
		// Step 4. Partition walkable surface to simple regions.
		// -------------------------------------------

		// Compact the heightfield so that it is faster to handle from now on.
		// This will result more cache coherent data as well as the neighbours
		// between walkable cells will be calculated.
		m_chf = rcAllocCompactHeightfield();
		if (!m_chf) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
			return nullptr;
		}
		if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf)) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
			return nullptr;
		}

		if (!m_keepInterResults) {
			rcFreeHeightField(m_solid);
			m_solid = 0;
		}

		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf)) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
			return nullptr;
		}

		if (m_monotonePartitioning) {
			// Partition the walkable surface into simple regions without holes.
			// Monotone partitioning does not need distancefield.
			if (!rcBuildRegionsMonotone(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea)) {
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build regions.");
				return nullptr;
			}
		}
		else {
			// Prepare for region partitioning, by calculating distance field along the walkable surface.
			if (!rcBuildDistanceField(m_ctx, *m_chf)) {
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
				return nullptr;
			}

			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildRegions(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea)) {
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build regions.");
				return nullptr;
			}
		}

		// -------------------------------------------
		// Step 5. Trace and simplify region contours.
		// -------------------------------------------

		// Create contours.
		m_cset = rcAllocContourSet();
		if (!m_cset) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
			return nullptr;
		}
		if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset)) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
			return nullptr;
		}

		// -------------------------------------------
		// Step 6. Build polygons mesh from contours.
		// -------------------------------------------

		// Build polygon navmesh from the contours.
		m_pmesh = rcAllocPolyMesh();
		if (!m_pmesh) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
			return nullptr;
		}
		if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh)) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
			return nullptr;
		}

		// -------------------------------------------
		// Step 7. Create detail mesh which allows to access approximate height on each polygon.
		// -------------------------------------------

		m_dmesh = rcAllocPolyMeshDetail();
		if (!m_dmesh) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
			return nullptr;
		}

		if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh)) {
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
			return nullptr;
		}

		if (!m_keepInterResults) {
			rcFreeCompactHeightfield(m_chf);
			m_chf = 0;
			rcFreeContourSet(m_cset);
			m_cset = 0;
		}

		// At this point the navigation mesh data is ready, you can access it from m_pmesh.
		// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

		// -------------------------------------------
		// (Optional) Step 8. Create Detour data from Recast poly mesh.
		// -------------------------------------------

		// The GUI may allow more max points per polygon than Detour can handle.
		// Only build the detour navmesh if we do not exceed the limit.
		if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
			unsigned char* navData = 0;
			int navDataSize = 0;

			// Update poly flags from areas.
			for (int i = 0; i < m_pmesh->npolys; ++i) {
				if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
					m_pmesh->flags[i] = FLAG_WALK;
			}

			dtNavMeshCreateParams params;
			memset(&params, 0, sizeof(params));
			params.verts = m_pmesh->verts;
			params.vertCount = m_pmesh->nverts;
			params.polys = m_pmesh->polys;
			params.polyAreas = m_pmesh->areas;
			params.polyFlags = m_pmesh->flags;
			params.polyCount = m_pmesh->npolys;
			params.nvp = m_pmesh->nvp;
			params.detailMeshes = m_dmesh->meshes;
			params.detailVerts = m_dmesh->verts;
			params.detailVertsCount = m_dmesh->nverts;
			params.detailTris = m_dmesh->tris;
			params.detailTriCount = m_dmesh->ntris;
			params.walkableHeight = (float)cfg.walkableHeight;
			params.walkableRadius = (float)cfg.walkableRadius;
			params.walkableClimb = (float)cfg.walkableClimb;
			rcVcopy(params.bmin, m_pmesh->bmin);
			rcVcopy(params.bmax, m_pmesh->bmax);
			params.cs = m_cfg.cs;
			params.ch = m_cfg.ch;
			params.buildBvTree = true;

			if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
				m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
				return nullptr;
			}

			m_nav = dtAllocNavMesh();

			if (!m_nav) {
				dtFree(navData);
				m_ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
				return nullptr;
			}

			dtStatus status;

			status = m_nav->init(navData, navDataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status)) {
				dtFree(navData);
				m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
				return nullptr;
			}
		}

		m_ctx->stopTimer(RC_TIMER_TOTAL);

		// Show performance stats.
		duLogBuildTimes(*m_ctx, m_ctx->getAccumulatedTime(RC_TIMER_TOTAL));
		m_ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

		return m_nav;
}

void CNavmesh::prepareQueries() {
	dtStatus status = m_navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		printf("NAVMESH PREPARATION FAILED!\n");
}

void CNavmesh::destroy() {
	delete[] m_triareas;
	m_triareas = 0;
	rcFreeHeightField(m_solid);
	m_solid = 0;
	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;
	rcFreePolyMesh(m_pmesh);
	m_pmesh = 0;
	rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = 0;
	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
}

void CNavmesh::dumpLog() {
	printf("\n");
	for (int i = 0; i < m_ctx->getLogCount(); ++i)
		printf("%s\n", m_ctx->getLogText(i));
	printf("\n");
}

void CNavmesh::buildAllTiles()
{
	if (!m_navMesh) return;

	float* input_bmin = new float[3];
	input_bmin[0] = m_input.aabb_min.x;
	input_bmin[1] = m_input.aabb_min.y;
	input_bmin[2] = m_input.aabb_min.z;

	float* input_bmax = new float[3];
	input_bmax[0] = m_input.aabb_max.x;
	input_bmax[1] = m_input.aabb_max.y;
	input_bmax[2] = m_input.aabb_max.z;

	const float* bmin =input_bmin;
	const float* bmax = input_bmax;

	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, m_cellsize, &gw, &gh);
	const int ts = 32;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = m_tileSize*m_cellsize;


	// Start the build process.
	m_ctx->startTimer(RC_TIMER_TEMP);

	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			m_tileBmin[0] = bmin[0] + x*tcs;
			m_tileBmin[1] = bmin[1];
			m_tileBmin[2] = bmin[2] + y*tcs;

			m_tileBmax[0] = bmin[0] + (x + 1)*tcs;
			m_tileBmax[1] = bmax[1];
			m_tileBmax[2] = bmin[2] + (y + 1)*tcs;

			int dataSize = 0;
			unsigned char* data = buildTileMesh(x, y, m_tileBmin, m_tileBmax, dataSize);
			if (data)
			{
				// Remove any previous data (navmesh owns and deletes the data).
				m_navMesh->removeTile(m_navMesh->getTileRefAt(x, y, 0), 0, 0);
				// Let the navmesh own the data.
				dtStatus status = m_navMesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
				if (dtStatusFailed(status))
					dtFree(data);
			}
		}
	}

	// Start the build process.	
	m_ctx->stopTimer(RC_TIMER_TEMP);

	m_totalBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TEMP) / 1000.0f;
}

unsigned char* CNavmesh::buildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize)
{
	if (m_input.inputs.empty())
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
		return 0;
	}

	m_tileMemUsage = 0;
	m_tileBuildTime = 0;
	bool m_keepInterResults = true;


	//OJO!!!!!!!!!!!!!!!!!!!!!!
	//cleanup();
	destroy();

	const float* verts = m_input.verts;
	const int nverts = m_input.nverts;
	const int ntris = m_input.ntris;

	// Init build configuration from GUI
	//memset(&m_cfg, 0, sizeof(m_cfg));
	/*m_cfg.cs = m_cellSize;
	m_cfg.ch = m_cellHeight;
	m_cfg.walkableSlopeAngle = m_agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
	m_cfg.maxSimplificationError = m_edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_vertsPerPoly;*/
	m_cfg.tileSize = 32;
	m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
	m_cfg.width = m_cfg.tileSize + m_cfg.borderSize * 2;
	m_cfg.height = m_cfg.tileSize + m_cfg.borderSize * 2;
	//m_cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	//m_cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;

	// Expand the heighfield bounding box by border size to find the extents of geometry we need to build this tile.
	//
	// This is done in order to make sure that the navmesh tiles connect correctly at the borders,
	// and the obstacles close to the border work correctly with the dilation process.
	// No polygons (or contours) will be created on the border area.
	//
	// IMPORTANT!
	//
	//   :''''''''':
	//   : +-----+ :
	//   : |     | :
	//   : |     |<--- tile to build
	//   : |     | :  
	//   : +-----+ :<-- geometry needed
	//   :.........:
	//
	// You should use this bounding box to query your input geometry.
	//
	// For example if you build a navmesh for terrain, and want the navmesh tiles to match the terrain tile size
	// you will need to pass in data from neighbour terrain tiles too! In a simple case, just pass in all the 8 neighbours,
	// or use the bounding box below to only pass in a sliver of each of the 8 neighbours.
	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	m_cfg.bmin[0] -= m_cfg.borderSize*m_cfg.cs;
	m_cfg.bmin[2] -= m_cfg.borderSize*m_cfg.cs;
	m_cfg.bmax[0] += m_cfg.borderSize*m_cfg.cs;
	m_cfg.bmax[2] += m_cfg.borderSize*m_cfg.cs;

	// Reset build times gathering.
	m_ctx->resetTimers();

	// Start the build process.
	m_ctx->startTimer(RC_TIMER_TOTAL);

	m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
	m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return 0;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[m_input.ntris_total];
	if (!m_triareas) {
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", m_input.ntris_total);
		return nullptr;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, m_input.ntris*sizeof(unsigned char));

	for (size_t i = 0; i < m_input.inputs.size(); ++i) {
		m_input.prepareInput(m_input.inputs[i]);

		rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, m_input.verts, m_input.nverts, m_input.tris, m_input.ntris, m_triareas);
		rcRasterizeTriangles(m_ctx, m_input.verts, m_input.nverts, m_input.tris, m_triareas, m_input.ntris, *m_solid, m_cfg.walkableClimb);

		m_input.unprepareInput();
	}

	if (!m_keepInterResults) {
		delete[] m_triareas;
		m_triareas = 0;
	}

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return 0;
	}

	if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return 0;
	}

	// Create contours.
	/*m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return 0;
	}
	if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return 0;
	}

	if (m_cset->nconts == 0)
	{
		return 0;
	}*/

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return 0;
	}
	if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return 0;
	}

	// Build detail mesh.
	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'dmesh'.");
		return 0;
	}

	if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf,
		m_cfg.detailSampleDist, m_cfg.detailSampleMaxError,
		*m_dmesh))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could build polymesh detail.");
		return 0;
	}

	if (!m_keepInterResults)
	{
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
	}

	unsigned char* navData = 0;
	int navDataSize = 0;
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		if (m_pmesh->nverts >= 0xffff)
		{
			// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
			m_ctx->log(RC_LOG_ERROR, "Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
			return 0;
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		/*params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		params.offMeshConCount = m_geom->getOffMeshConnectionCount();*/
		params.walkableHeight = 2.f;
		params.walkableRadius = 1.f;
		params.walkableClimb = 1.f;
		params.tileX = tx;
		params.tileY = ty;
		params.tileLayer = 0;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return 0;
		}
	}
	m_tileMemUsage = navDataSize / 1024.0f;

	m_ctx->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	duLogBuildTimes(*m_ctx, m_ctx->getAccumulatedTime(RC_TIMER_TOTAL));
	m_ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

	m_tileBuildTime = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

	dataSize = navDataSize;
	return navData;
}
