
clearListener()

fn findIndexOfVertex vtxs str_vtxs v = (

	-- Convert my array of floats to a single string
	local sv = with printAllElements off v as string
	
	-- Look for the string in the array
	local idx = findItem str_vtxs sv
	
	if idx > 0 then return idx
	
	-- Si no esta, añadirlo y devolver el indice (count)w
	append vtxs v
	append str_vtxs sv
	return vtxs.count
	
)

fn saveBinaryMesh obj vtxs idxs vertex_type = (
	local data_path = "D:\\T Usuarios\\T Daniel\\Documents\\universidad\\UPF Master\\proyecto\\GitHub\\engine\\engine\\data\\"
	local filename = data_path + "meshes\\"
	filename += obj.name + ".mesh"
	
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
		WriteShort fs (v-1) #unsigned		-- Pasar a base 0
	)
	
	-- Other chunks...
	
	-- Final chunk
	WriteLong fs end_magic
	WriteLong fs 0
	
	format "Exported mesh with % vtxs and % idxs\n" vtxs.count idxs.count
	
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
	local all_str_vtxs = #()			-- To keep the string version of the vertexs
	local all_idxs = #()
	local nfaces = getNumFaces obj
		
	local channelA = 1
	
	local world2local = inverse obj.transform
	local max2mcv = rotateXMatrix -90
	local world2mcv = world2local * max2mcv
	
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
		for vid in #(1,3,2) do (
			local wpos = meshop.getVert obj face_pos[ vid ]
			local uvA = meshop.getMapVert obj channelA face_mapA[ vid ]
			local wn = face_normals[ vid ]
			
			local mpos = wpos * world2mcv 
			local n = wn * world2mcv.rotationpart 
			
			-- Collect all data associated to this vtx
			local v = #( mpos.x, mpos.y, mpos.z, uvA.x, uvA.y, n.x, n.y, n.z ) -- , N.x, N.y, N.z )...
			
			-- Index the vtx or get the index if the vtx has already been defined
			local idx_of_v = findIndexOfVertex all_vtxs all_str_vtxs v
			append all_idxs idx_of_v
			
			--format "idx is %\n" idx_of_v
		)
		
	)
	
	-- Save in binary form to file
	saveBinaryMesh obj all_vtxs all_idxs 1004		-- Positions+UV2 only
	
	-- Delete the temp mesh
	if obj != obj_to_export then delete obj
)

exportMesh $