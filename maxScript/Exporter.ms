clearlistener()

-- Gets all the objects of the scene
allObject = $*

-- List of object not evaluables
NotEval = #(targetobject)

-- List of object without mesh
NotMesh = #(targetcamera, freecamera)

-- List of objects exported
expNames = #()


/*
addObjectsID()
exportXMLScene()
removeObjectsID()
*/
/**********************************************************************************************
Export the XML with entities and components
***********************************************************************************************/
fn exportXMLScene = (
	
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
		getMesh,
		getAABB,
		getCollider,
		getRigidbody
	)

	-- Initialize XML
	mXML = "<scene>\n"
	
	--Temporal
	mXML += "  <entity>\n"\
   + "<name name=\"Player\"/>\n"\
    +"<transform position=\"0 0 -5\" rotation=\"0 0 0 1\"  scale=\"1 1 1\"/>\n"\
    +"<aabb min=\"-0.5 0 -0.5\" max=\"0.5 2 0.5\"/>\n"\
    +"<mesh path=\"Prota\" color=\"1 1 1 1\"/>\n"\
    +"<playerDoomController movementVelocity=\"6\" rotationVelocity=\"90\"/>\n"\
	+"</entity>\n"
	
	mXML +=  "  <entity>\n"\
    +"<name name=\"Camera\"/>\n"\
    +"<transform position=\"0 2 -9\" rotation=\"0 0 0 1\"  scale=\"1 1 1\"/>\n"\
    +"<camera target=\"0 2 0\" fov=\"60\" znear=\"0.5\" zfar=\"1000\"/>\n"\
    +"<thirdPersonCameraController />\n"\
	+"</entity>"

	-- Calls every function in the function arrays
	for obj in allObject do (
		-- Check if needs to be evaluate
		if(validateEntity obj) then
		(		
			mXML += "\t<entity>\n"
			for func in functions do (
				mXML += "\t\t" + func obj
			)
			mXML += "\t</entity>\n"		
		)
	)
	
	mXML += "</scene>"
	
	-- print result XML
	print mXML
	saveXMLArchive mXML
)

fn validateEntity obj = (
	return ((findItem NotEval (classof obj)) == 0)
)
	
	
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

/**********************************************************************************************
Gets the object name
Params:
  - obj: target object
 Returns:
  - String with the component information
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
	local mString = "<mesh path=\"" + mPath + "\"/>\n"
	
	return mString
)	

fn validateMesh obj = (
	return ((findItem NotMesh (classof obj)) == 0)
)

fn validateExportedNames obj = (
	return ((findItem expNames obj.name) == 0)
)

/**********************************************************************************************
Gets the object name
Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getName obj = (
	local mString = "<name " + "name=\"" + obj.name + "_" + obj.id as string + "\" />\n"
	return mString
)

/**********************************************************************************************
Gets the object rigidbody
Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getRigidbody obj = (
	
	if ((validateCollider obj) == false) then
		return "";
	return "<rigidbody " + "density=\"" + (obj.modifiers[#PhysX_Rigid_Body].density as string) + "\" />\n"
	
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
	mString = "<collider staticFriction=\""  \
														+ obj.modifiers[#PhysX_Rigid_Body].StaticFriction as string + "\"" \
									 + " dynamicFriction=\"" \
														+ obj.modifiers[#PhysX_Rigid_Body].DynamicFriction as string + "\"" \
									 + " restitution=\"" \
														+ obj.modifiers[#PhysX_Rigid_Body].Bounciness as string + "\"" \
									 + " boxX=\"" \
														+ extents.x as string + "\"" \
									 + " boxY=\"" \
														+ extents.y as string + "\"" \
									 + " boxZ=\"" \
														+ extents.z as string + "\"" \													
			+ "/>\n"
	return mString
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
Gets the AABB min and max positions in the engine reference system
 Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getAABB obj = (
	
	if(validateMesh obj == false) then
	return ""
	
		-- Save the original transform and move the obj to the identity
		originalTransform = moveToIdentity obj
		
		local mMin = [obj.min.x, obj.min.z, obj.min.y]
		local mMax = [obj.max.x, obj.max.z, obj.max.y]
		
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

fn getTargetCamera obj = (
	
	if((validateCameras obj) == false) then
	return ""
	
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

fn validateCameras obj = (
		return (((classof obj) == targetcamera))
	)






/**********************************************************************************************
												HELPERS
**********************************************************************************************/

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
	local world2mcv = max2mcv
	
	obj.rotation = obj.rotation * rotateXMatrix -90
	obj.pos = obj.pos * world2mcv
	
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
/**********************************************************************************************
Calculate the path (relative/absolute) of the obj target
 Params:
  - obj: target object
  - isRelative: bool return relative path
 Returns:
  - String: path of the obj
***********************************************************************************************/
fn getObjectPath object_to_path isRelative = (
	
	local data_path = "C:\\Code\\engine\\engine\\data\\meshes\\"
	local relative_path = ""
	local filename = object_to_path.name
	local extension = ".mesh"
	
	if (isRelative) then
	return relative_path + filename
	
	return data_path + relative_path + filename + extension
	)

/**********************************************************************************************
Calculate the path  of the xml
 Returns:
  - String: path of the xml
***********************************************************************************************/
	fn getXMLPath = (
	local data_path = "C:\\Code\\engine\\engine\\"
	local filename = data_path + ""
	filename += "my_file.xml"	
	return filename
	)
	
/**********************************************************************************************
Create file xml and save it
 Params:
  - mString: String with the xml data
***********************************************************************************************/
fn saveXMLArchive mString = (
	local filename = getXMLPath()
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
	ABADIA ISLAND

***********************************************************************************************/	
	
fn saveBinaryMesh obj vtxs idxs vertex_type = (
	
	local filename = getObjectPath obj false
	
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
			local idx_of_v = findIndexOfVertex all_vtxs v
			append all_idxs idx_of_v
			
			--format "idx is %\n" idx_of_v
		)
		
	)
	
	-- Save in binary form to file
	saveBinaryMesh obj all_vtxs all_idxs 1004		-- Positions+UV2 only
	
	-- Delete the temp mesh
	if obj != obj_to_export then delete obj
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


fn max2mcvMatrix obj = (
	local max_world = obj.transform
	local max2mcv = rotatexmatrix -90
	local mcv2max = inverse max2mcv
	local mcv_world = mcv2max * max_world * max2mcv
	return mcv_world
)
	
	--fn max2mcvQuat
		/*
		- - +
	*/
	


-- Execution
addObjectsID()
exportXMLScene()
 --funcion que habre y cierra una última cosa para poder trabajar, a depurar
openCloseFile()
removeObjectsID()



