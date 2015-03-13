-- AssetPath
AssetPath = "D:\\Users\\Daniel\\Documents\\Assets\\"

-- Module path and list
ModulePath = AssetPath + "Modules\\"	
ModuleList = #()

-- Prefabs
PrefabPath = AssetPath + "Prefabs\\"
PrefabList = #()


clearListener()


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
	
	fn isModule o = (		
		ModuleList = loadModules ModulePath
		
		ret = false
		-- Quick check: Modules have at least 24 characters
		if (o.name.count > 23) then (			
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
			if activeGrid == $45degGrid then (
				obj.rotation *= (quat 0 0 0.382683 0.92388)
			)
			obj.position += p
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

	fn createModuleSelectionPoint dir = (
		local p = pickPoint() 
		if (classOf p == Point3) then (
			createModuleInPoint dir p		
			--createModuleSelectionPoint dir
		)
	)

(
	filein("Exporter.ms")
	
	global TestFloater
	
	-- Main window
	rollout LevelEdition "Edición de niveles"
	(		
		-- Buttons placed at the top
		button botonA "Subir" pos:[5,3] width:75 height:25 toolTip:""
		button botonB "Bajar" pos:[87,3] width:75 height:25 toolTip:""
		button botonC "Backface cull" pos:[5,34] width:75 height:25 toolTip:""
		button botonD "Grid 45/90º" pos: [87,34] width:75 height:25 toolTip:""
			
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
					try (filein "ExporterBasic.ms")
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
		fn loadDynamicModules list = (		
			local rci = rolloutCreator "RModules" "Lista de módulos" width:400 height:400
			rci.begin()
			
			for item in list do (
				--rci.addControl #button (item as string) (item as string) 
				--rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
				rci.addControl #button (item as string) (item as string) paramStr: ("across: 2 images: #(\"" + (ModulePath as string) + "icons\\" + (item as string) + ".png\", undefined, 1,1,1,1,1) " as string)
				rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
			)
			
			rci.end()
			
			return rci.def
		)
		
		edittext searchText "" 
		group "Búsqueda avanzada" (
			checkbox searchArq "Arquitectónicos" across: 2 align: #left triState: 1
			checkbox searchKit "Kits" align: #left
			
			checkbox searchSlo "Suelo" align: #left across: 4
			checkbox searchTch "Techo" align: #left
			checkbox searchPrd "Pared" align: #left
			checkbox searchAmb "Suelo/techo" align: #left
			
			checkbox searchEsq "Esquinas" align: #left across: 3
			checkbox searchPas "Pasillos" align: #left
			checkbox searchPar "Parches" align: #left
			
		)
		button searchButton "Buscar" 
		subRollout moduleSubRollout "test1" height:400
		button goToLocation "Abrir ubicación" across: 2
		button reload "Actualizar"
		button autoCeiling "Auto-techar escena"
		
		on reload pressed do (
			ModuleList = loadModules ModulePath
			
			for ro in moduleSubRollout.rollouts do
				removeSubRollout moduleSubRollout ro
			
			addSubRollout moduleSubRollout (loadDynamicModules ModuleList)
		)
		
		on searchButton pressed do (
			ModuleList = loadModules ModulePath
			local searchValues = #(searchText.text)
			-- Advanced search
			if (searchArq.state == true) then
				append searchValues "arqui"
			
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
			)
		)
		
		on Modules open do (
			ModuleList = loadModules ModulePath
			
			for ro in moduleSubRollout.rollouts do
				removeSubRollout moduleSubRollout ro
			
			addSubRollout moduleSubRollout (loadDynamicModules ModuleList)
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
				rci.addControl #button (item as string) (item as string) paramStr: ("across: 2 images: #(\"" + (PrefabPath as string) + "icons\\" + (item as string) + ".png\", undefined, 1,1,1,1,1) " as string)
				rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (PrefabPath as string) + (item as string) + ".max@)" as string) filter:on
			)
			
			rci.end()
			
			return rci.def
		)
		
		edittext searchText "" 
		button searchButton "Buscar" 
		subRollout moduleSubRollout "test1" height:400
		button goToLocation "Abrir ubicación" across: 2
		button reload "Actualizar"		
		
		on reload pressed do (
			ModuleList = loadModules ModulePath
			
			for ro in moduleSubRollout.rollouts do
				removeSubRollout moduleSubRollout ro
			
			addSubRollout moduleSubRollout (loadDynamicPrefabs ModuleList)
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
			
			for ro in moduleSubRollout.rollouts do
				removeSubRollout moduleSubRollout ro
			
			addSubRollout moduleSubRollout (loadDynamicPrefabs PrefabList)
		)		
	)

	try (closerolloutfloater TestFloaster) catch ()
	
	TestFloater = NewRolloutFloater "Tools" 340 675	
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