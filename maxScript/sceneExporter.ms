clearlistener()

fn exportXMLScene = (	
	local allObjects = $*
	local miString = "<scene>\n"
	for o in allObjects do(
		local mPath = getObjectPath o
		-- if have mesh to export then: not repeat, have mesh, etc...
 		exportMesh o
		----------------------------------
		miString += "<obj name=\"" + o.name + "\" path=\"" + mPath + "\" pos=\"" + o.pos as string + "\" rotation=\"" + o.rotation as string + "\"/>\n"	
		)
	miString += "</scene>\n"
	
	saveXMLArchive miString

	print miString	
	)

fn findIndexOfVertex vtxs v = (
	
	-- Busca en vtxs a ver si esta v
	local idx = 1
	for idx = 1 to vtxs.count do (
		-- Si esta, devolver el indice
		local j = 1
		while( j <= v.count ) do (
			if vtxs[ idx ][ j ] != v[ j ] then exit
			j = j + 1
		)
		if( j > v.count ) then return idx
	)
	
	-- Si no esta, añadirlo y devolver el indice (count)
	append vtxs v
	return vtxs.count
	
)

fn saveBinaryMesh obj vtxs idxs vertex_type = (
	local filename = getObjectPath obj
	
	--local prueba = "saving binary" +filename+ "."
	--print prueba
	
	if vtxs.count == 0 then throw ( "Invalid mesh to export\n" )
	
	-- Open file in binary mode, destroy existing version
	local fs = fopen filename "wb"
	if fs == undefined then throw ( "Can't create mesh output file " + filename )
	
	local mesh_magic = 0x44776655
	local version_number = 1
	local vtxs_magic = 0x44774444
	local idxs_magic = 0x44773333
	local end_magic = 0x44444444
	
	local bytes_per_index = 2
	local bytes_per_vertex = 4*vtxs[1].count
	
	-- Save a header
	WriteLong fs mesh_magic
	WriteLong fs version_number
	WriteLong fs vtxs.count
	WriteLong fs idxs.count
	WriteLong fs 4002					-- Primitive type. CMesh::ePrimitiveType=>TriangleList
	WriteLong fs bytes_per_vertex		-- # of bytes per vertex
	WriteLong fs bytes_per_index		-- # of bytes per index
	WriteLong fs vertex_type
	-- Write AABB ...
	
	
	WriteLong fs mesh_magic
	
	-- Save vtxs
	WriteLong fs vtxs_magic
	WriteLong fs (bytes_per_vertex*vtxs.count)
	for v in vtxs do (
		for c in v do (
			WriteFloat fs c
		)
	)
	
	-- Save indexs
	WriteLong fs idxs_magic
	WriteLong fs (bytes_per_index*idxs.count)
	for v in idxs do (
		WriteShort fs v #unsigned
	)
	
	-- Other chunks...
	
	-- Final chunk
	WriteLong fs end_magic
	WriteLong fs 0
	
	fclose fs
)

fn exportMesh obj_to_export = (
	-- Confirm obj is valid
	if obj_to_export == undefined then throw "export_mesh: Obj is not valid" 
	local obj = obj_to_export	
	if classof obj_to_export  != Editable_mesh then (
		obj = copy obj_to_export
		obj.name = obj_to_export.name
		convertToMesh obj
	)
	-- Collect all unique vertexs
	local all_vtxs = #()
	local all_idxs = #()
	local nfaces = getNumFaces obj
		
	local channelA = 1
	
	-- For each face
	local face_id
	for face_id = 1 to nfaces do (
		
		-- Position
		local face_pos = getFace obj face_id
		local face_mapA = meshop.getMapFace obj channelA face_id
		local face_normals = meshop.getFaceRNormals obj face_id
		
		--format "face_pos[%] = %\n" face_id face_pos
		
		-- For the 3 vtxs of each face
		local vid
		for vid in #(1,2,3) do (
			local pos = meshop.getVert obj face_pos[ vid ]
			local uvA = meshop.getMapVert obj channelA face_mapA[ vid ]
			local n = face_normals[ vid ]
			
			-- Collect all data associated to this vtx
			local v = #( pos.x, pos.y, pos.z, uvA.x, uvA.y, n.x, n.y, n.z ) -- , N.x, N.y, N.z )...
			
			-- Index the vtx or get the index if the vtx has already been defined
			local idx_of_v = findIndexOfVertex all_vtxs v
			append all_idxs idx_of_v
			
			--format "idx is %\n" idx_of_v
		)
		
	)
	
	-- Save in binary form to file
	print obj.name
	
	saveBinaryMesh obj all_vtxs all_idxs 1004		-- Positions+UV2 only
	
	-- Delete the temp mesh
	if obj != obj_to_export then delete obj
)

fn getObjectPath object_to_path = (
	local data_path = "C:\\Code\\MCV\\engine\\engine\\data\\"
	local filename = data_path + "meshes\\"
	filename += object_to_path.name + ".mesh"	
	return filename
	)
fn getXMLPath = (
	local data_path = "C:\\Code\\MCV\\engine\\engine\\data\\"
	local filename = data_path + "xml\\"
	filename += "scene.xml"	
	return filename
	)
	
fn saveXMLArchive mString = (
	local filename = getXMLPath()
	print "saving"
	print filename
	print "data"
	print mString
	local f = openFile filename mode:"wt"
        if f == undefined do
          f = createFile filename 
		if f == undefined then throw ( "Can't create xml output file " + filename )	
		format mString to: f
	close f
	)
	
fn openCloseFile = (
	local filename = getXMLPath()
	filename += "prueba"
	local fs = fopen filename "wb"
	if fs == undefined then throw ( "Can't open and close file " + filename )	
	fclose fs	
	)
	
exportXMLScene()
openCloseFile()