clearlistener()
(
/********************** CONFIG ************************/
ExportMeshes = true
export_meshes = true
export_materials = true

	engine_path = "D:\\Users\\Daniel\\Documents\\BrokenPuppet\\engine\\engine\\"
	out_path = engine_path + "data\\"
	prefab_path = out_path + "prefabs\\"
	scene_path = out_path + "scenes\\"
	mesh_path = out_path + "meshes\\"
	tools_path = engine_path + "tools\\"
	collider_path = mesh_path + "colliders\\"
	
-- Gets all the objects of the scene
allObject = $*

-- List of object not evaluables
NotEval = #(targetobject)

-- List of object without mesh
NotMesh = #(targetcamera, freecamera, Omnilight, Skylight, Directionallight,grid)

DistanceTextAttributes = "distanceTextData"
CameraAttribute = "objectCameraData"
CameraPivotAttribute = "objectCameraPivotData"
PlayerPivotAttribute = "objectPlayerPivotData"
PlayerAttribute = "objectPlayerData"

NotMeshAttributes = #(DistanceTextAttributes, CameraAttribute, CameraPivotAttribute, PlayerPivotAttribute)

-- List of objects exported
expNames = #()

-- List of objects exported
expObjects = #()

-- List of shapes exported
expShapesNames = #()

-- List of diffuses exported
expDiff = #()

LevelName = "Level"
PlayerName = "Player"
PlayerPivotName = "PlayerPivot"
CameraPivotName = "CameraPivot"
CameraName = "PlayerCamera"
BasicEnemyName = "BasicEnemy"


-- List of special names
reservedNames = #(PlayerName, PlayerPivotName, CameraPivotName, CameraName, LevelName)

-- List of mandatory names
mandatoryNames = #(PlayerName, PlayerPivotName, CameraPivotName, CameraName)

-- List of enemys
enemyNames = #(BasicEnemyName)


/********************** PREFAB ************************/

struct objPrefab (
	obj_name
	,components = #()
)

-- Create Player prefab
playerPrefab = objPrefab()
playerPrefab.obj_name = PlayerName
append playerPrefab.components getTransform
append playerPrefab.components getPlayerController

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

-- Create BasicEnemy prefab
basicEnemyPrefab = objPrefab()
basicEnemyPrefab.obj_name = BasicEnemyName
append basicEnemyPrefab.components getTransform
append basicEnemyPrefab.components getAiFsmBasic

-- Adding prefabs to the prefabs list
prefabList = #(playerPrefab, basicEnemyPrefab, playerPivotPrefab, cameraPivotPrefab, playerCameraPrefab)



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
Export the XML with entities and components
***********************************************************************************************/
fn exportXMLScene progress:undefined = (
	
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
		--getPlayerController,
		--getAiFsmBasic,
		getLevelMeshCollider,
		getPointLight,
		getAmbientLight,
		getDirectionalLight,
		getDistanceText
	)

	-- Initialize XML
	mXML = "<scene>\n"
	
	if (progress != undefined) then
		progress.value = 0
	count = 1
	-- Calls every function in the function arrays
	for obj in allObject do (
		-- Check if needs to be evaluate
		if(validateEntity obj) then
		(		
			local mPrefab = validatePrefabList obj
			
			if (mPrefab == false) then 
			(
				local tag = getEntityTag obj
				mXML += "\t<entity " + tag + ">\n"
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
		
		if (progress != undefined) then
			progress.value = (100.0 / allObject.count) * count
		count +=1
	)
	
	--mXML += "<entity><name name=\"condicion_victoria\" /> <trigger/> <transform position=\"41.028 19.27 -63.12\" rotation=\"0.0 0.0 0.0 1.0\" scale=\"1.0 1.0 1.0\"/>    <aabb min=\"-0.5 0.0 -0.866025\" max=\"1.0 0.1 0.866025\"/>    <victoryCondition/>  </entity>  <entity>    <name name=\"texto distancia\" />    <transform position=\"41.028 19.27 -63.12\" rotation=\"0.0 0.0 0.0 1.0\" scale=\"1.0 1.0 1.0\"/>    <distanceText text= \"YOU WIN\" distance=\"2\" size=\"100\" color=\"0xffff9999\"/>  </entity>  "
	
	mXML += "</scene>"
	
	-- print result XML
	--print mXML
	saveXMLArchive mXML
)

fn validatePrefabList obj = (
	for pref in prefabList do (
		if (pref.obj_name == obj.name) then
		(
			return pref
		)
	)
	return false
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

fn validateEntity obj = (
	return ((findItem NotEval (classof obj)) == 0)
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

fn validateMesh obj = (
	local validAttributes = true
	for atr in NotMeshAttributes do (
		if (isProperty obj.baseObject atr) then (
			validAttributes = false;
		)		
	)	
	return (((findItem NotMesh (classof obj)) == 0) and validAttributes)
)

fn validateExportedNames obj = (
	return ((findItem expNames obj.name) == 0)
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
	return (((findItem NotMesh (classof obj)) == 0) and validAttributes)
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
Gets the object name
Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getName obj = (
	if (validateName obj) then
	return "<name " + "name=\"" + obj.name + "\" />\n"
	
	return "<name " + "name=\"" + obj.name + "_" + obj.id as string + "\" />\n"
)

fn validateName obj = (
	return ((findItem reservedNames (obj.name)) != 0)
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
	
	return "<rigidbody " + "density=\"" + ((obj.modifiers[#PhysX_Rigid_Body].density * 1000) as string) + "\" " + params + " />\n"
	
)

fn validateRigidbody obj = (
	return ((obj.modifiers[#PhysX_Rigid_Body] != undefined) and (obj.modifiers[#PhysX_Rigid_Body].type == 1))
)

fn validateRigidbodyParams obj = (
	return isProperty obj.baseObject #objectRigidBodyData == true
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

fn validateStaticbody obj = (
	return ((obj.modifiers[#PhysX_Rigid_Body] != undefined) and (obj.modifiers[#PhysX_Rigid_Body].type == 3))
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
	if (obj.modifiers[#PhysX_Rigid_Body].meshType == 2) then (
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
	)
	if (obj.modifiers[#PhysX_Rigid_Body].meshType == 5) then (
		-- if not in expShapesNames export
		if (findItem expShapesNames obj.name) == 0 then (
			exportMeshShape obj
			append expShapesNames obj.name
		)
		mString = "<colliderMesh path=\"" + obj.name + "\" staticFriction=\"0.5\" dynamicFriction=\"0.8\" restitution=\"0.2\" />"
	)
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

fn validateCameras obj = (
		return (((classof obj) == targetcamera)or((classof obj) == freecamera))
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
	
fn validatePlayer obj = (
	if (isProperty obj.baseObject #objectPlayerData) == false then
	--messageBox ("El Player necesita los player data")
	return (((obj.name) == PlayerName)and(isProperty obj.baseObject #objectPlayerData) )
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
	
fn validatePivotPlayer obj = (
		return (((obj.name) == PlayerPivotName)and(isProperty obj.baseObject #objectPlayerPivotData) )
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
	
fn validateCameraPivot obj = (
		return (((obj.name) == CameraPivotName)and(isProperty obj.baseObject #objectCameraPivotData) )
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
	
fn validateCamera obj = (
		return (((obj.name) == CameraName)and(isProperty obj.baseObject #objectCameraData) )
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
	
fn validatePointLight obj = (
		return ((classof obj) == Omnilight)
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
	
fn validateAmbientLight obj = (
		return ((classof obj) == Skylight)
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
	
	local mString = "<directionalLight " \
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
		return ((classof obj) == Directionallight)
)

/**********************************************************************************************
Gets a Directional Light component
 Params:
  - obj: target object
 Returns:
  - String with the component information
***********************************************************************************************/
fn getDistanceText obj = (
	
	if((validateDistanceText obj) == false) then
	return ""
	
	local mString = "<distanceText " \
										+ "text=\"" + obj.textData as string + "\" " \
										+ "distance=\"" + obj.distanceData as string + "\" " \
										+ "size=\"" + obj.sizeData as string + "\" " \
										+ "color=\"" + obj.colorData as string + "\" " \										
										+ "/>\n"
	return mString
)	
	
fn validateDistanceText obj = (
		return isProperty obj.baseObject #distanceTextData == true
)


/**********************************************************************************************
										COMPONENTES A SUCIO,  
											 SIN ATRIBUTOS 
											NI PULIMIENTOS
										PARA PRIMERA ITERACIÓN
***********************************************************************************************/

/*
	La intención de este get es obtener el mismo enemy controller para todos los tipos de enemigos
	falta meter los datos propios de cada tipo de enemigo, como velocidad de movimiento y de rotación
	para eso hay que añadir los datos al addControllers, es decir los datos del componente enemy controller
*/
fn getEnemyController obj = (
	
	if((validateEnemy obj) == false) then
	return ""
	
	return "<enemyController/>"
)	
	
fn validateEnemy obj = (
	return ( (obj.name) == BasicEnemyName)
)



fn getAiFsmBasic obj = (
	
	if((validateAiFsmBasic obj) == false) then
	return ""
	
	return "<aiFSMBasic/>"
)	
	
fn validateAiFsmBasic obj = (
	return (((obj.name) == BasicEnemyName) == true)
)



fn getLevelMeshCollider obj = (
	
	if((validateLevelMeshCollider obj) == false) then
	return ""
	
	return "<colliderMesh path=\"Level\" staticFriction=\"0.5\" dynamicFriction=\"0.8\" restitution=\"0.2\" />"
)	
	
fn validateLevelMeshCollider obj = (
	return (((obj.name) == levelName) == true)
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
	
	local world2local = inverse obj.transform
	local max2mcv = rotateXMatrix -90
	local world2mcv = world2local * max2mcv
	
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
				local v = #( mpos.x, mpos.y, mpos.z, uvA.x, 1-uvA.y, n.x, n.y, n.z ) -- , N.x, N.y, N.z )...
				
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
	
	-- Save in binary form to file
	saveBinaryMesh obj all_vtxs all_idxs all_groups 1004		-- Positions+UV2 only
	
	-- Delete the temp mesh
	if obj != obj_to_export then delete obj
)


fn saveBinaryMesh obj vtxs idxs groups vertex_type = (
	local data_path = out_path
	local filename = data_path + "meshes\\"
	filename += obj.name + ".mesh"
	
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
	
fn importTexture infile = (
	local base_name = getFilenameFile infile
	local full_target = out_path + "textures/" + base_name + ".dds"
	
	-- Convert to dds using texconv.exe por ejemplo
	--copyFile infile full_target
	local cmd = tools_path + "texconv.exe -f DXT5 \"" + infile + "\" -o " + out_path + "textures\\"
	format "Executing: --%--\n" cmd
	local log_file = out_path + "log\\" + base_name + ".log"
	local rc = DOSCommand (cmd + " > " + log_file)
	format "  rc = %\n" rc
	return base_name
)
	
fn importMaterial m = (
	if export_materials == false then return 0
	-- if is exported	
	if (m.diffusemap == undefined) then return 0
	if (findItem expDiff m.diffusemap.filename) != 0 then return 0
	
	append expDiff m.diffusemap.filename
	 
	if classof m != Standardmaterial then throw( "No se importar materiales que no sean std" )
	local diff_name = importTexture m.diffusemap.filename
	--	importTexture m.normalmap.filename	
	-- Exportar la definicion del material a un xml
	local out_filename = out_path + "materials\\" + m.name + ".xml"
	local mfs = createfile out_filename
	if mfs == undefined then throw ("Can't create material filename" + out_filename )
	format "<std_material name=\"%\" diffuse=\"%\"/>\n" m.name diff_name to:mfs
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
		if classof sm == Standardmaterial then return sm
		if classof sm == Multimaterial then throw( "El objeto " + obj.name + " submaterial " + (submesh as string) + " es a la vez un multimaterial!")
		throw ("No se exportar materiales de tipo " + ( classof(  sm )  as string ))
	end
	throw ("No se exportar materiales de tipo " + ( classof(  sm )  as string ))
	return m
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


fn max2mcvMatrix obj = (
	local max_world = obj.transform
	local max2mcv = rotatexmatrix -90
	local mcv2max = inverse max2mcv
	local mcv_world = mcv2max * max_world * max2mcv
	return mcv_world
)


-- Execution
--addObjectsID()
--exportXMLScene()
 --funcion que abre y cierra una última cosa para poder trabajar, a depurar
--openCloseFile()
--removeObjectsID()
)