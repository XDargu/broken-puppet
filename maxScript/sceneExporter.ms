clearlistener()
-- hacer: dar formato correcto a la exportación y sacar la generación de texto a una funcion
-- todo: añadir datos físicos básicos a la exportación
-- todo: añadir datos fisicos complejos, joints

NotEval = #(targetobject)
NotMesh = #(targetcamera, freecamera)

fn exportXMLScene = (	
	local allObjects = $*	
	local expNames = #()
	
	local miString = "<scene>\n"
	
	for obj in allObjects do(	
	
		-- first check if the  object needs to be evaluated
	if((findItem NotEval (classof obj)) == 0) then
	(		
		miString += "<entity>\n"
		
		-- Añadimos nombre
		miString += addName obj
		-- Añadimos transform
		miString += addTransform obj
		-- Añadimos Mesh
		miString += addMesh obj expNames	
		-- Añadimos Camera
		miString += addTargetCamera obj			
		
		miString += "</entity>\n"
		)
	)
	
	miString += "</scene>\n"
	
	saveXMLArchive miString

	print miString	
)


-- Creamos los datos ID
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

fn removeObjectsID = (
	local allObjects = $*
	for o in allObjects do(
			if (isProperty o.baseObject #id == true) then
			(
				print "borrando"
				custAttributes.delete o objectDataID
			)	
		)
)

-- Método para añadir las IDs a todos los elementos
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



-- Buscar qué componentes tiene
-- Que atributos necesita cada componente
-- Transform, name, mesh

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
	local filename = getObjectPath obj false
	
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

fn getObjectPath object_to_path isRelative = (
	local data_path = "C:\\Code\\MCV\\engine\\engine\\data\\meshes\\"
	local relative_path = ""
	local filename = object_to_path.name
	local extension = ".mesh"
	
	if (isRelative) then
	return relative_path + filename
	
	return data_path + relative_path + filename + extension
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
	
	
------------------------ Métodos de obtención de componentes ------------------------
	
fn addName obj = (
	-- Se comprueba si tiene name
	-- Se añaden los atributos necesarios
	local miString = "<name  name=\"" + obj.name + "_" + obj.id as string + "\" />\n"
	return miString
	)	
	
fn addTransform obj = (
	
	local miString = ""
	-- Se comprueba si tiene transform, (por defecto si)
	-- Se añaden los atributos necesarios
	if ((classof obj) != targetcamera) then
	(
			miString = "<transform position=\"" \
													+ obj.pos.x as string+" " \
													+ obj.pos.y as string+" " \
													+ obj.pos.z as string+" " \
								+ "\" rotation=\"" \
													+ obj.rotation.x as string+" "\
													+ obj.rotation.y as string+" "\
													+ obj.rotation.z as string+" "\
													+ obj.rotation.w as string+" "\
								+"\" scale=\""\
													+ obj.scale.x as string+ " "\
													+ obj.scale.y as string+ " "\
													+ obj.scale.z as string+ " "\
	+"\"/>\n"
	)
	else
	(
					miString = "<transform position=\"" \
													+ obj.pos.x as string+" " \
													+ obj.pos.y as string+" " \
													+ obj.pos.z as string+" " \
					+"\"/>\n"
	)
	return miString
	)
	
fn addMesh obj expNames = (

	miString = ""
	-- Se comprueba si tiene Mesh (por defecto si)
	if((findItem NotMesh (classof obj)) == 0) then
	(

	local mPath = getObjectPath obj true
	
	-- If mesh have been exported, not export
	if((findItem expNames obj.name) == 0) then(
		-- exportamos la mesh
		exportMesh obj				
		-- añadimos en nombre de la exportada a la lista de exportadas
		insertItem obj.name expNames 1
	)
	
	-- Se añaden los atributos necesarios
	local miString = "<mesh path=\"" + mPath + "\"/>\n"
	)
	return miString
)	
	
	
	
fn addTargetCamera obj = (
	local miString = ""
	-- Se comprueba si es de clase camara
	if((classof obj) == targetcamera) then (
	-- Se añaden los atributos necesarios
	
			miString = "<camera target=\"" \
													+ obj.target.pos.x as string+" " \
													+ obj.target.pos.y as string+" " \
													+ obj.target.pos.z as string+" " \
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
			miString = "<camera " \										
								+ " fov=\""   \ 
													+ obj.fov as string +"\""	\												
								+ " znear=\""   \ 
													+ obj.nearrange as string+"\""\
								+ " zfar=\""   \ 
													+ obj.farrange as string +""	\
	+"\"/>\n"
	)
-- fov target.pos znear zfar
	return miString
	)	
-------------------------------------------------------------------------------------------
	
addObjectsID()
exportXMLScene()

-- funcion que habre y cierra una última cosa para poder trabajar, a depurar	
openCloseFile()
removeObjectsID()