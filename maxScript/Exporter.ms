filein("GeneralConfig.ms")
clearlistener()

/********************** 

	Excepcion, 
	donde meter el comp int,
	ya que en el xml puede dar problemas de carga por el orden.

************************/

	/********************** CONFIG ************************/

	out_path = engine_path + "data\\"
	prefab_path = out_path + "prefabs\\"
	scene_path = out_path + "scenes\\"
	mesh_path = out_path + "meshes\\"
	tools_path = engine_path + "tools\\"
	collider_path = mesh_path + "colliders\\"
		
	-- Gets all the objects of the scene
	allObject = $*

	-- List of object not evaluables
	NotEval = #(targetobject, UConstraint, line)

	-- List of object without mesh
	NotMesh = #(targetcamera, freecamera, Omnilight, Skylight, Directionallight, grid, line, freeSpot)

	DistanceTextAttributes = "distanceTextData"
	CameraAttribute = "objectCameraData"
	CameraPivotAttribute = "objectCameraPivotData"
	PlayerPivotAttribute = "objectPlayerPivotData"
	PlayerAttribute = "objectPlayerData"
	TriggerAttribute = "TRIGGER"
	RoomAABBAttribute = "ROOM"
	NoEvalAttribute = "NOEVAL"
	ClueZoneAttribute = "mCluePoint"
	RecastAABBAttribute = "RECASTAABB"
	
	NotMeshAttributes = #(DistanceTextAttributes, CameraAttribute, CameraPivotAttribute, PlayerPivotAttribute, TriggerAttribute, RoomAABBAttribute, ClueZoneAttribute, RecastAABBAttribute)
	
	-- Logic attribute	
	logicAttribute = "Logic_element"

	-- List of objects exported
	expNames = #()

	-- List of objects exported
	expObjects = #()

	-- List of shapes exported
	expShapesNames = #()

	-- List of diffuses exported
	expDiff = #()
	
	-- List of diffuses exported
	expNormal = #()
	
	-- List of UConstrait exported
	expJoints = #()
	
	
	jointsToExp = #()

	LevelName = "Level"
	PlayerName = "Player"
	PlayerPivotName = "PlayerPivot"
	CameraPivotName = "CameraPivot"
	CameraName = "PlayerCamera"
	BasicEnemyName = "BasicEnemy"
	grandmaEnemyName = "EnemyGrandma"
	soldierEnemyName = "EnemySoldier"
	IntPullName = "int_pull"
	IntPushName = "int_push"
	
	WaterMaterial = "distorsion"
	GlassMaterial = "glass"


	-- List of special names
	reservedNames = #(PlayerName, PlayerPivotName, CameraPivotName, CameraName, LevelName)

	-- List of mandatory names
	mandatoryNames = #(PlayerName, PlayerPivotName, CameraPivotName, CameraName)
	
	-- List of no export materials
	noExportMaterials = #(WaterMaterial, GlassMaterial)

	-- List of enemys
	enemyNames = #(BasicEnemyName, grandmaEnemyName, soldierEnemyName)

	-- List of prefabs
	ex_prefabList = #()
	
	grandma_ragdoll_ind = 0
	soldier_ragdoll_ind = 0

	/********************** PREFAB ************************/

	struct objPrefab (
		obj_name
		,components = #()
	)

	struct objRender (
		obj_name
		,obj_sub_mesh = #()
		,mat_name = #()
	)

	/*
	- StaticBody

	- Player
	- PlayerPivot
	- CameraPivot
	- Camera

	*/

	/**********************************************************************************************
													HELPERS
	**********************************************************************************************/

	fn getRelativeTransform obj parent = (	
		return (obj.transform * (inverse parent.transform))
	)	
	
	/**********************************************************************************************
	Sets and object transfrom (position, rotation adn scale)
	 Params:
	  - obj: target object
	  - trans: array with position (as a vector), rotation (as a quaternion) and scale (as a vector)
	***********************************************************************************************/
	fn changeTransform obj trans = (
		
		obj.scale = trans[3]
		obj.rotation = trans[2]
		obj.pos = trans[1]	
	)
	

	/**********************************************************************************************
	Moves and object to the identity position, rotation and scale
	 Params:
	  - obj: target object
	 Returns:
	  - Array with the original position (as a vector), rotation (as a quaternion) and scale (as a vector)
	***********************************************************************************************/
	fn moveToIdentity obj = (
		
		local mTransform = #(obj.pos, obj.rotation, obj.scale)
		
		-- Colocar en identidad	
		obj.pos = [0, 0, 0]
		obj.rotation = (quat 0 0 0 1)
		obj.scale = [1, 1, 1]
		
		-- Devuelve transform original
		return mTransform
		
	)

	/**********************************************************************************************
	Moves an object from the max to the mvc reference system
	 Params:
	  - obj: target object
	***********************************************************************************************/
	fn world2mcv obj = (
		local world2local = inverse obj.transform
		local max2mcv = rotateXMatrix -90
		local _world2mcv = max2mcv
		
		obj.rotation = obj.rotation * rotateXMatrix -90
		obj.pos = obj.pos * _world2mcv
		
	)

	/**********************************************************************************************
	Moves an object from the mcv to the max reference system
	 Params:
	  - obj: target object
	***********************************************************************************************/
	fn mcv2world obj = (
		local local2world = inverse obj.transform
		local mcv2max = rotateXMatrix 90
		local mcv2world = mcv2max

		obj.rotation = obj.rotation * rotateXMatrix 90
		obj.pos = obj.pos * mcv2world
	)

	fn max2mcvQuat q = (
		return quat -q.x -q.y -q.z q.w
	)
		
	/**********************************************************************************************
	Create file xml and save it
	 Params:
	  - mString: String with the xml data
	***********************************************************************************************/
	fn saveXMLArchive mString = (
		local filename = scene_path + "my_file.xml"
		print "saving"
		print filename
		local f = openFile filename mode:"wt"
			if f == undefined do
			  f = createFile filename 
			if f == undefined then throw ( "Can't create xml output file " + filename )	
			format mString to: f
		close f
	)

	/**********************************************************************************************
	Calculate the path  of the xml
	 Returns:
	  - String: path of the xml
	***********************************************************************************************/
	fn getXMLPath = (
		local data_path = engine_path
		local filename = data_path + ""
		filename += "my_file.xml"	
		return filename
	)
		
	/**********************************************************************************************
		---TO DELETE---
	this method open and close a new file in the same directory that the xml,
		that becouse 3dsmax don´t close the last file opened.
	***********************************************************************************************/
	fn openCloseFile = (
		local filename = getXMLPath()
		filename += "prueba"
		local fs = fopen filename "wb"
		if fs == undefined then throw ( "Can't open and close file " + filename )	
		fclose fs	
	)

	/**********************************************************************************************
	Object ID structure
	 Params:
	  - ID: Unique object id
	***********************************************************************************************/
	objectDataID = attributes objectData
	(
		parameters main rollout:params
		(
			id type:#float ui:ID default:0
		)
		 
		rollout params "Object Custom ID"
		(
			spinner ID "ID" type: #float
		)
	)

	/**********************************************************************************************
	Set the ID of all the objects of the scene
	***********************************************************************************************/
	fn addObjectsID = (
		local allObjects = $*
		local contID = 0;
		for o in allObjects do
		(	
			contID +=1
			if (isProperty o.baseObject #id != true) then
			(
				custAttributes.add o objectDataID		
			)	
			o.id = contID
		)
	)

	/**********************************************************************************************
	Remove the ID of all the objects of the scene
	***********************************************************************************************/
	fn removeObjectsID = (
		local allObjects = $*
		for o in allObjects do(
				if (isProperty o.baseObject #id == true) then
				(
					custAttributes.delete o objectDataID
				)	
			)
	)

	/**********************************************************************************************
	Calculate the path (relative/absolute) of the obj target
	 Params:
	  - obj: target object
	  - isRelative: bool return relative path
	 Returns:
	  - String: path of the obj
	***********************************************************************************************/
	fn getObjectPath object_to_path isRelative = (
		
		local data_path = out_path  + "meshes\\"
		local relative_path = ""
		local filename = object_to_path.name
		local extension = ".mesh"
		
		if (isRelative) then
		return relative_path + filename
		
		return data_path + relative_path + filename + extension
		)

	fn max2mcvMatrix obj = (
		local max_world = obj.transform
		local max2mcv = rotatexmatrix -90
		local mcv2max = inverse max2mcv
		local mcv_world = mcv2max * max_world * max2mcv
		return mcv_world
	)

	/**********************************************************************************************
											COMPONENTES A SUCIO,  
												 SIN ATRIBUTOS 
												NI PULIMIENTOS
											PARA PRIMERA ITERACIÓN
	***********************************************************************************************/

	fn validateEnemy obj = (
		return ( (obj.name) == BasicEnemyName) or ( (obj.name) == grandmaEnemyName) or ( (obj.name) == soldierEnemyName)
	)


	/**********************************************************************************************
		ABADIA ISLAND 

	***********************************************************************************************/	

	fn saveBinaryMesh obj vtxs idxs groups vertex_type = (
		local data_path = out_path
		local filename = data_path + "meshes\\"
		filename += obj.name + ".mesh"
		print obj.name
		if vtxs.count == 0 then throw ( "Invalid mesh to export\n" )
		
		-- Open file in binary mode, destroy existing version
		local fs = fopen filename "wb"
		if fs == undefined then throw ( "Can't create mesh output file " + filename )
		
		local mesh_magic = 0x44776655
		local version_number = 2
		local vtxs_magic = 0x44774444
		local idxs_magic = 0x44773333
		local groups_magic = 0x44772222
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
		
		-- Other chunks
		WriteLong fs groups_magic
		WriteLong fs (groups.count*2*4)
		for g in groups do (
			WriteLong fs g[1]		-- First index (already in base0)
			WriteLong fs g[2]		-- Number of indices
		)
		
		-- Final chunk
		WriteLong fs end_magic
		WriteLong fs 0
		
		format "Exported mesh with % vtxs and % idxs\n" vtxs.count idxs.count
		
		fclose fs
	)
	
	fn computeTangentSpace vtxs idxs = (
	local vertexCount = vtxs.count
	local tan1 = #()
	local tan2 = #()

	-- default arrays to zero
	for i = 1 to vertexCount do (
		tan1[i] = [0,0,0]
		tan2[i] = [0,0,0]
	)

	local a = 1
	while( a < idxs.count ) do (
		-- Get the indices of each triangle in base 1
		local i1 = idxs[a]
		local i2 = idxs[a+1]
		local i3 = idxs[a+2]
		
		-- vtx-i is the original array of floats #(pos, uv, N ...)
		local vtx1 = vtxs[ i1 ]	
		local vtx2 = vtxs[ i2 ]
		local vtx3 = vtxs[ i3 ]
		
		-- The positions are the 3 first floats in each array
		local v1 = [vtx1[1], vtx1[2], vtx1[3] ]
		local v2 = [vtx2[1], vtx2[2], vtx2[3] ]
		local v3 = [vtx3[1], vtx3[2], vtx3[3] ]

		-- Then the tex coords
		local w1 = [vtx1[4], vtx1[5]]
		local w2 = [vtx2[4], vtx2[5]]
		local w3 = [vtx3[4], vtx3[5]]
		        
		local x1 = v2.x - v1.x
        local x2 = v3.x - v1.x
        local y1 = v2.y - v1.y
        local y2 = v3.y - v1.y
        local z1 = v2.z - v1.z
        local z2 = v3.z - v1.z
        
        local s1 = w2.x - w1.x
        local s2 = w3.x - w1.x
        local t1 = w2.y - w1.y
        local t2 = w3.y - w1.y
		
		local r = 1.0 / (s1 * t2 - s2 * t1)

		local sdir = [(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r]
        local tdir = [(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r]

		tan1[i1] = tan1[i1] + sdir
		tan1[i2] = tan1[i2] + sdir
		tan1[i3] = tan1[i3] + sdir
		                     
		tan2[i1] = tan2[i1] + tdir
		tan2[i2] = tan2[i2] + tdir
		tan2[i3] = tan2[i3] + tdir

		-- Next triangle
		a = a + 3
	)
	
	
	for a = 1 to vtxs.count do (
		local n = [ vtxs[a][6], vtxs[a][7], vtxs[a][8] ]
		local t = tan1[a]
		
        -- Gram-Schmidt orthogonalize
		local dot_nt = dot n t
        local tangent = normalize( t - n * dot_nt)
		
        -- Calculate handedness
		local cross_nt = cross n t
		local dot_cross_tan2 = dot cross_nt tan2[a]
        local tangent_w = 1
		if dot_cross_tan2 < 0 then (
			tangent_w = -1
		)
		
		-- Append the tangent + tanget_w to the information of each vertex
		join vtxs[a] #( tangent.x, tangent.y, tangent.z, tangent_w )
	)
)
		
	fn findIndexOfVertex vtxs str_vtxs v = (

		-- Convert my array of floats to a single string
		local sv = with printAllElements off v as string
		
		-- Look for the string in the array
		local idx = findItem str_vtxs sv
		
		if idx > 0 then return idx
		
		-- Si no esta, añadirlo y devolver el indice (count)
		append vtxs v
		append str_vtxs sv
		return vtxs.count
	)

	-- Returns a list of list of 
	fn getFacesGroups obj = (
		local obj_to_export
		if classof obj_to_export  != Editable_mesh then (
			obj_to_export = copy obj
			obj_to_export.name = obj.name
			convertToMesh obj_to_export
		)
		
		local nfaces = getNumFaces obj_to_export
		local f = #()
		local face_id
		for face_id = 1 to nfaces do (
			local mat_id = getFaceMatID obj_to_export face_id
			-- If no face has been assigned to this material_id
			if f[ mat_id ] == undefined then (
				f[ mat_id ] = #()
			)
			append f[ mat_id ] face_id
		)
		
		delete obj_to_export
		
		return f
	)
	
	fn stringHasChar the_string char = (
		for i = 1 to the_string.count do (
			if (the_string[i] == char) then (
				return true
			)
		)
		
		return false
	)
		
	fn importTexture infile = (
		local base_name = getFilenameFile infile
		local full_target = out_path + "textures/" + base_name + ".dds"
		
		if (stringHasChar infile " " == true) then (
			local msg = "La ruta de textura " + base_name + " contiene un espacio"
			throw msg
		)
		
		-- Convert to dds using texconv.exe por ejemplo
		--copyFile infile full_target
		local cmd = tools_path + "texconv.exe -f DXT5 " + infile + " -o " + out_path + "textures\\"
		format "Executing: --%--\n" cmd
		local log_file = out_path + "log\\" + base_name + ".log"
		local rc = DOSCommand (cmd)
		format "  rc = %\n" rc
		return base_name
	)
		
	fn importMaterial m = (
		if export_materials == false then return 0
		-- If is a material which should no be exported
		if (findItem noExportMaterials m.name) != 0 then return 0		
		-- if has no diffuse
		if (m.diffusemap == undefined) then return 0
		-- If has already been exported
		if (findItem expDiff m.diffusemap.filename) != 0 then return 0
		--if (findItem expNormal m.bumpmap.normal_map.filename) != 0 then return 0
		
		append expDiff m.diffusemap.filename
		--append expNormal m.bumpmap.normal_map.filename
		 
		if classof m != Standardmaterial then throw( "No se importar materiales que no sean std" )
		local diff_name = importTexture m.diffusemap.filename
		local normal_name = ""
		local emissive_name = ""
		local specular_name = ""
		local gloss_name = ""
		
		if (m.bumpmap != undefined) then (
			normal_name = importTexture m.bumpmap.normal_map.filename
		)
		
		if (m.selfIllumMap != undefined) then (
			emissive_name = importTexture m.selfIllumMap.filename
		)
		
		if (m.specularMap != undefined) then (
			specular_name = importTexture m.specularMap.filename
		)
		
		if (m.glossinessMap != undefined) then (
			gloss_name = importTexture m.glossinessMap.filename
		)
		
		--	importTexture m.normalmap.filename	
		-- Exportar la definicion del material a un xml
		local out_filename = out_path + "materials\\" + m.name + ".xml"
		local mfs = createfile out_filename
		if mfs == undefined then throw ("Can't create material filename" + out_filename )
		format "<std_material name=\"%\" diffuse=\"%\" normal=\"%\" specular=\"%\" glossiness=\"%\" emissive=\"%\"/>\n" m.name diff_name normal_name specular_name gloss_name emissive_name to:mfs
		print ("Exported material: " + (m.name as string))
		close mfs
	)
		
	fn getMaterialOfObject obj submesh = ( 
		
		if obj.material == undefined then( obj.material = Standardmaterial())
		local m = obj.material
		
		if classof m == Standardmaterial then 
			return m
		end
		if classof m == Multimaterial then 
			local sm = m[ submesh ]
			if classof sm == UndefinedClass then return Standardmaterial()
			if classof sm == Standardmaterial then return sm
			if classof sm == Multimaterial then throw( "El objeto " + obj.name + " submaterial " + (submesh as string) + " es a la vez un multimaterial!")
			throw ("No se exportar materiales de tipo " + ( classof(  sm )  as string ))
		end
		throw ("No se exportar materiales de tipo " + ( classof(  sm )  as string ))
		return m
	)
	
	fn FlattenMap obj = (
		max modify mode    
		convertto obj editable_mesh
		modPanel.setCurrentObject obj
		subObjectLevel = 4
		Unwrapper = Unwrap_UVW name: "MCVAutomap"
		Unwrapper.setApplyToWholeObject true
		addModifier obj Unwrapper
		Temp = obj.modifiers[ "MCVAutomap"]
		Temp.setMapChannel 3
		Temp.setTVSubObjectMode 3
		Temp.flattenMap 45.0  #() 0.02 true 0 true true     
		subObjectLevel = 0
	 
		convertto obj editable_mesh
		--deleteModifier obj Unwrapper
)
	
	fn BakeLightmap obj size =
	(
		FlattenMap obj
		--Clear all render elements 
		obj.iNodeBakeProperties.removeAllBakeElements() 	
		be2 = LightingMap() -- instance of the bake element class
		be2.outputSzX =be2.outputSzY = size --set the size of the baked map --specifythe full file path, name and type:
		local mName = getIdName obj
		local f_path = out_path + "textures\\lightmaps\\"
		be2.fileType = (f_path + mName + "_lighting.tga")
		be2.fileName = filenameFromPath be2.fileType
		be2.filterOn = true --enable filtering
		be2.shadowsOn =true --enable shadows
		be2.enabled = true --enable baking --Preparing theobjectfor baking:
		obj.INodeBakeProperties.addBakeElement be2 --add second element
		obj.INodeBakeProperties.bakeEnabled = true --enabling baking
		obj.INodeBakeProperties.bakeChannel = 3 --channel to bake
		obj.INodeBakeProperties.nDilations = 1 --expand the texturea bit
		select obj --we are baking the selection, so we select the object --Call the renderer to bake both elements:
		render rendertype:#bakeSelected vfb:off progressBar:true outputSize:[size,size]
		
		-- Convert to dds using texconv.exe por ejemplo
		--copyFile infile full_target
		local infile = (f_path + mName + "_lighting.tga");
		local cmd = tools_path + "texconv.exe -f DXT5 " + infile + " -o " + out_path + "textures\\lightmaps\\"
		format "Executing: --%--\n" cmd
		local rc = DOSCommand (cmd)
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
		local all_groups = #()
		local nfaces = getNumFaces obj
			
		local channelA = 1
		local channelL = 3
		if not (meshop.getMapSupport obj channelL) then (
			channelL = 1
		)
		
		local world2local = inverse obj.transform
		local max2mcv = rotateXMatrix -90
		local _world2mcv = world2local * max2mcv
		
		local face_groups = getFacesGroups obj

		-- For each group of faces (per material id)
		local group_of_faces
		for group_of_faces in face_groups do (

			-- Skip mat_id without faces
			if group_of_faces == undefined then continue
				
			-- This group starts at this index
			local first_index = all_idxs.count
			
			-- For each face in this material
			local face_id
			for face_id in group_of_faces do (
				
				-- Position
				local face_pos = getFace obj face_id
				local face_mapA = meshop.getMapFace obj channelA face_id
				local face_mapL = meshop.getMapFace obj channelL face_id
				local face_normals = meshop.getFaceRNormals obj face_id
				--format "face_pos[%] = %\n" face_id face_pos
				
				-- For the 3 vtxs of each face
				local vid
				for vid in #(1,3,2) do (
					local wpos = meshop.getVert obj face_pos[ vid ]
					local uvA = meshop.getMapVert obj channelA face_mapA[ vid ]
					local uvL = meshop.getMapVert obj channelL face_mapL[ vid ]
					local wn = face_normals[ vid ]
					if (wn == undefined) then ( wn = [1,0,0])
					
					local mpos = wpos * _world2mcv 
					local n = wn * _world2mcv.rotationpart 
					
					-- Collect all data associated to this vtx
					local v = #( mpos.x, mpos.y, mpos.z, uvA.x, 1-uvA.y, n.x, n.y, n.z, uvL.x, 1-uvL.y) -- , N.x, N.y, N.z )...
					
					-- Index the vtx or get the index if the vtx has already been defined
					local idx_of_v = findIndexOfVertex all_vtxs all_str_vtxs v
					append all_idxs idx_of_v
					
					--format "idx is %\n" idx_of_v
				)
			)
			
			-- First index & number of indices
			local g = #( first_index, group_of_faces.count * 3 )
			append all_groups g
		)
		
		computeTangentSpace all_vtxs all_idxs
		
		-- Save in binary form to file
		saveBinaryMesh obj all_vtxs all_idxs all_groups 1008		-- Positions+UV2 only
		
		-- Delete the temp mesh
		if obj != obj_to_export then delete obj
	)


	-- COLLIDER CONTROLLERS


	fn findIndexOfVertexShape vtxs str_vtxs v = (

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

	fn saveBinaryMeshShape obj vtxs idxs vertex_type = (
		local filename = collider_path
		filename += obj.name + "_collider.mesh"
		
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

	fn exportMeshShape obj_to_export = (
		
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
		local _world2mcv = world2local * max2mcv
		
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
				
				local mpos = wpos * _world2mcv 
				local n = wn * _world2mcv.rotationpart 
				
				-- Collect all data associated to this vtx
				local v = #( mpos.x, mpos.y, mpos.z, uvA.x, uvA.y, n.x, n.y, n.z ) -- , N.x, N.y, N.z )...
				
				-- Index the vtx or get the index if the vtx has already been defined
				local idx_of_v = findIndexOfVertexShape all_vtxs all_str_vtxs v
				append all_idxs idx_of_v
				
				--format "idx is %\n" idx_of_v
			)
			
		)
		
		-- Save in binary form to file
		saveBinaryMeshShape obj all_vtxs all_idxs 1004		-- Positions+UV2 only
		
		-- Delete the temp mesh
		if obj != obj_to_export then delete obj
	)

	/** NUEVO INICIO **/

	/**********************************************************************************************
												COMPONENT FUNCTIONS
	**********************************************************************************************/
	
	/**********************************************************************************************
	Gets the object transform
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getTransform obj = (
		
		-- Change the reference system of the object to mcv
		
		local mObj = max2mcvMatrix obj
		local qObj = mObj.rotationPart as quat
		qObj = quat -qObj.x -qObj.y -qObj.z qObj.w
		
		local mString = ""
		
		-- Check if the obj is a target camera (it needs special attributes)
		if ((classof obj) != targetcamera) then
		(
				mString = "<transform position=\"" \
														+ mObj.row4.x as string+" " \
														+ mObj.row4.y as string+" " \
														+ mObj.row4.z as string+"" \
									+ "\" rotation=\"" \
														+ qObj.x as string+" "\
														+ qObj.y as string+" "\
														+ qObj.z as string+" "\
														+ qObj.w as string+""\
									+"\" scale=\""\
														+ obj.scale.x as string+ " "\
														+ obj.scale.z as string+ " "\
														+ obj.scale.y as string+ ""\
			+"\"/>\n"
		)
		else
		(
						mString = "<transform position=\"" \
														+ mObj.row4.x as string+" " \
														+ mObj.row4.y as string+" " \
														+ mObj.row4.z as string+"" \
						+"\"/>\n"
		)
		
		return mString
	)

	fn validateMesh obj = (
		local validAttributes = true
		for atr in NotMeshAttributes do (
			if (isProperty obj.baseObject atr) then (
				validAttributes = false;
			)		
		)	
		return (((findItem NotMesh (classof obj)) == 0) and validAttributes and (classof obj == Editable_mesh))
	)

	fn validateExportedNames obj = (
		return ((findItem expNames obj.name) == 0)
	)

	/**********************************************************************************************
	 ---------------------------------------------------- DEPRECATED --------------------------------------------

	Gets the object name
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	 ---------------------------------------------------- DEPRECATED --------------------------------------------
	***********************************************************************************************/
	fn getMesh obj = (

		-- Se comprueba si tiene Mesh (por defecto si)
		if(validateMesh obj == false) then
			return "";

		local mPath = getObjectPath obj true
		
		--If mesh have been exported, not export
		if(validateExportedNames obj) then(
			exportMesh obj				
			-- add obj.name to the list of exported meshes
			insertItem obj.name expNames 1
		)
		
		-- Se añaden los atributos necesarios
		local mString = "<mesh color=\"1 1 1 1\" path=\"" + mPath + "\"/>\n"
		
		return mString
	)	


	fn validateIsNotVisible obj =(
	
		if (isProperty  obj.baseObject #collisionMesh) then (
			return obj.collisionMesh
		)
		
		return false
	) 

	fn validateCompRender obj = (
		if (ExportMeshes == false) then
		return false
		-- Si ha sido exportado, devolvemos la posición dentro del array de
		
		local validAttributes = true
		for atr in NotMeshAttributes do (
			if (isProperty obj.baseObject atr) then (
				validAttributes = false;
			)		
		)	
		-- The collision mesh has no render
		local isVisible = not(validateIsNotVisible obj)
		return ( ((classof obj) != UConstraint ) and ((findItem NotMesh (classof obj)) == 0) and validAttributes and isVisible)
	)

	fn validateObjExported obj = (
		local count = 1
		for o in expObjects do (
			if (o.obj_name == obj.name) then
			(
				return count
			)
			count +=1
			
		)
		return 0
	)

	/**********************************************************************************************
	Gets the object render component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getRender obj= (
		if(validateCompRender obj == false) then 
		return ""
		local mString = "<render>\n"
		
		-- Get the position of an array
		local arrayPos = validateObjExported obj
		
		-- Si no está en la lista, se crea un nuevo struct a rellenar y se ejecuta todo el proceso
		if(arrayPos == 0) then
		(
			local objRend = objRender()
					
			exportMesh obj
			local groups = getFacesGroups obj
			local sub_mesh = 0
			local mat_id = 1
					
			objRend.obj_name = obj.name
			
			for g in groups do (
				if g != undefined then (
					local m = getMaterialOfObject obj mat_id
					importMaterial m
					
					-- Add data to structRender
					append  objRend.obj_sub_mesh sub_mesh
					append  objRend.mat_name m.name
					
					mString += "<draw mesh=\"" + obj.name + "\" sub_mesh=\"" + sub_mesh as string +  "\" mat=\"" + m.name + "\"/>\n"
					sub_mesh = sub_mesh + 1
				)
				mat_id = mat_id + 1
			)
			
			append expObjects objRend 	
		)
		-- Si está en la lista, no se ejecuta, y se vuelve a crear el texto
		else
		(
			local objRend = expObjects[arrayPos]
			local count = 1
			for o in objRend.mat_name do (
				mString += "<draw mesh=\"" + objRend.obj_name + "\" sub_mesh=\"" + objRend.obj_sub_mesh[count] as string +  "\" mat=\"" + objRend.mat_name[count] + "\"/>\n"	
				count += 1
			)
		)
		mString += "</render>\n"
		
	)	
	fn validateLogicObj obj = (
		return (isProperty  obj.baseObject #Logic_element)
	)

	fn validateName obj = (
		-- If is a logic element or a reserved name
		return ((findItem reservedNames (obj.name)) != 0) or (validateLogicObj obj == true)
	)

	fn getIdName obj = (
		if (validateName obj) then (
			return  obj.name
		)
		return  obj.name + "_" + obj.id as string
	)
	
	/**********************************************************************************************
	Gets the object name
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getName obj = (		
		local mName = getIdName obj		
		return "<name " + "name=\"" + mName + "\" />\n"		
	)
	
	
	fn validatePushInt obj = (
		-- If is a logic element or a reserved name
		return (isProperty obj.baseobject #PushInt)
	)
	/**********************************************************************************************
	Gets the object Push Int
	Params:
	  - obj: target object
	***********************************************************************************************/	
	fn getPushInt obj = (		
		if(validatePushInt obj == false) then return ""
		return "<switchPushController />\n"
	)
	
	
	fn validatePullInt obj = (
		-- If is a logic element or a reserved name
		return (isProperty obj.baseobject #PullInt)
	)
	/**********************************************************************************************
	Gets the object Pull Int
	Params:
	  - obj: target object
	***********************************************************************************************/	
	fn getPullInt obj = (		
		if(validatePullInt obj == false) then return ""
		return "<switchPullController />\n"
	)


	fn validateRigidbody obj = (
		return ((obj.modifiers[#PhysX_Rigid_Body] != undefined) and (obj.modifiers[#PhysX_Rigid_Body].type == 1 or  obj.modifiers[#PhysX_Rigid_Body].type == 2))
	)
	
	fn validateRigidbodyParams obj = (
		return isProperty obj.baseObject #objectRigidBodyData == true
	)

	/**********************************************************************************************
	Gets the object rigidbody
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getRigidbody obj = (
		
		if ((validateRigidbody obj) == false) then
			return "";
		
		params = ""
		if (validateRigidbodyParams obj) then (
			if (obj.objectRigidBodyData.lockXPos) then
				params += "lockXPos=\"" + (obj.objectRigidBodyData.lockXPos as string) + "\" ";
			if (obj.objectRigidBodyData.lockYPos) then
				params += "lockYPos=\"" + (obj.objectRigidBodyData.lockYPos as string) + "\" ";
			if (obj.objectRigidBodyData.lockZPos) then
				params += "lockZPos=\"" + (obj.objectRigidBodyData.lockZPos as string) + "\" ";
			
			if (obj.objectRigidBodyData.lockXRot) then
				params += "lockXRot=\"" + (obj.objectRigidBodyData.lockXRot as string) + "\" ";
			if (obj.objectRigidBodyData.lockYRot) then
				params += "lockYRot=\"" + (obj.objectRigidBodyData.lockYRot as string) + "\" ";
			if (obj.objectRigidBodyData.lockZRot) then
				params += "lockZRot=\"" + (obj.objectRigidBodyData.lockZRot as string) + "\" ";
		)
		if (obj.modifiers[#PhysX_Rigid_Body].type == 2) then params += "kinematic=\"true\" "
		
		return "<rigidbody " + "density=\"" + ((obj.modifiers[#PhysX_Rigid_Body].density * 1000) as string) + "\" " + params + " />\n"
		
	)

	fn validateStaticbody obj = (
		return ((obj.modifiers[#PhysX_Rigid_Body] != undefined) and (obj.modifiers[#PhysX_Rigid_Body].type == 3))
	)

	/**********************************************************************************************
	Gets the object Staticbody
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getStaticbody obj = (
		
		if ((validateStaticbody obj) == false) then
			return "";
		return "<staticbody/>\n"
		
	)

	/**********************************************************************************************
	Checks if an object has a collider
	 Params:
	  - obj: target object
	 Returns:
	  - Validation boolean
	***********************************************************************************************/
	fn validateCollider obj = (
		return (obj.modifiers[#PhysX_Rigid_Body] != undefined);
	)


	/**********************************************************************************************
	Gets the Collider bounding box and material properties
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getCollider obj = (	
		
		if ((validateCollider obj) == false) then
			return "";
		
		-- Save the original transform and move the obj to the identity
		originalTransform = moveToIdentity obj
		
		-- Get the min and max boundingBox
		local mMin = [obj.min.x, obj.min.z, obj.min.y]
		local mMax = [obj.max.x, obj.max.z, obj.max.y]
		
		-- Get the extents in both direction
		local extents = (mMax - mMin) / 2;
		
		-- Moves the obj to the original transform
		changeTransform obj originalTransform
		
		-- Make the return string
		mString = ""
		
		if (obj.modifiers[#PhysX_Rigid_Body].meshType == 1) then (
			mString = "<colliderSphere staticFriction=\""  \
															+ obj.modifiers[#PhysX_Rigid_Body].StaticFriction as string + "\"" \
										 + " dynamicFriction=\"" \
															+ obj.modifiers[#PhysX_Rigid_Body].DynamicFriction as string + "\"" \
										 + " restitution=\"" \
															+ obj.modifiers[#PhysX_Rigid_Body].Bounciness as string + "\"" \
										 + " radius=\"" \
															+ obj.modifiers[#PhysX_Rigid_Body].meshRadius as string + "\"" \
				+ "/>\n"
		)
		
		if (obj.modifiers[#PhysX_Rigid_Body].meshType == 2) then (
		extents = [obj.modifiers[#PhysX_Rigid_Body].meshLength, obj.modifiers[#PhysX_Rigid_Body].meshWidth, obj.modifiers[#PhysX_Rigid_Body].meshHeight]
		extents = extents / 2
			
		mString = "<collider staticFriction=\""  \
															+ obj.modifiers[#PhysX_Rigid_Body].StaticFriction as string + "\"" \
										 + " dynamicFriction=\"" \
															+ obj.modifiers[#PhysX_Rigid_Body].DynamicFriction as string + "\"" \
										 + " restitution=\"" \
															+ obj.modifiers[#PhysX_Rigid_Body].Bounciness as string + "\"" \
										 + " boxX=\"" \
															+ extents.x as string + "\"" \
										 + " boxY=\"" \
															+ extents.z as string + "\"" \
										 + " boxZ=\"" \
															+ extents.y as string + "\"" \													
				+ "/>\n"
		)
		if ((obj.modifiers[#PhysX_Rigid_Body].meshType == 5) or (obj.modifiers[#PhysX_Rigid_Body].meshType == 6)) then (
			-- if not in expShapesNames export
			if (findItem expShapesNames obj.name) == 0 then (
				exportMeshShape obj
				append expShapesNames obj.name
			)
			mString = "<colliderMesh path=\"" + obj.name + "\" staticFriction=\"0.5\" dynamicFriction=\"0.8\" restitution=\"0.2\" />"
		)
		return mString
	)

	
	fn validateCompAABB obj = (
		
		 local is_collision = (isProperty obj.baseObject #collisionMesh == true)
			return (validateCompRender obj) or (isProperty obj.baseObject #TRIGGER == true) or (isProperty obj.baseObject #ROOM == true) or (isProperty obj.baseObject #mCluePoint == true) or (isProperty obj.baseObject #RECASTAABB == true) or is_collision
	)
	
	/**********************************************************************************************
	Gets the AABB min and max positions in the engine reference system
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getAABB obj = (
		
		if(validateCompAABB obj == false) then
		return ""
		
			-- Save the original transform and move the obj to the identity
			originalTransform = moveToIdentity obj
			
			local mMin = [obj.min.x, obj.min.z, -obj.max.y]
			local mMax = [obj.max.x, obj.max.z, -obj.min.y]
			
			-- Moves the obj to the original transform
			changeTransform obj originalTransform
			
			-- Make the return string
			local mString = "<aabb min=\"" \
												+ mMin.x as string + " "\
												+ mMin.y as string + " "\
												+ mMin.z as string + ""\
												+"\"" \					
										+" max=\"" \
												+ mMax.x as string + " "\
												+ mMax.y as string + " "\
												+ mMax.z as string + ""\
												+"\"" \	
										+ "/>\n"
		
		return mString
	)

	fn validateCameras obj = (
		return (((classof obj) == targetcamera)or((classof obj) == freecamera))
	)

	/**********************************************************************************************
	Gets a Camera component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getCamera obj = (

		if((validateCameras obj) == false) then
		return ""
		
		local mString = ""
		-- Se comprueba si es de clase camara
		if((classof obj) == targetcamera) then (
		-- Se añaden los atributos necesarios
		local mTarget = max2mcvMatrix obj.target
		
				mString = "<camera target=\"" \
														+ mTarget.row4.x as string+" " \
														+ mTarget.row4.y as string+" " \
														+ mTarget.row4.z as string+" " \
									+ "\" fov=\""   \ 
														+ obj.fov as string +"\""	\												
									+ " znear=\""   \ 
														+ obj.nearrange as string+"\""\
									+ " zfar=\""   \ 
														+ obj.farrange as string +""	\
		+"\"/>\n"
		)
		if((classof obj) == freecamera) then (
		-- Se añaden los atributos necesarios
				mString = "<camera " \										
									+ " fov=\""   \ 
														+ obj.fov as string +"\""	\												
									+ " znear=\""   \ 
														+ obj.nearrange as string+"\""\
									+ " zfar=\""   \ 
														+ obj.farrange as string +""	\
		+"\"/>\n"
		)
		
		-- fov target.pos znear zfar
		return mString
	)	
		
	fn validatePlayer obj = (
		if (isProperty obj.baseObject #objectPlayerData) == false then
		--messageBox ("El Player necesita los player data")
		return (((obj.name) == PlayerName)and(isProperty obj.baseObject #objectPlayerData) )
	)

	/**********************************************************************************************
	Gets a Player component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getPlayerController obj = (
		
		if((validatePlayer obj) == false) then
		return ""
		
		local mString = "<basicPlayerController " \
											+ "movementVelocity=\"" + obj.objectPlayerData.movementVelocity as string+  "\" " \
											+ "rotationVelocity=\"" + obj.objectPlayerData.rotationVelocity as string + "\" " \
											+ "/>\n"
		
		
		return mString
	)	


	/**********************************************************************************************
	Gets a Character component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getCharacterController obj = (
		
		local mString = ""
		
		if((validatePlayer obj) or (validateEnemy obj)) then
		(
			local mString = "<life life=\"100\" /><unityCharacterController " \
											+ "/>\n"
		)
		
		return mString
	)	

	fn validatePivotPlayer obj = (
			return (((obj.name) == PlayerPivotName)and(isProperty obj.baseObject #objectPlayerPivotData) )
	)

	/**********************************************************************************************
	Gets a Player Pivot component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getPlayerPivotController obj = (
		
		if((validatePivotPlayer obj) == false) then
		return ""
		
		local mString = "<playerPivotController " \
											+ "rotationVelocity=\"" + obj.objectPlayerPivotData.rotationVelocity as string + "\" " \
											+ "/>\n"
		return mString
	)	


	fn validateCameraPivot obj = (
			return (((obj.name) == CameraPivotName)and(isProperty obj.baseObject #objectCameraPivotData) )
	)

	/**********************************************************************************************
	Gets a Camera Pivot component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getCameraPivotController obj = (
		
		if((validateCameraPivot obj) == false) then
		return ""
		
		local mString = "<cameraPivotController " \
											+ "tiltVelocity=\"" + obj.objectCameraPivotData.tiltVelocity as string + "\" " \
											+ "/>\n"
		return mString
	)	


	fn validateCamera obj = (
			return (((obj.name) == CameraName)and(isProperty obj.baseObject #objectCameraData) )
	)

	/**********************************************************************************************
	Gets a Camera controller component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getCameraController obj = (
		
		if((validateCamera obj) == false) then
		return ""
		
		local mString = "<thirdPersonCameraController " \
											+ "offset=\"" \
											+ obj.objectCameraData.offsetX as string + " " \
											+ obj.objectCameraData.offsetY as string + " " \
											+ obj.objectCameraData.offsetZ as string + "\"" \
											+ "/>\n"
		return mString
	)	


	fn validatePointLight obj = (
			return ((classof obj) == Omnilight)
	)
	/**********************************************************************************************
	Gets a Point Light component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getPointLight obj = (
		
		if((validatePointLight obj) == false) then
		return ""
		
		local mString = "<pointLight " \
											+ "radius=\"" + obj.farAttenEnd as string + "\" " \
											+ "intensity=\"" + obj.multiplier as string + "\" " \
											+ "color=\"" \
														+ (obj.rgb.r/255) as string + " "\
														+ (obj.rgb.g/255) as string + " "\
														+ (obj.rgb.b/255) as string + " "\
														+ 1 as string + "\""\
											+ "/>\n"
		return mString
	)	

	fn validateAmbientLight obj = (
			return ((classof obj) == Skylight)
	)

	/**********************************************************************************************
	Gets a Ambient Light component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getAmbientLight obj = (
		
		if((validateAmbientLight obj) == false) then
		return ""
		
		local mString = "<ambientLight " \
											+ "intensity=\"" + obj.multiplier as string + "\" " \
											+ "color=\"" \
														+ (obj.rgb.r/255) as string + " "\
														+ (obj.rgb.g/255) as string + " "\
														+ (obj.rgb.b/255) as string + " "\
														+ 1 as string + "\""\
											+ "/>\n"
		return mString
	)	


	fn validateDirectionalLight obj = (
			return ((classof obj) == freeSpot)
	)

	/**********************************************************************************************
	Gets a Directional Light component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getDirectionalLight obj = (
		
		if((validateDirectionalLight obj) == false) then
		return ""
		
		local mString = "<shadows " \
											+ "intensity=\"" + obj.multiplier as string + "\" " \
											+ "color=\"" \
														+ (obj.rgb.r/255) as string + " "\
														+ (obj.rgb.g/255) as string + " "\
														+ (obj.rgb.b/255) as string + " "\
														+ 1 as string + "\""\
											+ " correction=\"true\"/>\n<camera fov=\"" + (obj.falloff ) as string + "\" znear=\"" + obj.nearAttenEnd as string + "\" zfar=\"50\" width=\"30\" height=\"30\" />\n"
		return mString
	)	
	

	fn validateJointFixed obj = (
	
	-- Get all the Uconstraints in the scene
	list = for o in $* where (classof o == UConstraint) collect o
	
	local varJointReturn = undefined
	-- check if the obj is an actor in a Uconstraint
	for mJoint in list do (
		
		--jointsToExp		
		local ownerTest = false
		
		if ((mJoint.constraintType == 0) == false) then continue
		local body0 = false
		local body1 = false
		local undefinedTest = false
		
		if( mJoint.body0 != undefined) then (
			body0 = mJoint.body0 == obj
		)
		else (
			undefinedTest = true
		)

		if( mJoint.body1 != undefined) then (
			body1 = mJoint.body1 == obj
		)
		else (
			undefinedTest = true
		)
		
			
		if (body0) or (body1) then (
			
			local mName = getIdName mJoint
			
			-- If the mJoint is not in the exp list add it and return false
			if (findItem expJoints mName ) == 0 then (
				-- Add it				
				append expJoints mName
				-- If have only one actor
				if (undefinedTest) then varJointReturn = mJoint
			)
			-- If there are two actor
			-- It´s the secont time it appears
			else (					
				if ((varJointReturn == undefined) and (findItem jointsToExp mName) == 0) then		
				varJointReturn = mJoint
			)
		)
	)	
	-- If didn´t exported it yet
	if (varJointReturn != undefined) then(
		append jointsToExp (getIdName varJointReturn)
	)
	
	return varJointReturn
)
	
/**********************************************************************************************
Gets a FixedJoint
 Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getJointFixed obj = (
	local mJoint = validateJointFixed obj
	if (mJoint == undefined) then return ""
	
	mString = "<jointFixed "
	
	local mObj = max2mcvMatrix mJoint
	local qObj = mObj.rotationPart as quat
	qObj = quat -qObj.x -qObj.y -qObj.z qObj.w
	
	-- Translations limits
	-- mode 1 = Locked, 2 = Limited, 3 = Free
	
	i = 1;
	-- Actors
	if (mJoint.body0 != undefined) then (		
		mString += "actor" + i as string + " = \"" +getIdName mJoint.body0 as string + "\" "
		i += 1;
	)
	
	if (mJoint.body1 != undefined) then 
	mString += "actor" + i as string + " = \"" + getIdName mJoint.body1 as string + "\" "
	
	mString += " jointPosition=\"" \
											+ mObj.row4.x as string+" " \
											+ mObj.row4.y as string+" " \
											+ mObj.row4.z as string+"" \
						+ "\" jointRotation=\"" \
											+ qObj.x as string+" "\
											+ qObj.y as string+" "\
											+ qObj.z as string+" "\
											+ qObj.w as string+"\" "
	
	
	-- Advanced
	mString += "parentChildCollision=\"" + mJoint.collision as string + "\" "
	mString += "breakable=\"" + mJoint.breakable as string + "\" "
	mString += "maxBreakForce=\"" + mJoint.maxForce as string + "\" "
	mString += "maxTorqueForce=\"" + mJoint.maxTorque as string + "\" "
	
	mString += "/>\n"	
	
	return mString
)


		
fn validateJointD6 obj = (
	
	-- Get all the Uconstraints in the scene
	list = for o in $* where (classof o == UConstraint) collect o
	
	-- check if the obj is an actor in a Uconstraint
	for mJoint in list do (
		
		if ((mJoint.constraintType == 6) == false) then continue
		local body0 = false
		local body1 = false
		local undefinedTest = false
		
		if( mJoint.body0 != undefined) then (
			body0 = mJoint.body0 == obj
		)
		else (
			undefinedTest = true
		)

		if( mJoint.body1 != undefined) then (
			body1 = mJoint.body1 == obj
		)
		else (
			undefinedTest = true
		)
		
			
		if (body0) or (body1) then (
			
			local mName = getIdName mJoint
			
			-- If the mJoint is not in the exp list add it and return false
			if (findItem expJoints mName ) == 0 then (
				-- Add it
				
				append expJoints mName
				-- If have only one actor
				if (undefinedTest) then return mJoint
				-- return false
				return undefined
			)
			-- If there are two actor
			else (
				return mJoint
			)
		)
	)	
	
	return undefined
)

/**********************************************************************************************
Gets a Distance Text component
 Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getJointD6 obj = (
	local mJoint = validateJointD6 obj
	if (mJoint == undefined) then return ""
	
	mString = "<jointD6 "
	
	local mObj = max2mcvMatrix mJoint
	local qObj = mObj.rotationPart as quat
	qObj = quat -qObj.x -qObj.y -qObj.z qObj.w
	
	-- Translations limits
	-- mode 1 = Locked, 2 = Limited, 3 = Free
	
	i = 1;
	-- Actors
	if (mJoint.body0 != undefined) then (		
		mString += "actor" + i as string + " = \"" + mJoint.body0.name + "_" + mJoint.body0.id as string + "\" "
		i += 1;
	)
	
	if (mJoint.body1 != undefined) then 
	mString += "actor" + i as string + " = \"" + mJoint.body1.name + "_" + mJoint.body1.id as string + "\" "
	
	mString += " jointPosition=\"" \
											+ mObj.row4.x as string+" " \
											+ mObj.row4.y as string+" " \
											+ mObj.row4.z as string+"" \
						+ "\" jointRotation=\"" \
											+ qObj.x as string+" "\
											+ qObj.y as string+" "\
											+ qObj.z as string+" "\
											+ qObj.w as string+"\" "
	
	-- Translations limits
	-- mode 1 = Locked, 2 = Limited, 3 = Free
	mString += "linearModeX=\"" + mJoint.linearModeY as string + "\" "
	mString += "linearModeY=\"" + mJoint.linearModeX as string + "\" "
	mString += "linearModeZ=\"" + mJoint.linearModeZ as string + "\" "
	
	mString += "linearPosition=\"" + mJoint.linearPosition as string + "\" "
	mString += "linearRestitution=\"" + mJoint.linearRestitution as string + "\" "
	
	-- Spring
	mString += "posSpring=\"" + mJoint.posSpring as string + "\" "
	mString += "posDamping=\"" +mJoint.posDamping as string + "\" "
	
	-- Modos de swing/twist:
	-- 1: Locked
	-- 2: Limited
	-- 3: Free
	
	/* Rotaciones:
	
	El primer swing es swing2
	El segundo swing es swing1
	
	 - Swing1 = Rotar en Y
	 - Swing2 = Rotar en X
	 - Twist = Rotar en Z
	 
	 Ya que de Max a DX se cambia de sistema:
	  - X en Max = X en DX
	  - Y en Max = -Z en DX
	  - Z en Max = Y  en DX
	  
	  Es ncesario cambiar los valores:
	   - Swing1 tendrá los valores de Twist
	   - Swing2 tendrá los valores de Swing2
	   - Twist tendrá los valores de Swing1 negados (ya que la Z de DX es la -Y de max)
	   
	*/
	-- Swing 1 (Swing Z en max)
	mString += "swing1Mode=\"" + mJoint.twistMode as string + "\" "
	mString += "swing1Angle=\"" + mJoint.twistAngleHigh as string + "\" "
	mString += "swing1Restitution=\"" + mJoint.twistRestitutionHigh as string + "\" "
	mString += "swing1Spring=\"" + mJoint.twistSpringHigh as string + "\" "
	mString += "swing1Damping=\"" + mJoint.twistDampingHigh as string + "\" "
	
	-- Swing 2 (Swing Y en max)
	mString += "swing2Mode=\"" + mJoint.swing1mode as string + "\" "
	mString += "swing2Angle=\"" + mJoint.swing1Angle as string + "\" "
	mString += "swing2Restitution=\"" + mJoint.swing1Restitution as string + "\" "
	mString += "swing2Spring=\"" + mJoint.swing1Spring as string + "\" "
	mString += "swing2Damping=\"" + mJoint.swing1Damping as string + "\" "
	
	-- Twist
	mString += "twistMode=\"" + mJoint.swing2Mode as string + "\" "
	mString += "twistAngle=\"" + mJoint.swing2Angle as string + "\" "
	mString += "twistRestitution=\"" + mJoint.swing2Restitution as string + "\" "
	mString += "twistSpring=\"" + mJoint.swing2Spring as string + "\" "
	mString += "twistDamping=\"" + mJoint.swing2Damping as string + "\" "
	
	-- Advanced
	mString += "parentChildCollision=\"" + mJoint.collision as string + "\" "
	mString += "breakable=\"" + mJoint.breakable as string + "\" "
	mString += "maxBreakForce=\"" + mJoint.maxForce as string + "\" "
	mString += "maxTorqueForce=\"" + mJoint.maxTorque as string + "\" "
	
	mString += "/>\n"	
	
	return mString
)

	
		
fn validateJointPrismatic obj = (
	
	-- Get all the Uconstraints in the scene
	list = for o in $* where (classof o == UConstraint) collect o
	
	-- check if the obj is an actor in a Uconstraint
	for mJoint in list do (
		
		if ((mJoint.constraintType == 2) == false) then continue
		local body0 = false
		local body1 = false
		local undefinedTest = false
		
		if( mJoint.body0 != undefined) then (
			body0 = mJoint.body0 == obj
		)
		else (
			undefinedTest = true
		)

		if( mJoint.body1 != undefined) then (
			body1 = mJoint.body1 == obj
		)
		else (
			undefinedTest = true
		)
		
			
		if (body0) or (body1) then (
			
			local mName = getIdName mJoint
			
			-- If the mJoint is not in the exp list add it and return false
			if (findItem expJoints mName ) == 0 then (
				-- Add it
				
				append expJoints mName
				-- If have only one actor
				if (undefinedTest) then return mJoint
				-- return false
				return undefined
			)
			-- If there are two actor
			else (
				return mJoint
			)
		)
	)	
	
	return undefined
)

/**********************************************************************************************
Gets a Distance Text component
 Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getJointPrismatic obj = (
	local mJoint = validateJointPrismatic obj
	if (mJoint == undefined) then return ""
		
	local mObj = max2mcvMatrix mJoint
	local qObj = mObj.rotationPart as quat
	qObj = quat -qObj.x -qObj.y -qObj.z qObj.w
	
	local hingePos = undefined
	
	mString = "<jointPrismatic "
	
	i = 1;
	-- Actors
	if (mJoint.body0 != undefined) then (		
		
		-- Calculate transform 
		local body0_rel_mcv = getRelativeTransform mJoint mJoint.body0
		local q_body0_rel_mcv = body0_rel_mcv.rotationPart as quat
		q_body0_rel_mcv = quat -q_body0_rel_mcv.x -q_body0_rel_mcv.y -q_body0_rel_mcv.z q_body0_rel_mcv.w
		
		mString += "actor" + i as string + " = \"" + getIdName mJoint.body0 as string + "\" "
		
				mString += " jointRelativePosition0=\"" \
												+ body0_rel_mcv.row4.x as string+" " \
												+ body0_rel_mcv.row4.y as string+" " \
												+ body0_rel_mcv.row4.z as string+"" \
						+ "\" jointRelativeRotation0=\"" \
											+ q_body0_rel_mcv.x as string+" "\
											+ q_body0_rel_mcv.y as string+" "\
											+ q_body0_rel_mcv.z as string+" "\
											+ q_body0_rel_mcv.w as string+"\" "
		
		i += 1;
	)
	
	if (mJoint.body1 != undefined) then (
		
		-- Calculate transform 
		local body1_rel_mcv = getRelativeTransform mJoint mJoint.body1
		local q_body1_rel_mcv = body1_rel_mcv.rotationPart as quat
		q_body1_rel_mcv = quat -q_body1_rel_mcv.x -q_body1_rel_mcv.y -q_body1_rel_mcv.z q_body1_rel_mcv.w
		mString += "actor" + i as string + " = \"" + getIdName mJoint.body1 as string + "\" "
		
		mString += " jointRelativePosition1=\"" \
										+ body1_rel_mcv.row4.x as string+" " \
										+ body1_rel_mcv.row4.y as string+" " \
										+ body1_rel_mcv.row4.z as string+"" \
				+ "\" jointRelativeRotation1=\"" \
									+ q_body1_rel_mcv.x as string+" "\
									+ q_body1_rel_mcv.y as string+" "\
									+ q_body1_rel_mcv.z as string+" "\
									+ q_body1_rel_mcv.w as string+"\" "
	)
	
	mString += " jointPosition=\"" \
										+ mObj.row4.x as string+" " \
										+ mObj.row4.y as string+" " \
										+ mObj.row4.z as string+"" \
					+ "\" jointRotation=\"" \
										+ qObj.x as string+" "\
										+ qObj.y as string+" "\
										+ qObj.z as string+" "\
										+ qObj.w as string+"\" "
	
	
	-- Advanced
	mString += "parentChildCollision=\"" + mJoint.collision as string + "\" "
	mString += "breakable=\"" + mJoint.breakable as string + "\" "
	mString += "maxBreakForce=\"" + mJoint.maxForce as string + "\" "
	mString += "maxTorqueForce=\"" + mJoint.maxTorque as string + "\" "

	-- Translations limits
	-- mode 1 = Locked, 2 = Limited, 3 = Free
	mString += "linearModeX=\"" + mJoint.linearModeX as string + "\" "
	mString += "linearModeY=\"" + mJoint.linearModeY as string + "\" "
	mString += "linearModeZ=\"" + mJoint.linearModeZ as string + "\" "
	
	mString += "linearPosition=\"" + mJoint.linearPosition as string + "\" "
	mString += "linearRestitution=\"" + mJoint.linearRestitution as string + "\" "
	
	-- Spring
	mString += "posSpring=\"" + mJoint.posSpring as string + "\" "
	mString += "posDamping=\"" +mJoint.posDamping as string + "\" "
	
	mString += "parentCollition=\"" +mJoint.Collision as string + "\" "
	
	
	mString += "/>\n"	
	
	return mString
)	

		
fn validateJointHinge obj = (
	
	-- Get all the Uconstraints in the scene
	list = for o in $* where (classof o == UConstraint) collect o
	
	-- check if the obj is an actor in a Uconstraint
	for mJoint in list do (
		
		if ((mJoint.constraintType == 4) == false) then continue
		local body0 = false
		local body1 = false
		local undefinedTest = false
		
		if( mJoint.body0 != undefined) then (
			body0 = mJoint.body0 == obj
		)
		else (
			undefinedTest = true
		)

		if( mJoint.body1 != undefined) then (
			body1 = mJoint.body1 == obj
		)
		else (
			undefinedTest = true
		)
		
			
		if (body0) or (body1) then (
			
			local mName = getIdName mJoint
			
			-- If the mJoint is not in the exp list add it and return false
			if (findItem expJoints mName ) == 0 then (
				-- Add it
				
				append expJoints mName
				-- If have only one actor
				if (undefinedTest) then return mJoint
				-- return false
				return undefined
			)
			-- If there are two actor
			else (
				return mJoint
			)
		)
	)	
	
	return undefined
)

/**********************************************************************************************
Gets a Hinge joint component
 Params:
  - obj: target object
 Returns:
  - Hinge joint data
***********************************************************************************************/
fn getJointHinge obj = (
		
	local mJoint = validateJointHinge obj
	if (mJoint == undefined) then return ""
		
	local mObj = max2mcvMatrix mJoint
	local qObj = mObj.rotationPart as quat
	qObj = quat -qObj.x -qObj.y -qObj.z qObj.w
	
	
	
	local hingePos = undefined
	
	mString = "<jointHinge "
	
	i = 1;
	-- Actors
	if (mJoint.body0 != undefined) then (		
		
		-- Calculate transform 
		local body0_rel_mcv = getRelativeTransform mJoint mJoint.body0
		local q_body0_rel_mcv = body0_rel_mcv.rotationPart as quat
		q_body0_rel_mcv = quat -q_body0_rel_mcv.x -q_body0_rel_mcv.y -q_body0_rel_mcv.z q_body0_rel_mcv.w
		
		mString += "actor" + i as string + " = \"" + getIdName mJoint.body0 as string + "\" "
		
				mString += " jointRelativePosition0=\"" \
												+ body0_rel_mcv.row4.x as string+" " \
												+ body0_rel_mcv.row4.y as string+" " \
												+ body0_rel_mcv.row4.z as string+"" \
						+ "\" jointRelativeRotation0=\"" \
											+ q_body0_rel_mcv.x as string+" "\
											+ q_body0_rel_mcv.y as string+" "\
											+ q_body0_rel_mcv.z as string+" "\
											+ q_body0_rel_mcv.w as string+"\" "
		
		i += 1;
	)
	
	if (mJoint.body1 != undefined) then (
		
		-- Calculate transform 
		local body1_rel_mcv = getRelativeTransform mJoint mJoint.body1
		local q_body1_rel_mcv = body1_rel_mcv.rotationPart as quat
		q_body1_rel_mcv = quat -q_body1_rel_mcv.x -q_body1_rel_mcv.y -q_body1_rel_mcv.z q_body1_rel_mcv.w
		mString += "actor" + i as string + " = \"" + getIdName mJoint.body1 as string + "\" "
		
		mString += " jointRelativePosition1=\"" \
										+ body1_rel_mcv.row4.x as string+" " \
										+ body1_rel_mcv.row4.y as string+" " \
										+ body1_rel_mcv.row4.z as string+"" \
				+ "\" jointRelativeRotation1=\"" \
									+ q_body1_rel_mcv.x as string+" "\
									+ q_body1_rel_mcv.y as string+" "\
									+ q_body1_rel_mcv.z as string+" "\
									+ q_body1_rel_mcv.w as string+"\" "
	)
	
	
	mString += " jointPosition=\"" \
										+ mObj.row4.x as string+" " \
										+ mObj.row4.y as string+" " \
										+ mObj.row4.z as string+"" \
					+ "\" jointRotation=\"" \
										+ qObj.x as string+" "\
										+ qObj.y as string+" "\
										+ qObj.z as string+" "\
										+ qObj.w as string+"\" "
	
	-- Translations limits
	-- mode 1 = Locked, 2 = Limited, 3 = Free
	
	mString += "linearPosition=\"" + mJoint.linearPosition as string + "\" "
	mString += "linearRestitution=\"" + mJoint.linearRestitution as string + "\" "
	
	mString += "swingAngle=\"" + mJoint.swing1Angle as string + "\" "
	
	-- Spring
	mString += "posSpring=\"" + mJoint.posSpring as string + "\" "
	mString += "posDamping=\"" +mJoint.posDamping as string + "\" "
	
	-- Advanced
	mString += "parentChildCollision=\"" + mJoint.collision as string + "\" "
	mString += "breakable=\"" + mJoint.breakable as string + "\" "
	mString += "maxBreakForce=\"" + mJoint.maxForce as string + "\" "
	mString += "maxTorqueForce=\"" + mJoint.maxTorque as string + "\" "
	
	mString += "/>\n"	
	
	return mString
)	
	
	


fn validatePlatformPath obj = (
	
	local mPath = undefined
	
	if(isProperty obj.baseObject #mPathName) then
	(
		mPath = getnodebyname obj.mPathName
	)
		
	return mPath
	
)

/**********************************************************************************************
Gets the component platform path, it contains a list of point that makes a path
Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/	
fn getPlatformPath obj = (	
	local mPath = validatePlatformPath obj
	if (mPath == undefined) then return ""
	
	local copyPath = copy mPath
	
	--local world2local = inverse copyPath.transform
	--local max2mcv = rotateXMatrix -90
	--local _world2mcv = world2local * max2mcv
	
	--copyPath.transform = _world2mcv
	
	local mString = "<platformPath>"
	
	for k = 1 to (numknots copyPath 1) do (				
		local mPoint = getKnotPoint copyPath 1 k
		
		mString += "<point index=\"" + k as string + "\"  position=\"" \
														+ mPoint.x as string+" " \
														+ mPoint.z as string+" " \
														+ (-mPoint.y) as string+"" \
		+"\"/>\n"		
	)
	
	mString += "</platformPath>"
	delete copyPath
	return mString
)


	/**********************************************************************************************
	Gets the Ragdoll component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getPlayerRagdoll obj = (		
		return "<ragdoll name=\"prota\"/>\n"	
	)


	fn  validateGrandmaRagdoll obj = (
		return  (obj.name == grandmaEnemyName)
	)
	
	/**********************************************************************************************
	Gets the Ragdoll component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getGrandmaRagdoll obj = (
			if (validateGrandmaRagdoll obj)==true then
			(
				grandma_ragdoll_ind = grandma_ragdoll_ind + 1;				
				return "<ragdoll name=\"abuela#" + grandma_ragdoll_ind as string	 + "\"/>\n"	
			)
				
		return ""
	)

	fn getSoldierRagdoll obj = (
		soldier_ragdoll_ind = soldier_ragdoll_ind + 1;				
		return "<ragdoll name=\"soldado#" + soldier_ragdoll_ind as string	 + "\"/>\n"	
	)	
	

	fn  validateTrigger obj = (
		return  (isProperty obj.baseObject TriggerAttribute)
	)
	
	/**********************************************************************************************
	Gets the trigger component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getTrigger obj = (
			if (validateTrigger obj) then
				return "<trigger/>\n"		
		return ""
	)
	
	fn  validateRecastAABB obj = (
		return  (isProperty obj.baseObject RecastAABBAttribute)
	)
	
	/**********************************************************************************************
	Gets the recastAABB component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getRecastAABB obj = (
			if (validateRecastAABB obj) then
				return "<recastAABB/>\n"		
		return ""
	)
	
	fn  validateRoomAABB obj = (
		return  (isProperty obj.baseObject RoomAABBAttribute)
	)
	
	/**********************************************************************************************
	Gets the room AABB component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getRoomAABB obj = (
			if ((validateRoomAABB obj) == false) then
				return ""		
		
		return "<zoneAABB/>\n"
	)
	
	fn  validateClueZone obj = (
		return  (isProperty obj.baseObject ClueZoneAttribute)
	)
	
	/**********************************************************************************************
	Gets the Clue Zone component
	Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/	
	fn getClueZone obj = (
			if ((validateClueZone obj) == false) then
				return ""
		
		local mString = "<logicGoldenNeedle "
			
		local clue_point = (getnodebyname obj.mCluePoint).pos
			
			
			
		mString += "cluePoint=\"" \
							+ clue_point.x as string+" " \
							+ clue_point.z as string+" " \
							+ (-clue_point.y) as string+"" \
							+ "\" "
			
		mString += "/>\n"
		return mString;
	)

	
	
	fn validateParticleSystem obj = (
		if (isProperty obj.baseObject #mSystemActive == true) then ( 
			if obj.mSystemActive == true then return true
		)			
		return false
	)

	/**********************************************************************************************
	Gets a Distance Text component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getParticleSystem obj = (
		
		if((validateParticleSystem obj) == false) then
		return ""
		
		local mString = "<particleGroup " \
											+ "name=\"" + obj.mSystemName as string + "\" " \
											+ "/>\n"
		return mString
	)	

	
	
	fn validateDistanceText obj = (
			return isProperty obj.baseObject #mString == true
	)

	/**********************************************************************************************
	Gets a Distance Text component
	 Params:
	  - obj: target object
	 Returns:
	  - String with the component information
	***********************************************************************************************/
	fn getDistanceText obj = (
		
		if((validateDistanceText obj) == false) then
		return ""
		
		local mString = "<distanceText " \
											+ "text=\"" + obj.mString as string + "\" " \
											+ "distance=\"" + obj.mDistance as string + "\" " \
											+ "size=\"" + obj.mSize as string + "\" " \
											+ "color=\"" + obj.mColor as string + "\" " \										
											+ "/>\n"
		return mString
	)	

	fn validateScene exporting = (
		valid = true
		
		-- Errors
		for obj in mandatoryNames do (
			if (getNodeByName obj == undefined) then (
				messageBox (" Falta el objeto: " + obj)
				valid = false
			)
		)
		
		-- Warnings
		if (lights.count == 0) then (
			if exporting then
			(
				if not(queryBox "AVISO: No hay luces en la escena, ¿seguro que quieres exportar?" beep:false) then (
					valid = false
				)
			)
			else
			(
				messageBox ("AVISO: No hay luces en la escena")
			)
		)
		
		return valid
	)

	fn validateEntity obj = (
		return (((findItem NotEval (classof obj)) == 0) and (not  (isProperty obj.baseObject NoEvalAttribute)))
	)
	
	fn validateEntityCollisionTag obj = (	
		if(obj != undefined)then (
			return (isProperty obj.baseobject #collisionTag)
		)
		return false
	)

	fn getEntityCollisionTag obj = (
		local collisionTag = ""
		
		if(validateEntityCollisionTag obj) then (
			collisionTag = "collisionTag = \"" + obj.collisionTag + "\""
		)
		else
		(		
			if(obj.name == PlayerName) then
			(
				collisionTag = "collisionTag = \"Player\""
			)
			if(validateEnemy obj) then				
			(
				collisionTag = "collisionTag = \"Enemy\""
			)
			if(validateRigidbody obj) then
			(
				collisionTag = "collisionTag = \"Actor\""
			)
			if(validateStaticbody obj) then
			(
				collisionTag = "collisionTag = \"Level\""
			)
		)

		return collisionTag
	)
	
	fn getEntityTag obj = (
		local tag = "";
		if(obj.name == PlayerName) then
		(
			tag = "tag = \"player\""
		)
		if(validateEnemy obj) then
		(
			tag = "tag = \"enemy\""
		)
		if(validateRigidbody obj) then
		(
			tag = "tag = \"actor\""
		)
		if(validateStaticbody obj) then
		(
			tag = "tag = \"level\""
		)	
		return tag
	)

	fn validatePrefabList obj = (
		for pref in ex_prefabList do (
			if (pref.obj_name == obj.name) then
			(
				return pref
			)
		)
		return false
	)

	/**********************************************************************************************
	Initialize the prefab values
	***********************************************************************************************/
	fn initializePrefabs = (
		-- Create Player prefab
		playerPrefab = objPrefab()
		playerPrefab.obj_name = PlayerName
		append playerPrefab.components getTransform
		append playerPrefab.components getPlayerController
		append playerPrefab.components getPlayerRagdoll

		-- Create PlayerPivot prefab
		playerPivotPrefab = objPrefab()
		playerPivotPrefab.obj_name = PlayerPivotName
		append playerPivotPrefab.components getTransform
		append playerPivotPrefab.components getPlayerPivotController

		-- Create CameraPivot prefab
		cameraPivotPrefab = objPrefab()
		cameraPivotPrefab.obj_name = CameraPivotName
		append cameraPivotPrefab.components getTransform
		append cameraPivotPrefab.components getCameraPivotController

		-- Create PlayerCamera prefab
		playerCameraPrefab = objPrefab()
		playerCameraPrefab.obj_name = CameraName
		append playerCameraPrefab.components getTransform
		append playerCameraPrefab.components getCameraController

		-- Create soldierEnemy prefab
		soldierEnemyPrefab = objPrefab()
		soldierEnemyPrefab.obj_name = soldierEnemyName
		append soldierEnemyPrefab.components getName
		append soldierEnemyPrefab.components getTransform
		append soldierEnemyPrefab.components getSoldierRagdoll		
		
		-- Create GrandmaEnemy prefab
		grandmaEnemyPrefab = objPrefab()
		grandmaEnemyPrefab.obj_name = grandmaEnemyName
		append grandmaEnemyPrefab.components getName
		append grandmaEnemyPrefab.components getTransform
		append grandmaEnemyPrefab.components getGrandmaRagdoll		
		
		-- Create IntPull prefab
		pullIntPrefab = objPrefab()
		pullIntPrefab.obj_name = IntPullName
		append pullIntPrefab.components getName
		append pullIntPrefab.components getTransform
		append pullIntPrefab.components getJointPrismatic

		-- Adding prefabs to the prefabs list
		ex_prefabList = #(playerPrefab, grandmaEnemyPrefab, soldierEnemyPrefab, playerPivotPrefab, cameraPivotPrefab, playerCameraPrefab, pullIntPrefab)
	)
	
	/**********************************************************************************************
	Returns a transform relative to a parent
	***********************************************************************************************/
	fn getRelativeTransform obj parent = (	
		return (obj.transform * (inverse parent.transform))
	)	
	
	/**********************************************************************************************
	Export one object with the viewer
	***********************************************************************************************/
	fn exportViewer progress:undefined = (
		local logicObjs = #()
		expNames = #()
		expObjects = #()
		expShapesNames = #()
		expDiff = #()
		expNormal = #()
		expJoints = #()
		
		initializePrefabs()
		
		/*if (validateScene true == false) then (
			return false;
		)*/
		
		-- Array with all the component functions
		/* Order:
		  - Name
		  - Transform
		  - Mesh
		  - AABB
		  - Collider
		  - Rigidbody
		  - StaticBody
		  - Camera
		  - PlayerController
		  - CameraController
		*/
		functions = #(
			getName, 
			getTransform,
			--getMesh,
			getRender,
			getAABB,			
			getPointLight,
			getAmbientLight,
			getDirectionalLight,
			getDistanceText	
		)

		-- Initialize XML
		mXML = "<scene><entity>"
		mXML += "<name name=\"the_light\"/>"
		mXML += "<transform position=\"0 7 30\" rotation=\"0.8 -0.59 0.05 0.12\" scale=\"1 1 1\"/>"
		mXML += "<shadows res=\"1024\" intensity=\"1.0\" color=\"1 1 1 1\"/>"
		mXML += "<camera fov=\"120.0\" znear=\"5\" zfar=\"2500\"/>"		
		mXML += "</entity>"
		mXML += "<entity>"
		mXML += "<name name=\"PlayerCamera\" />"
		mXML += "<transform position=\"-1.5 2.5 3.0\" rotation=\"-0.44 -0.83 0.13 -0.32\"/>"
		mXML += "<camera fov=\"75.0\" znear=\"0.1\" zfar=\"1000.0\"/>"
		mXML += "<viewerCameraController/>"
		mXML += "</entity>"

		if (progress != undefined) then
			progress.value = 0
		count = 1
		
		if ($ == undefined) then (
			throw "No hay nada seleccionado"
			return 0
		)
		
		local firstObj = #()
		for obj in $ do ( 
			if (findItem mandatoryNames obj.name == 0) then (
				append firstObj obj 
			)
		)
		
		
		local t_ref = firstObj[1].transform
		
		-- Calls every function in the function arrays
		for obj in firstObj do 
		(
			obj.transform = (obj.transform * (inverse t_ref))
			
			-- Check if needs to be evaluate
			if(validateEntity obj)then
			(
				-- Check if needs to check this after
				if (validateLogicObj obj == false) then
				(
					local mPrefab = validatePrefabList obj
					
					if (mPrefab == false) then 
					(
						local collisionTag = getEntityCollisionTag obj
						local tag = getEntityTag obj
						mXML += "\t<entity " + tag + " " + collisionTag +">\n"
						for func in functions do (
							mXML += "" + func obj
						)
						mXML += "\t</entity>\n"		
					)
					else
					(
						local tag = getEntityTag obj
						mXML += "\t<entity " + tag + " prefab=\"" + mPrefab.obj_name + "\">\n"
						for func in mPrefab.components do (
							mXML += "" + func obj
						)
						mXML += "\t</entity>\n"	
					)
				)
				else
				(
					-- add nuevas cosas					 
					append logicObjs obj
				)

			)
			
			if (progress != undefined) then
				progress.value = (100.0 / firstObj.count) * count
			count +=1
			
			obj.transform = (obj.transform * (t_ref))
		)
		for obj in logicObjs do 
		(
			local mPrefab = validatePrefabList obj
			
			if (mPrefab == false) then 
			(
				local collisionTag = getEntityCollisionTag obj
				local tag = getEntityTag obj
				mXML += "\t<entity " + tag + " " + collisionTag +">\n"
				for func in functions do (
					mXML += "" + func obj
				)
				mXML += "\t</entity>\n"		
			)
			else
			(
				local tag = getEntityTag obj
				mXML += "\t<entity " + tag + " prefab=\"" + mPrefab.obj_name + "\">\n"
				for func in mPrefab.components do (
					mXML += "" + func obj
				)
				mXML += "\t</entity>\n"	
			)
		
		)
		
		--mXML += "<entity><name name=\"condicion_victoria\" /> <trigger/> <transform position=\"41.028 19.27 -63.12\" rotation=\"0.0 0.0 0.0 1.0\" scale=\"1.0 1.0 1.0\"/>    <aabb min=\"-0.5 0.0 -0.866025\" max=\"1.0 0.1 0.866025\"/>    <victoryCondition/>  </entity>  <entity>    <name name=\"texto distancia\" />    <transform position=\"41.028 19.27 -63.12\" rotation=\"0.0 0.0 0.0 1.0\" scale=\"1.0 1.0 1.0\"/>    <distanceText text= \"YOU WIN\" distance=\"2\" size=\"100\" color=\"0xffff9999\"/>  </entity>  "
		
		mXML += "</scene>"
		
		-- print result XML
		--print mXML
		saveXMLArchive mXML
	)

	/**********************************************************************************************
	Export the XML with entities and components
	***********************************************************************************************/
	fn exportXMLScene progress:undefined = (
		local logicObjs = #()
		expNames = #()
		expObjects = #()
		expShapesNames = #()
		expDiff = #()
		expNormal = #()
		expJoints = #()
		jointsToExp = #()
		
		grandma_ragdoll_ind = 0
		soldier_ragdoll_ind = 0
		
		initializePrefabs()
		
		/*if (validateScene true == false) then (
			return false;
		)*/
		
		-- Array with all the component functions
		/* Order:
		  - Name
		  - Transform
		  - Mesh
		  - AABB
		  - Collider
		  - Rigidbody
		  - StaticBody
		  - Camera
		  - PlayerController
		  - CameraController
		*/
		functions = #(
			getName, 
			getTransform,
			--getMesh,
			getRender,
			getAABB,
			getCollider,
			getRigidbody,
			getStaticbody,
			getCamera,
			getCameraController,
			getCameraPivotController,		
			getPlayerPivotController,
			getCharacterController,
			getLevelMeshCollider,
			getPointLight,
			getAmbientLight,
			getDirectionalLight,
			getDistanceText,
			getJointPrismatic,
			getJointHinge,
			getJointD6,
			getJointFixed,
			getPlatformPath,
			getTrigger,
			getRoomAABB,
			getPullInt,
			getPushInt,
			getClueZone,
			getRecastAABB,
			getParticleSystem
		)

		-- Initialize XML
		mXML = "<scene>\n"
		
		if (progress != undefined) then
			progress.value = 0
		count = 1
		-- Calls every function in the function arrays
		for obj in allObject do 
			(
			-- Check if needs to be evaluate
			if(validateEntity obj)then
			(
				-- Check if needs to check this after
				if (validateLogicObj obj == false) then
				(
					local mPrefab = validatePrefabList obj
					
					if (mPrefab == false) then 
					(
						local collisionTag = getEntityCollisionTag obj
						local tag = getEntityTag obj
						mXML += "\t<entity " + tag + " " + collisionTag +">\n"
						for func in functions do (
							mXML += "" + func obj
						)
						mXML += "\t</entity>\n"		
					)
					else
					(
						local tag = getEntityTag obj
						mXML += "\t<entity " + tag + " prefab=\"" + mPrefab.obj_name + "\">\n"
						for func in mPrefab.components do (
							mXML += "" + func obj
						)
						mXML += "\t</entity>\n"	
					)
				)
				else
				(
					-- add nuevas cosas					 
					append logicObjs obj
				)

			)
			
			if (progress != undefined) then
				progress.value = (100.0 / allObject.count) * count
			count +=1
		)
		for obj in logicObjs do 
		(
			local mPrefab = validatePrefabList obj
			
			if (mPrefab == false) then 
			(
				local collisionTag = getEntityCollisionTag obj
				local tag = getEntityTag obj
				mXML += "\t<entity " + tag + " " + collisionTag +">\n"
				for func in functions do (
					mXML += "" + func obj
				)
				mXML += "\t</entity>\n"		
			)
			else
			(
				local tag = getEntityTag obj
				mXML += "\t<entity " + tag + " prefab=\"" + mPrefab.obj_name + "\">\n"
				for func in mPrefab.components do (
					mXML += "" + func obj
				)
				mXML += "\t</entity>\n"	
			)
		
		)
		
		--mXML += "<entity><name name=\"condicion_victoria\" /> <trigger/> <transform position=\"41.028 19.27 -63.12\" rotation=\"0.0 0.0 0.0 1.0\" scale=\"1.0 1.0 1.0\"/>    <aabb min=\"-0.5 0.0 -0.866025\" max=\"1.0 0.1 0.866025\"/>    <victoryCondition/>  </entity>  <entity>    <name name=\"texto distancia\" />    <transform position=\"41.028 19.27 -63.12\" rotation=\"0.0 0.0 0.0 1.0\" scale=\"1.0 1.0 1.0\"/>    <distanceText text= \"YOU WIN\" distance=\"2\" size=\"100\" color=\"0xffff9999\"/>  </entity>  "
		
		mXML += "</scene>"
		
		-- print result XML
		--print mXML
		saveXMLArchive mXML
	)


-- Execution
--addObjectsID()
--exportXMLScene()
--openCloseFile()
--removeObjectsID()