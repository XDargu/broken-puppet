filein("Exporter.ms")

clearListener()
(	
	
	-- SOLO FUNCIONA CON UN OBJETO --TODO: QUE FUNCIONE CON VARIOS
	fn saveModulePicture dir m = (
		
		obj = m
		select m
		
		rendercam = Targetcamera fov:60 nearclip:1 farclip:1000 \ 
		nearrange:0 farrange:1000 mpassEnabled:off mpassRenderPerPass:off \
		/*pos: (obj.pos + [10, 10, 10])*/ pos: (obj.pos + [-1,-1,1.4] * (obj.max.z - obj.pos.z)) \
		isSelected:off \
		target:(Targetobject transform:(matrix3 [1,0,0] [0,1,0] [0,0,1] (obj.pos + [0, 0, (obj.max.z - obj.pos.z) * 0.475  ])))
		--target:(Targetobject transform:(matrix3 [1,0,0] [0,1,0] [0,0,1] (obj.pos + [0, 0, 3])))
		
		
		select obj
		f_name = dir + "icons\\" + obj.name as string + ".png" 
		Render outputfile:f_name camera: rendercam outputSize: [120, 120] vfb: false forcew2sided: true renderType: #selection
		
		delete rendercam
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
			
			saveModulePicture dir s
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

	fn searchInList list search = (
		local items = #()
		for item in list do (
			if (findString item search != undefined) then
				append items item
		)
		return items
	)

	fn createModuleInPoint dir p = (
		mergeMaxFile (dir) #mergeDups #useSceneMtlDups #select
		for obj in selection do (
			if activeGrid == $45degGrid then (
				obj.rotation *= (quat 0 0 0.382683 0.92388)
			)
			obj.position += p
		)
	)

	fn createModuleSelectionPoint dir = (
		local p = pickPoint() 
		if (classOf p == Point3) then (
			createModuleInPoint dir p		
			--createModuleSelectionPoint dir
		)
	)

	-- AssetPath
	AssetPath = "D:\\Users\\Daniel\\Documents\\Assets\\"

	-- Module path and list
	ModulePath = AssetPath + "Modules\\"	
	ModuleList = loadModules ModulePath

	-- Prefabs
	PrefabPath = AssetPath + "Prefabs\\"
	PrefabList = loadModules PrefabPath

	global TestFloater
	
	-- Main window
	rollout LevelEdition "Edición de niveles"
	(		
		-- Buttons placed at the top
		button botonA "Subir" pos:[5,3] width:75 height:25 toolTip:""
		button botonB "Bajar" pos:[87,3] width:75 height:25 toolTip:""
		button botonC "Backface cull" pos:[5,34] width:75 height:25 toolTip:""
		button botonD "Cambiar grid" pos: [87,34] width:75 height:25 toolTip:""
			
		on botonA pressed do	( 
			objs = $*
			for o in objs do (
				o.pos.z += 8
			)
		) -- A message pop up when you click the button
		on botonB pressed do	(
			objs = $*
			for o in objs do (
				o.pos.z -= 8
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
		on botonD pressed do (
			if activeGrid == $45degGrid then (
				activeGrid = undefined
			)
			else (
				activeGrid = $45degGrid
			)
		)
	)
	
	rollout Exporter "Exportador" 
	(
		group "Exportador"
		(
			checkbox exportMesh "Exportar meshes" checked: ExportMeshes
			button exportAll "Exportar todo" width:110 height:25 toolTip:"" across: 2 
			button exportSel "Exportar selección" width:110 height:25 toolTip:"" 
			button validScn "Validar escena" width:135 height:25 toolTip:""
			button exportMod "Exportar módulos" width:135 height:25 toolTip:""
			progressbar progress					
		)
		
		on exportAll pressed do (
				addObjectsID()
				exportXMLScene progress: progress
				 --funcion que abre y cierra una última cosa para poder trabajar, a depurar
				openCloseFile()
				removeObjectsID()
				/*try (filein "Exporter.ms")
				catch ( messageBox "No se ha podido exportar correctamente" )*/
			)
			on exportSel pressed do (
				filein "ExporterBasic.ms"
				/*try (filein "ExporterBasic.ms")
				catch ( messageBox "No se ha podido exportar correctamente" )*/
			)
			on exportMod pressed do (
				exportSelectedModule ModulePath progress
			)
			on validScn pressed do (
				if (validateScene false == true) then ( messageBox "La escena es correcta" )
				
			)
			on exportMesh changed theState do (
				ExportMeshes = theState
			)	
	)
	
	-- Módulos
	rollout Modules "Módulos"
	(		
		bitmap image width: 120
		edittext searchText "" width: 180 across: 2
		button searchButton "Buscar" align: #right
		listbox theListbox "" items:ModuleList height: 6
		button create "Crear" across: 3
		button goToLocation "Abrir ubicación"
		button reload "Actualizar" 
		
		on create pressed do (
			if (theListbox.selected != undefined) then
				createModuleSelectionPoint (ModulePath + theListbox.selected + ".max")
		)
		
		on reload pressed do (
			ModuleList = loadModules ModulePath
			theListbox.items = ModuleList
		)
		
		on searchButton pressed do (
			ModuleList = loadModules ModulePath
			theListbox.items = searchInList ModuleList searchText.text
		)
		
		on goToLocation pressed do (
			shellLaunch "explorer.exe" ModulePath
		)
		
		on theListbox selected sel do (
			image.fileName = ModulePath + "icons\\" + theListbox.selected + ".png"
		)
	)
	
	-- Prefabs
	rollout Prefabs "Prefabs"
	(
		bitmap image width: 120
		edittext searchText "" width: 180 across: 2
		button searchButton "Buscar" align: #right
		listbox theListbox "" items:PrefabList height: 6
		button create "Crear" across: 3
		button goToLocation "Abrir ubicación"
		button reload "Actualizar" 
		/*button boton "dvn_arqui_suelo_esqui2_ex_01" pos:[5,3] width:67 height:25 toolTip:""
		button boton1 "EXP_2" pos:[75,3] width:67 height:25 toolTip:""
		button boton3 "EXP_3" pos:[5,31] width:67 height:25 toolTip:""
		button boton4 "EXP_4" pos:[75,31] width:67 height:25 toolTip:""
		-- These buttons won't do anything when clicked
		*/
		on create pressed do (
			if (theListbox.selected != undefined) then
				createModuleSelectionPoint (PrefabPath + theListbox.selected + ".max")
		)
		
		on reload pressed do (
			PrefabList = loadModules PrefabPath
			theListbox.items = PrefabList
		)
		
		on searchButton pressed do (
			ModuleList = loadModules PrefabPath
			theListbox.items = searchInList PrefabList searchText.text
		)
		
		on goToLocation pressed do (
			shellLaunch "explorer.exe" PrefabPath
		)
		
		on theListbox selected sel do (
			image.fileName = PrefabPath + "icons\\" + theListbox.selected + ".png"
		)

	)

	try (closerolloutfloater TestFloaster) catch ()
	
	TestFloater = NewRolloutFloater "Tools" 285 475	
	--cui.RegisterDialogBar TestFloater style: #(#cui_handles, #cui_dock_left , #cui_dock_right, #cui_floatable)
	
	AddRollout LevelEdition TestFloater rolledUp:on
	AddRollout Exporter TestFloater rolledUp:on
	AddRollout Modules TestFloater rolledUp:on
	AddRollout Prefabs TestFloater rolledUp:on
	
	setGridSpacing 8
	-- 45 degrees grid
	if ($45degGrid == undefined) then (
		oGrid = grid()
		oGrid.name = "45degGrid"
	)
	$45degGrid.rotation = (quat 0 0 0.382683 0.92388)
	$45degGrid.grid = 8
	$45degGrid.length = 200
	$45degGrid.width = 200

)