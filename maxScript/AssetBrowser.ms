macroScript MCVAssetBrowser
	category: "MCV tools"
	buttonText: "MCV Asset Browser"
	toolTip:	"Herramienta de edición de niveles y exportador del Broken Puppet"
	icon:#("ViewportNavigationControls",9)
(

	filein("GeneralConfig.ms")
	filein("Exporter.ms")
	filein("RenderThumbnail.ms")

	-- AssetPath
	 --AssetPath = "C:\\Users\\Daniel\\Documents\\Assets\\"

	-- Module path and list
	 ModulePath = AssetPath + "Modules\\"	
	 ModuleList = #()

	-- Prefabs
	 PrefabPath = AssetPath + "Prefabs\\"
	 PrefabList = #()	
	


	--clearListener()

		fn saveModulePicture dir assetName m = (
			
			vt=vsai_Thumbnails()
			f_name = dir + "icons\\" + assetName as string 
			
			vt.RenderIt objs: m filetype: "png" rSkylight: false rPath: f_name
			
			/*if (classof $ == ObjectSet) then (			
				obj = m as array
				
				p_max = $.max
				p_min = $.min

				p_center = (p_min + p_max) / 2
				p_final = [p_center.x, p_center.y, p_min.z]
				
				dist = (p_max.z - p_final.z)
				
				if (dist < 0.0001) then ( dist = 8)
				print dist
				rendercam = Targetcamera fov:60 nearclip:1 farclip:1000 \ 
				nearrange:0 farrange:1000 mpassEnabled:off mpassRenderPerPass:off \
				 pos: (p_final + [-1,-1,1.4] * dist) \
				isSelected:off \
				target:(Targetobject transform:(matrix3 [1,0,0] [0,1,0] [0,0,1] (p_final + [0, 0, dist * 0.475  ])))
				--target:(Targetobject transform:(matrix3 [1,0,0] [0,1,0] [0,0,1] (obj.pos + [0, 0, 3])))
				
				select obj
				f_name = dir + "icons\\" + assetName as string + ".png" 
				Render outputfile:f_name camera: rendercam outputSize: [120, 120] vfb: false forcew2sided: true renderType: #selection
				
				delete rendercam

			)
			else ( 		
				obj = m[1]
				select m
				
				dist = (obj.max.z - obj.pos.z)
				if (dist < 0.0001) then ( dist = 8)
				print dist
				rendercam = Targetcamera fov:60 nearclip:1 farclip:1000 \ 
				nearrange:0 farrange:1000 mpassEnabled:off mpassRenderPerPass:off \
				 pos: (obj.pos + [-1,-1,1.4] * dist) \
				isSelected:off \
				target:(Targetobject transform:(matrix3 [1,0,0] [0,1,0] [0,0,1] (obj.pos + [0, 0, dist * 0.475  ])))
				--target:(Targetobject transform:(matrix3 [1,0,0] [0,1,0] [0,0,1] (obj.pos + [0, 0, 3])))
				
				select obj
				f_name = dir + "icons\\" + assetName as string + ".png" 
				Render outputfile:f_name camera: rendercam outputSize: [120, 120] vfb: false forcew2sided: true renderType: #selection
				
				delete rendercam
			)*/
		)
		
		-- Find the better candidate to be the prefab pivot
		fn getParentCandidate sel = (		
			local cand	= undefined
			local maxCount = -1
			local count = 0
			
			select sel
			
			local p_max = $.max
			local p_min = $.min
			local p_center = (p_min + p_max) / 2
			local p_size = p_max - p_min
			
			for o in sel do (
				count = 0
				
				-- If it is snapped to the grid
				if ((mod o.pos.x 2 == 0) and (mod o.pos.z 2 == 0)) then
				(
					count += 1
				)
				
				-- If it is laying on the grid
				if (o.pos.z == 0) then
				(
					count += 1
				)
				
				-- If it is in the  center of the scene
				if (o.pos == [0,0,0]) then
				(
					count += 1
				)
				
				-- The nearests objects to the center are better candidates
				local distanceToCenter = p_center - o.pos
				distanceToCenter = distanceToCenter / p_size -- Normalize distance
				distanceToCenter = length distanceToCenter
				
				-- The weigth of the distance to center is 2 because almost never is going to reach values near 1
				count += distanceToCenter * 2
				
				if (count > maxCount) then ( 
					maxCount = count
					cand = o
				)
			)

			return cand
		)
		
		fn exportSelectedAsset dir assetName progress = (
			progress.value = 0	
			local typ=".max"	-- change this to the filetype you want

			local sel=selection as array
			local parentPos = (getParentCandidate sel).pos
			local parentRot = (getParentCandidate sel).rotation
			print parent
			
			local origTransforms = #()
			
			local count = 1
			for s in sel do
			(
				append origTransforms s.transform
				if (s.parent == undefined) then (
					s.pos -= parentPos
					--s.rotation = (quat 0 0 0 1)
				)
				
				progress.value = (100.0 / sel.count) * count
				count += 1
			)
			
			local f = dir+assetName+typ
			if typ==".max" then
			(
				saveNodes sel f quiet:true
			) else
			(
				select sel
				exportfile f selectedOnly:true --#noprompt
			)
			
			saveModulePicture dir assetName sel		
			count = 1
			for s in sel do (
				if (s.parent == undefined) then (
				s.transform = origTransforms[count]
				)
				count += 1
			)
		)

		fn exportSelectedModule dir progress = (
			
			progress.value = 0	
			local typ=".max"	-- change this to the filetype you want

			local sel=selection as array
			
			local origPositions = #()
			local origRotations = #()
			
			local count = 1
			for s in sel do
			(
				append origPositions s.pos
				append origRotations s.rotation
				s.pos = [0,0,0]
				s.rotation = (quat 0 0 0 1)
				local f = dir+s.name+typ
				if typ==".max" then
				(
					saveNodes s f quiet:true
				) else
				(
					select s
					exportfile f selectedOnly:true --#noprompt
				)
				progress.value = (100.0 / sel.count) * count
				count += 1
				
				saveModulePicture dir s.name #(s)
			)
			
			count = 1
			for s in sel do (
				s.rotation = origRotations[count]
				s.pos = origPositions[count]		
				count += 1
			)
		)

		fn loadModules dir = (	
			local modules = #()
			for f in ( getFiles (dir + "*.max") ) do
			(
				append modules (getFilenameFile f)
			)
			return modules	
		)
		
		fn isModule o = (		
			ModuleList = loadModules ModulePath
			
			ret = false
			-- Quick check: Modules have at least 24 characters
			if (o.name.count > 22) then (			
				-- Third check: is a module
				if (findItem ModuleList o.name != 0) then (
					ret = true
				)
			)		
			
			return ret
		)

		fn searchInList list search = (
			local items = #()
			local allSearchKeys = false
			for item in list do (
				allSearchKeys = true
				for skey in search do (
					if (findString item skey == undefined) then (
						allSearchKeys = false
						break
					)
				)
				
				if (allSearchKeys == true) then
					append items item
			)
			return items
		)

		fn createModuleInPoint dir p = (
			mergeMaxFile (dir) #mergeDups #useSceneMtlDups #select
			for obj in selection do (
				if (obj.parent == undefined) then 
				(
					obj.position += p
				)
			)		
			-- If the imported max is a Prefab, group it
			-- Grouping alters the pivot!!!!
			
			/*if ((findString dir PrefabPath) == 1) then (
				local prefName = "__pref_" + (substring dir (PrefabPath.count+1) (dir.count - PrefabPath.count - 4))
				group selection name: prefName
				select (getnodebyname prefName)
			)*/
			return selection
		)
		
		/**********************************************************************************************
		Object PATH structure
		 Params:
		  - PATH: path of vertex based in a spline
		***********************************************************************************************/
		objectSpinePath = attributes objectSpinePath
		(
			parameters main rollout:params
			(
				mPathName type:#string ui:SpinePathName default:""
			)
			 
			rollout params "Object Spine Path"
			(
				editText SpinePathName "SpinePathName" type: #string
			)
		)

		/**********************************************************************************************
			Set the mPathName of all the objects of the scene
		***********************************************************************************************/
		fn addObjectSpinePath = (	
			local obj = $
			if (obj != undefined) and (isProperty obj.baseObject #mPathName != true) then
			(
				custAttributes.add obj objectSpinePath
				objectSpinePath = obj.name + "_Path"
			)				
		)
		
		/**********************************************************************************************
		Object CLUEZONE structure
		 Params:
		  - CLUE POINT: point where the golden needle goes
		***********************************************************************************************/
		objectClueZone = attributes objectClueZone
		(
			parameters main rollout:params
			(
				mCluePoint type:#string ui:CluePointName default:""
			)
			 
			rollout params "Object Clue Point"
			(
				editText CluePointName "CluePointName" type: #string
			)
		)

		/**********************************************************************************************
			Set the Clue Point of all the objects of the scene
		***********************************************************************************************/
		fn addObjectClueZone = (	
			local obj = $
			if (obj != undefined) and (isProperty obj.baseObject #mCluePoint != true) then
			(
				actionMan.executeAction 0 "373"
				custAttributes.add obj objectClueZone
				objectClueZone = obj.name + "_Clue"
			)				
		)

		global createModuleSelectionPoint
		fn createModuleSelectionPoint dir = (
			local p = pickPoint() 
			if (classOf p == Point3) then (
				createModuleInPoint dir p		
				--createModuleSelectionPoint dir
			)
		)
		
		/**********************************************************************************************
		Object DISTANCETEXT structure
		 Params:
		  - MDTSTRING: text to show near the position
		***********************************************************************************************/
		objectDistanceText = attributes objectDistanceText
		(
			parameters main rollout:params
			(
				mDistTextString type:#string ui:mDistTextString default:"inserte aqui su texto"
				mDistTextDistance type:#float ui:mDistTextDistance default:0.0
				mDistTextSize type:#float ui:mDistTextSize default:0.0
				mDistTextColor type:#string ui:mDistTextColor default:"FFFFFFFF"
			)
			 
			rollout params "Object Distance Text"
			(
				editText mDistTextString "Texto" type: #string
				spinner  mDistTextDistance "Distancia" type: #float
				spinner  mDistTextSize "Tamanyo" type: #float
				editText mDistTextColor "Color" type: #string
			)
		)

		/**********************************************************************************************
			Set the String of all the objects of the scene
		***********************************************************************************************/
		fn addObjectsDistanceText = 
		(		
			local objSelected = $
			if (isProperty objSelected.baseobject #mString != true) then
			(
				custAttributes.add objSelected objectDistanceText	
							
			)			
		)
		
	
		/**********************************************************************************************
		Object PARTICLE SYSTEM structure
		 Params:
		  - MSTRING: text to show near the position
		***********************************************************************************************/
		objectParticleSystem = attributes objectParticleSystem
		(
			parameters main rollout:params
			(
				mSystemName type:#string ui:mSystemName default:"ps_dust"
				mSystemActive type:#boolean ui:mSystemActive default:true
			)
			 
			rollout params "Object Particle System"
			(
				editText mSystemName "Nombre" type: #string
				checkbox mSystemActive "Activo" type: #boolean
			)
		)

		/**********************************************************************************************
			Set the String of all the objects of the scene
		***********************************************************************************************/
		fn addObjectsParticleSystem = 
		(		
			local objSelected = $
			if (isProperty objSelected.baseobject #mSystemActive != true) then
			(
				custAttributes.add objSelected objectParticleSystem
			)			
		)
		
			

		
		
	/**********************************************************************************************
		Activate all the particle systems in the scene
	***********************************************************************************************/	
	fn activateParticleSystems = 
	(				
		
		for obj in $* do(
			print "hola"
			if  (isProperty obj.baseobject #mSystemActive == true) then (
				obj.mSystemActive = true
			)			
		)		
	)	
	
	/**********************************************************************************************
		Deactivate all the particle systems in the scene
	***********************************************************************************************/	
	fn deactivateParticleSystems = 
	(				
		
		for obj in $* do(
			if  (isProperty obj.baseobject #mSystemActive == true) then (
				obj.mSystemActive = false
			)			
		)		
	)	
	
		
	/**********************************************************************************************
	Object AUDIO SOURCE structure
	 Params:
	  - MSTRING: text to show near the position
	***********************************************************************************************/
	objectAudioSource = attributes objectAudioSource
	(
		parameters main rollout:params
		(
			mAudioSound type:#string ui:mAudioSound default:"light.wav"
			mAudioLoop type:#boolean ui:mAudioLoop default:true
			mAudioDistance type:#float ui:mAudioDistance default:40
			mAudioVolume type:#float ui:mAudioVolume default:0.6
			mAudioActive type:#boolean ui:mAudioActive default:true
		)
		 
		rollout params "Object Distance Text"
		(
			editText mAudioSound "Sound" type: #string
			checkbox mAudioLoop "Loop" type: #boolean
			spinner  mAudioDistance "Distancia" type: #float
			spinner  mAudioVolume "Volume" type: #float
			checkbox mAudioActive "Active" type: #boolean
		)
	)

	/**********************************************************************************************
		Set the String of all the objects of the scene
	***********************************************************************************************/
	fn addObjectsAudioSource = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #mAudioActive != true) then
		(
			custAttributes.add objSelected objectAudioSource
		)			
	)
	
	
	
	
	objectIsNotVisible = attributes objectIsNotVisible
	(
		parameters main rollout:params
		(
			collisionMesh type:#boolean ui:collisionMesh default:true
		)

		rollout params "Object Custom IsNotVisible"
		(
			checkbox collisionMesh "collisionMesh" type: #boolean
		)
	)
	
	fn addObjectsIsNotVisible = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #collisionMesh != true) then
		(
			custAttributes.add objSelected objectIsNotVisible	
		)	
		else(
				objSelected.collisionMesh = true
		)
		
	)
	
	fn AddAudioSource = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #mAudioActive != true) then
		(
			custAttributes.add objSelected objectAudioSource	
		)	
		else(
				objSelected.mAudioActive = true
		)
		
	)
	
	fn setNotVisible = 
	(				
		local allObj = $*
		
		for obj in allObj do(
			if  (isProperty obj.baseobject #IsNotVisible == true) then (
				obj.IsNotVisible = true
			)			
		)		
	)	
		
	
	/**********************************************************************************************
	Object  structure Trigger
	 Params:
	  - TRIGGER: Indicate that the object is a trigger
	***********************************************************************************************/
	objectDataTrigger = attributes objectTrigger
	(
		parameters main rollout:params
		(
			TRIGGER type:#boolean ui:TRIGGER default:true
			Logic_element type:#boolean ui:Logic_element default:true
			bot_only type:#boolean ui:bot_only default:true
			player_only type:#boolean ui:player_only default:true
		)

		rollout params "Object Custom Trigger"
		(
			checkbox TRIGGER "Trigger activo" type: #boolean
			checkbox Logic_element "Logico" type: #boolean
			checkbox bot_only "Solo Bot" type: #boolean
			checkbox player_only "Solo Player" type: #boolean
		)
	)
	
	fn addObjectsTrigger = 
	(		
		local objSelected = $
		actionMan.executeAction 0 "373"
		if (isProperty objSelected.baseobject #TRIGGER != true) then
		(
			custAttributes.add objSelected objectDataTrigger	
						
		)	
		
	)
	
	/**********************************************************************************************
	Object  structure RecastAABB
	 Params:
	  - RECASTAABB: Indicate that the object is a RecastAABB
	***********************************************************************************************/
	objectDataRecastAABB = attributes objectRecastAABB
	(
		parameters main rollout:params
		(
			RECASTAABB type:#boolean ui:RECASTAABB default:true
			Logic_element type:#boolean ui:Logic_element default:true
		)

		rollout params "Object Custom Recast AABB"
		(
			checkbox RECASTAABB "RECASTAABB" type: #boolean
			checkbox Logic_element "Logic_element" type: #boolean
		)
	)
	
	fn addObjectsRecastAABB = 
	(		
		local objSelected = $
		actionMan.executeAction 0 "373"
		if (isProperty objSelected.baseobject #RECASTAABB != true) then
		(
			custAttributes.add objSelected objectDataRecastAABB	
						
		)	
		
	)
	
	/**********************************************************************************************
	Object  structure RoomAABB
	 Params:
	  - ROOM: Indicate that the object is a room
	***********************************************************************************************/
	objectDataRoomAABB = attributes objectRoomAABB
	(
		parameters main rollout:params
		(
			ROOM type:#boolean ui:ROOM default:true
			Logic_element type:#boolean ui:Logic_element default:true
		)

		rollout params "Object Custom Room AABB"
		(
			checkbox ROOM "ROOM" type: #boolean
			checkbox Logic_element "Logic_element" type: #boolean
		)
	)
	
	fn addObjectsRoomAABB = 
	(		
		local objSelected = $
		actionMan.executeAction 0 "373"
		if (isProperty objSelected.baseobject #ROOM != true) then
		(
			custAttributes.add objSelected objectDataRoomAABB
						
		)	
		
	)
	
	/**********************************************************************************************
	Object  structure Parent
	Params:
	  - mParent: parent node
	***********************************************************************************************/
	objectDataParent = attributes objecParent
	(
		parameters main rollout:params
		(
			mParent type:#node
		)

		rollout params "Object Custom Parent"
		(
			pickbutton chooseParent "Select Parent"
			
			on params open do (
				if (mParent != undefined) then (
					chooseParent.text = mParent.name
				)
			)
			
			on chooseParent picked obj do  (
				mParent = obj
				chooseParent.text = obj.name
			)
		)		
		
	)
	
	fn addObjectsParent = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #mParent != true) then
		(
			custAttributes.add objSelected objectDataParent
						
		)	
		
	)
	
	/**********************************************************************************************
	Object  structure Vibration
	Params:
	  - mVibrationAmount: Intensity of the vibration
	  - mVibrationFrequency: Frequency of the vibration
	***********************************************************************************************/
	objectDataVibration = attributes objectVibration
	(
		parameters main rollout:params
		(
			mVibrationAmount type:#float ui:mVibrationAmount default:0.01
			mVibrationFrequency type:#float ui:mVibrationFrequency default:100
		)

		rollout params "Object Custom Vibration"
		(
			spinner  mVibrationAmount "Intensidad" type: #float
			spinner  mVibrationFrequency "Frecuencia" type: #float
		)
	)
	
	fn addObjectsVibration = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #mVibrationAmount != true) then
		(
			custAttributes.add objSelected objectDataVibration
						
		)	
	)
	
	/**********************************************************************************************
	Object  structure LocalRotation
	Params:
	  - mLocalRotationSpeed: Speed of the rotation
	  - mLocalRotationAxis: Local axis (0: fron, 1: right, 2: up)
	***********************************************************************************************/
	objectDataLocalRotation = attributes objectLocalRotation
	(
		parameters main rollout:params
		(
			mLocalRotationSpeed type:#float ui:mLocalRotationSpeed default:2
			mLocalRotationAxis type:#string default: "X"
		)

		rollout params "Object Custom Local Rotation"
		(
			spinner  mLocalRotationSpeed "Velocidad" type: #float
			dropdownlist mLocalRotationAxisList "Eje" items:# ("X","Y","Z")
			
			on params open do (
				mLocalRotationAxisList.selection = findItem mLocalRotationAxisList.items mLocalRotationAxis
			)
			
			on mLocalRotationAxisList selected i do  (
				mLocalRotationAxis = mLocalRotationAxisList.items [i]
			)
		)		
		
	)
	
	fn addObjectsLocalRotation = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #mLocalRotationSpeed != true) then
		(
			custAttributes.add objSelected objectDataLocalRotation
						
		)	
		
	)
	
	/**********************************************************************************************
	Object  structure RoomAABB
	 Params:
	  - ROOM: Indicate that the object is a room
	***********************************************************************************************/
	objectDataRoomAABB = attributes objectRoomAABB
	(
		parameters main rollout:params
		(
			ROOM type:#boolean ui:ROOM default:true
			Logic_element type:#boolean ui:Logic_element default:true
		)

		rollout params "Object Custom Room AABB"
		(
			checkbox ROOM "ROOM" type: #boolean
			checkbox Logic_element "Logic_element" type: #boolean
		)
	)
	
	fn addObjectsRoomAABB = 
	(		
		local objSelected = $
		actionMan.executeAction 0 "373"
		if (isProperty objSelected.baseobject #ROOM != true) then
		(
			custAttributes.add objSelected objectDataRoomAABB
						
		)	
		
	)
	
	
	/**********************************************************************************************
	Object  structure CollisionTag
	 Params:
	  - Is an collisionTag
	***********************************************************************************************/
	objectDataCollisionTag = attributes objectCollisionTag
	(
		parameters main rollout:params
		(
			collisionTag type:#string ui:collisionTag default:"undefined"
		)

		rollout params "Object Custom collision tag"
		(
			Edittext collisionTag "collisionTag" type: #string
		)
	)
	
	fn addObjectsCollisionTag = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #collisionTag != true) then
		(
			custAttributes.add objSelected objectDataCollisionTag	
		)	
		
	)

	/**********************************************************************************************
	Object  structure materialTag
	 Params:
	  - Is an materialTag
	***********************************************************************************************/
	objectDataMaterialTag = attributes objectMaterialTag
	(
		parameters main rollout:params
		(
			materialTag type:#string ui:materialTag default:"wood"
		)

		rollout params "Object Custom material tag"
		(
			Edittext materialTag "materialTag" type: #string
		)
	)
	
	fn addObjectsMaterialTag = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #materialTag != true) then
		(
			custAttributes.add objSelected objectDataMaterialTag	
		)	
		
	)
	
	
	/**********************************************************************************************
	Object  structure NoEval
	 Params:
	  - NOEVAL: Indicate that the object is not evaluable
	***********************************************************************************************/
	objectDataNoEval = attributes objectNoEval
	(
		parameters main rollout:params
		(
			NOEVAL type:#boolean ui:NOEVAL default:true
		)

		rollout params "Object Custom NoEval"
		(
			checkbox NOEVAL "NOEVAL" type: #boolean
		)
	)
	
	fn addObjectsNoEval = 
	(		
		local objSelected = $
		actionMan.executeAction 0 "373"
		if (isProperty objSelected.baseobject #NOEVAL != true) then
		(
			custAttributes.add objSelected objectDataNoEval
						
		)	
		
	)
	
		
	/**********************************************************************************************
	Object  structure Logic
	 Params:
	  - Logic: Unique object logic
	***********************************************************************************************/
	objectDataLogic = attributes objectLogic
	(
		parameters main rollout:params
		(
			Logic_element type:#boolean ui:Logic_element default:true
		)

		rollout params "Object Custom Logic"
		(
			checkbox Logic_element "Logic_element" type: #boolean
		)
	)
	
	fn addObjectsLogic = 
	(
		local objSelected = $
		if (isProperty objSelected.baseObject #Logic_element != true) then
		(
			custAttributes.add objSelected objectDataLogic		
		)	
	)
	
	
		/**********************************************************************************************
		Object OCLUSION PLANE structure
		 Params:
		  - MSTRING: text to show near the position
		***********************************************************************************************/
		objectOclusionPlane = attributes objectOclusionPlane
		(
			parameters main rollout:params
			(
				mOclusionPlane type:#boolean ui:mOclusionPlane default:true
			)
			 
			rollout params "Object Oclusion Plane"
			(
				checkbox mOclusionPlane "Oclusion" type: #boolean
			)
		)

		/**********************************************************************************************
			Set the String of all the objects of the scene
		***********************************************************************************************/
		fn addObjectsOclusionPlane  = (		
			local objSelected = $
			if(classof objSelected == Plane) then(
				if (isProperty objSelected.baseobject #mOclusionPlane != true) then(
					custAttributes.add objSelected objectOclusionPlane
					custAttributes.add objSelected objectDataLogic
					custAttributes.add objSelected objectIsNotVisible							
				)			
			)
			else(
				messagebox "El Oclusion Plane debe hacerse sobre planos"
			)
			
		)
		
		
		-- Reloads all the instances of the asset with the given path
		fn updateAsset dir assetName = (
			amount = 0
			objs = $* as array
			for obj in objs do (
				if (obj.name == assetName) then (
					-- Reimport the module
					newMod = createModuleInPoint (dir + ".max" as string) obj.pos
					newMod.transform = obj.transform
					delete obj
					amount += 1
				)
			)
			return amount
		)

	(	
		global TestFloater
		
		-- Main window
		rollout LevelEdition "Edición de niveles"
		(		
			-- Buttons placed at the top
			button botonA "Subir" pos:[5,3] width:75 height:25 toolTip:""
			button botonB "Bajar" pos:[86,3] width:75 height:25 toolTip:""
			button botonC "Backface cull" pos:[167,3] width:75 height:25 toolTip:""
			button buttonAddPath "Platform path" pos:[248,3] width:75 height:25 toolTip:""
			button button8Grid "Grid 8x8" pos:[5,34] width:75 height:25 toolTip:""
			button button4Grid "Grid 4x4" pos:[86,34] width:75 height:25 toolTip:""
			button button2Grid "Grid2x2" pos:[167,34] width:75 height:25 toolTip:""
			button button1Grid "Grid 1x1" pos:[248,34] width:75 height:25 toolTip:""
			button botonAddTrigger "Add Trigger" pos:[5,65] width:75 height:25 toolTip:""
			button botonAddLogic "Add Logic" pos:[86,65] width:75 height:25 toolTip:""
			button botonAddRoomAABB "Add RoomAABB" pos:[167,65] width:75 height:25 toolTip:""
			button botonAddNoEval "Add NoEval" pos:[248,65] width:75 height:25 toolTip:""
			button botonAddClueZone "Add ClueZone" pos:[5,99] width:75 height:25 toolTip:""
			button botonAddRecastAABB "Add RecastAABB" pos:[86,99] width:75 height:25 toolTip:""
			button botonAddDistanceText "Add DistanceText" pos:[167,99] width:75 height:25 toolTip:""
			button botonAddParticleSystem "Add ParticleSystem" pos:[248,99] width:75 height:25 toolTip:""
			button botonActivateParticles "Activate Particles" pos:[5,130] width:75 height:25 toolTip:""
			button botonDeactivateParticles "Deactivate Particles" pos:[89,130] width:75 height:25 toolTip:""
			button botonNotVisible "Not Visible" pos:[167,130] width:75 height:25 toolTip:""
			button botonAddAudioSource "Audio Source" pos:[248,130] width:75 height:25 toolTip:""
			button botonAddOclusionPlane "OclusionPlane" pos:[5,161] width:75 height:25 toolTip:""
			button botonAddCollisionTag "Collision Tag" pos:[86,161] width:75 height:25 toolTip:""
			button botonAddMaterialTag "Material Tag" pos:[167,161] width:75 height:25 toolTip:""			
			button botonAddVibration "Add vibration" pos:[5,192] width:75 height:25 toolTip:""
			button botonAddLocalRotation "Add Local Rotation" pos:[86,192] width:75 height:25 toolTip:""
			button botonAddParent "Add Parent" pos:[167,192] width:75 height:25 toolTip:""
			
							
			on botonA pressed do	( 
				objs = $*
				for o in objs do (
					o.pos.z += getGridSpacing()
				)
			) -- A message pop up when you click the button
			on botonB pressed do	(
				objs = $*
				for o in objs do (
					o.pos.z -= getGridSpacing()
				)
				--filein "box.ms"
			)	-- box.ms is the name of an external Script we execute when the button is clicked	
			on botonC pressed do (
				objs = $*
				for o in objs do (
					if (o.backfaceCull == true) then (
						o.backfaceCull = false
					)
					else (
						o.backfaceCull = true
					)
				)
			)
			
			on buttonAddPath pressed do (
				addObjectSpinePath()
			)
			on button8Grid pressed do (
				setGridSpacing 8
			)
			on button4Grid pressed do (
				setGridSpacing 4
			)
			on button2Grid pressed do (
				setGridSpacing 2
			)
			on button1Grid pressed do (
				setGridSpacing 1
			)
			on botonAddTrigger pressed do (
				addObjectsTrigger()
			)
			on botonAddLogic pressed do (
				addObjectsLogic()
			)
			on botonAddRoomAABB pressed do (
				addObjectsRoomAABB()
			)
			on botonAddNoEval pressed do (
				addObjectsNoEval()
			)
			on botonAddClueZone pressed do (
				addObjectClueZone()
			)
			on botonAddRecastAABB pressed do (
				addObjectsRecastAABB()
			)
			on botonAddDistanceText pressed do (
				addObjectsDistanceText()
			)
			on botonAddParticleSystem pressed do (
				addObjectsParticleSystem()
			)		
			on botonActivateParticles pressed do (
				activateParticleSystems()
			)
			on botonDeactivateParticles pressed do (
				deactivateParticleSystems()
			)
			on botonNotVisible pressed do (
				addObjectsIsNotVisible()
			)
			on botonAddAudioSource pressed do (
				AddAudioSource()
			)
			on botonAddOclusionPlane pressed do (
				addObjectsOclusionPlane()
			)
			on botonAddCollisionTag pressed do (
				addObjectsCollisionTag()
			)
			on botonAddMaterialTag pressed do (
				addObjectsMaterialTag()
			)
			on botonAddVibration pressed do (
				addObjectsVibration()
			)
			on botonAddLocalRotation pressed do (
				addObjectsLocalRotation()
			)
			on botonAddParent pressed do (
				addObjectsParent()
			)
		)
		
		rollout AttributesEdition "Edición de Atributos"
		(
			group "Exportador"
			(
				button createPrefabs "Crear Prefabs" width:110 height:25 toolTip:"" across: 1
			)
			
			on createPrefabs pressed do (
				local aux = 0
				for i in PrefabList do (				
					local pos = Point3 (aux * 10) 3 0
					print pos
					print (PrefabPath + i as string + ".max")
					newMod = createModuleInPoint (PrefabPath + i as string + ".max") pos	
					aux = aux + 1
				)		
			)
		)
		
		rollout Exporter "Exportador" 
		(
			group "Exportador"
			(
				checkbox exportMeshCheck "Exportar meshes" across: 2
				checkbox exportMaterial "Exportar materiales"
				button exportAll "Exportar todo" width:110 height:25 toolTip:"" across: 2 
				button exportSel "Exportar selección" width:110 height:25 toolTip:"" 
				button preView "Previsualizar" width:135 height:25 toolTip:""
				button validScn "Validar escena" width:135 height:25 toolTip:""
				button exportLightmaps "Exportar lightmaps" width:135 height:25 toolTip:""
				button exportMod "Exportar módulos" width:135 height:25 toolTip:""
				button exportPref "Exportar prefabs" width:135 height:25 toolTip:""
				button buildAndPlay "Exportar y ejecutar" width:135 height:25 toolTip:""
				button play "Ejecutar" width:135 height:25 toolTip:""
				progressbar progress					
			)
			
			on Exporter open do (
				if (ExportMeshes != undefined) then (
					exportMeshCheck.checked = ExportMeshes)
				if (export_materials != undefined) then
					exportMaterial.checked = export_materials
			)
			
			on buildAndPlay pressed do (
				try (
					disableSceneRedraw()
					-- Fisicaaas
					objs = $* as array
					objs_m = #()
					for o in objs do (
						if isModule o then (
							append objs_m o						
						)
					)
					
					select objs_m
					macros.run "PhysX" "PxSetSelAsStaticRigidBodyMS"
					
					for o in objs_m do (
						select o
						o.modifiers[#PhysX_Rigid_Body].meshType = 5
					)
					
					addObjectsID()
					exportXMLScene progress: progress
					--funcion que abre y cierra una última cosa para poder trabajar, a depurar
					openCloseFile()
					removeObjectsID()
					enableSceneRedraw()
					
					ShellLaunch  (engine_path + "engine.exe") ""
				)
				catch ( 
					format "*** % ***\n" (getCurrentException())
					messageBox ("No se ha podido exportar correctamente:\n " + getCurrentException() as string) 
					enableSceneRedraw()
				)
			)
			
			on preView pressed do (
				try (
						addObjectsID()
						exportViewer progress: progress
						--funcion que abre y cierra una última cosa para poder trabajar, a depurar
						openCloseFile()
						removeObjectsID()
						ShellLaunch  (engine_path + "engined.exe") ""
					)
					catch ( 
						format "*** % ***\n" (getCurrentException())
						messageBox ("No se ha podido exportar correctamente:\n " + getCurrentException() as string) 
					)
			)
			
			on play pressed do (
				ShellLaunch  (engine_path + "engine.exe") ""
			)
			
			on exportLightmaps pressed do (
				addObjectsID()
				for o in objs do (
					if (o.modifiers[#PhysX_Rigid_Body] == undefined) then (
						BakeLightmap o 256
					)
					else (
						if (o.modifiers[#PhysX_Rigid_Body].type != 1) then (
							BakeLightmap o 256
						)
					)
				)
				removeObjectsID()
			)
			
			on exportAll pressed do (
					try (
						addObjectsID()
						exportXMLScene progress: progress
						--funcion que abre y cierra una última cosa para poder trabajar, a depurar
						openCloseFile()
						removeObjectsID()
					)
					catch ( 
						format "*** % ***\n" (getCurrentException())
						messageBox ("No se ha podido exportar correctamente:\n " + getCurrentException() as string) 
					)
				)
				on exportSel pressed do (
					if ($ == undefined) then ( messageBox "No hay nada seleccionado" )
					else (
						try (
							exportMesh $
						)
						catch ( 
							format "*** % ***\n" (getCurrentException())
							messageBox ("No se ha podido exportar correctamente:\n " + getCurrentException() as string) 
						)
					)
				)
				on exportMod pressed do (
					if ($ == undefined) then ( messageBox "No hay nada seleccionado" )
					else ( exportSelectedModule ModulePath progress )
				)
				on exportPref pressed do (
					if ($ == undefined) then ( messageBox "No hay nada seleccionado" )
					else (
						defName = "Escribe un nombre"
						if (classof $ == ObjectSet) then (defName =  $[1].name)
						else ( defName = $.name )
						-- instantiate the object
						theObj = dotNetObject "MaxCustomControls.RenameInstanceDialog" defName
						theobj.text ="New prefab"
						DialogResult = theObj.Showmodal()

						--test if the ok button was pressed
						dotnet.compareenums TheObj.DialogResult ((dotnetclass "System.Windows.Forms.DialogResult").OK)
						--get the new text string
						assetName = theobj.InstanceName
						
						exportSelectedAsset PrefabPath assetName progress
					)
				)
				on validScn pressed do (
					if (validateScene false == true) then ( messageBox "La escena es correcta" )
					
				)
				on exportMeshCheck changed theState do (
					ExportMeshes = theState
				)	
				
				on exportMaterial changed theState do (
					export_materials = theState
				)	
		)
		
		-- Módulos
		rollout Modules "Módulos"
		(	
			fn loadDynamicModules list = (		
				local rci = rolloutCreator "RModules" "Lista de módulos" width:400 height:400
				rci.begin()
				
				for item in list do (
					--rci.addControl #button (item as string) (item as string) 
					--rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
					--labelText = substring item 1 20
					local space = "  "
					local labelText = item
					for s = 0  to ((21 - item.count) / 2) do (
						labelText = space + labelText
					)
					local fName = substituteString ModulePath "\\" "\\\\"
					rci.addControl #button (item as string) (item as string) paramStr: ("offset: [30, 0] across: 4 images: #(\"" + (fName as string) + "icons\\\\" + (item as string) + ".png\", undefined, 1,1,1,1,1) " as string)
					rci.addControl #label ((item + "name") as string) (labelText as string) paramStr: "align: #left height: 20 width:125 offset: [-65, 130]"
					rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
				)
				
				rci.end()
				
				return rci.def
			)
			
			edittext searchText "" 
			group "Búsqueda avanzada" (
				checkbox searchArq "Arquitectónicos" align: #left triState: 1
				
				checkbox searchRmp "Rampas" align: #left across: 3 triState: 1
				checkbox searchTub "Tuberías" align: #left
				checkbox searchSelev "Suelo elevado" align: #left
				
				checkbox searchSlo "Suelo" align: #left across: 4
				checkbox searchTch "Techo" align: #left
				checkbox searchPrd "Pared" align: #left
				checkbox searchAmb "Suelo/techo" align: #left
				
				checkbox searchEsq "Esquinas" align: #left across: 2
				checkbox searchPas "Pasillos" align: #left
				
				checkbox searchPar "Parches" align: #left across: 2
				checkbox searchHlf "Mitades" align: #left
				
			)
			button searchButton "Buscar"  across: 2
			pickbutton searchPickButton "Seleccionar y buscar" 
			subRollout moduleSubRollout "test1" height:400
			progressbar progress
			button goToLocation "Abrir ubicación" across: 3
			button reload "Actualizar"
			button rebake "Recargar"
			pickbutton rebakePick "Seleccionar y recargar"
			button autoCeiling "Auto-techar escena"
			
			on rebake pressed do (
				ModuleList = loadModules ModulePath
				progress.value = 0
				
				countTotal = 0
				count = 0
				for module in ModuleList do (
					countTotal += updateAsset (ModulePath + module as string) module
					count += 1
					progress.value = (100.0 / ModuleList.count) * count
				)
				messagebox ("Se han recargado " + (count as string) + " módulos")
			)
			
			on reload pressed do (
				ModuleList = loadModules ModulePath
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicModules ModuleList)
			)
			
			on rebakePick picked obj do 
			(
				ModuleList = loadModules ModulePath
				local searchValues = #(obj.name)
				
				local searchList = searchInList ModuleList searchValues
				
				progress.value = 0
				
				countTotal = 0
				count = 0
				for module in searchList do (
					countTotal += updateAsset (ModulePath + module as string) module
					count += 1
					progress.value = (100.0 / ModuleList.count) * count
				)
				messagebox ("Se han recargado " + (count as string) + " módulos")
			)
			
			on searchPickButton picked obj do
			(
				ModuleList = loadModules ModulePath
				local searchValues = #(obj.name)
				
				local searchList = searchInList ModuleList searchValues
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicModules searchList)
			)
			
			on searchButton pressed do (
				ModuleList = loadModules ModulePath
				local searchValues = #(searchText.text)
				-- Advanced search
				if (searchArq.state == true) then
					append searchValues "arqui"
				
				if (searchRmp.state == true) then
					append searchValues "rampa"
				if (searchSelev.state == true) then
					append searchValues "selev"				
				if (searchTub.state == true) then
					append searchValues "tuber"
				
				if (searchSlo.state == true) then
					append searchValues "suelo"
				if (searchTch.state == true) then
					append searchValues "techo"
				if (searchPrd.state == true) then
					append searchValues "pared"
				if (searchAmb.state == true) then
					append searchValues "ambos"
				
				if (searchEsq.state == true) then
					append searchValues "esqui"
				if (searchPas.state == true) then
					append searchValues "pasil"
				
				if (searchPar.state == true) then
					append searchValues "parche"
				if (searchHlf.state == true) then
					append searchValues "half"
				
				local searchList = searchInList ModuleList searchValues
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicModules searchList)
			)
			
			on goToLocation pressed do (
				shellLaunch "explorer.exe" ModulePath
			)
			
			on autoCeiling pressed do (
				-- Auto create ceiling modules
				progress.value = 0
				count = 0
				
				objs = $* as array
				clearSelection()
				for o in objs do (
					if (isModule o == true) then (
						if ((substring o.name 11 5) == "suelo") or ((substring o.name 11 5) == "pared") then (
							-- Is there a module over it?
							module_over = false
							for o2 in objs do (
								if (o2.pos == (o.pos + [0, 0, 8])) then (								
									if (isModule o) then (
										if ((substring o2.name 11 5) == "techo") then (
											module_over = true
											break
										)
									)
								)
							)
							
							if (module_over == false) then (
								-- Up module check
								moduleUp = replace o.name 11 5 "techo"
								--print moduleUp
								newMod = createModuleInPoint (ModulePath + moduleUp + ".max") (o.pos + [0, 0, 8])
								newMod.rotation = o.rotation
								newMod.pos = o.pos + [0,0,8]
							)
						)
					)
					
					count += 1
					progress.value = (100.0 / objs.count) * count
				)
			)
			
			on Modules open do (
				ModuleList = loadModules ModulePath
				
				local searchValues = #(searchText.text)
				-- Advanced search
				append searchValues "arqui"
				append searchValues "rampa"
				
				local searchList = searchInList ModuleList searchValues
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicModules searchList)
			)
		)
		
		-- Módulos
		rollout Prefabs "Prefabs"
		(	
			fn loadDynamicPrefabs list = (		
				local rci = rolloutCreator "RPrefabs" "Lista de prefabs" width:400 height:400
				rci.begin()
				
				for item in list do (
					--rci.addControl #button (item as string) (item as string) 
					--rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
					space = "  "
					labelText = item
					for s = 0  to ((21 - item.count) / 2) do (
						labelText = space + labelText
					)
					local fName = substituteString PrefabPath "\\" "\\\\"
					rci.addControl #button (item as string) (item as string) paramStr: ("offset: [30, 0] across: 4 images: #(\"" + (fName as string) + "icons\\\\" + (item as string) + ".png\", undefined, 1,1,1,1,1) " as string)
					rci.addControl #label ((item + "name") as string) (labelText as string) paramStr: "align: #left height: 20 width:125 offset: [-65, 130]"
					rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (fName as string) + (item as string) + ".max@)" as string) filter:on
				)
				
				rci.end()
				
				return rci.def
			)
			
			edittext searchText "" 
			button searchButton "Buscar" text: "Player"
			subRollout moduleSubRollout "test1" height:400
			button goToLocation "Abrir ubicación" across: 2
			button reload "Actualizar"		
			
			on reload pressed do (
				PrefabList = loadModules PrefabPath
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicPrefabs PrefabList)
			)
			
			on searchButton pressed do (
				PrefabList = loadModules PrefabPath
				local searchValues = #(searchText.text)
				
				local searchList = searchInList PrefabList searchValues
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicPrefabs searchList)
			)
			
			on goToLocation pressed do (
				shellLaunch "explorer.exe" PrefabPath
			)
			
			on Prefabs open do (
				PrefabList = loadModules PrefabPath
				local searchValues = #("Player")
				
				local searchList = searchInList PrefabList searchValues
				
				for ro in moduleSubRollout.rollouts do
					removeSubRollout moduleSubRollout ro
				
				addSubRollout moduleSubRollout (loadDynamicPrefabs searchList)
			)		
		)

		try (closerolloutfloater TestFloaster) catch ()
		
		TestFloater = NewRolloutFloater "Tools" 340 825
		cui.RegisterDialogBar TestFloater style: #(#cui_handles, #cui_dock_left , #cui_dock_right, #cui_floatable)
		
		AddRollout LevelEdition TestFloater rolledUp:on heigth: 120
		AddRollout AttributesEdition TestFloater rolledUp:on 
		AddRollout Exporter TestFloater rolledUp:on
		AddRollout Modules TestFloater rolledUp:on
		AddRollout Prefabs TestFloater rolledUp:on
		
		setGridSpacing 8
	)
)