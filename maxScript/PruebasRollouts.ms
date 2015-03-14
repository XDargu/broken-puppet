-- AssetPath
AssetPath = "D:\\Users\\Daniel\\Documents\\Assets\\"

-- Module path and list
ModulePath = AssetPath + "Modules\\"	
ModuleList = #()

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
			print dir
			createModuleInPoint dir p		
			--createModuleSelectionPoint dir
		)
	)
	
rollout test "test" width:400 height:400
(	
	fn loadModules dir = (	
		local modules = #()
		for f in ( getFiles (dir + "*.max") ) do
		(
			append modules (getFilenameFile f)
		)
		return modules	
	)
	
	
	fn dynamic_buttons list = (
		
		local rci = rolloutCreator "RModules" "Modules" width:400 height:400
		rci.begin()
		
		for item in list do (
			--rci.addControl #button (item as string) (item as string) 
			--rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
			rci.addControl #button (item as string) (item as string) paramStr: ("across: 3 images: #(\"" + (ModulePath as string) + "icons\\" + (item as string) + ".png\", undefined, 1,1,1,1,1) " as string)
			rci.addHandler (item as string) #pressed codeStr: ("createModuleSelectionPoint(@" + (ModulePath as string) + (item as string) + ".max@)" as string) filter:on
		)
		/*rci.addControl #spinner ("x"+i as string) ("X "+i as string) paramStr:"range:[0,50000,0]"
		rci.addControl #spinner ("y"+i as string) ("Y "+i as string) paramStr:"range:[0,50000,0]"
		
		rci.addControl #button ("set"+i as string) ("Set "+i as string)
		
		
		rci.addHandler ("set"+i as string) #pressed filter:on codeStr:(@"format @% %\n@ "+ ("x"+i as string) +".value "+ ("y"+i as string) +".value")*/
		
		rci.end()
		
		return rci.def
	)
	
	fn function_pointer edge_Count=
	(
		local array_Rollouts = #();
		
		for i = 1 to edge_Count do
		(
			local rci = rolloutCreator ("point_"+i as string) ("Point "+i as string) width:400 height:400
			rci.begin()
			
			rci.addControl #spinner ("x"+i as string) ("X "+i as string) paramStr:"range:[0,50000,0]"
			rci.addControl #spinner ("y"+i as string) ("Y "+i as string) paramStr:"range:[0,50000,0]"
			
			rci.addControl #button ("set"+i as string) ("Set "+i as string)
			
			
			rci.addHandler ("set"+i as string) #pressed filter:on codeStr:(@"format @% %\n@ "+ ("x"+i as string) +".value "+ ("y"+i as string) +".value")
			
			rci.end()
			
			append array_Rollouts rci.def
		)
		return array_Rollouts;
	)
	
	spinner amount "Cantidad" type: #integer
	button create "Recrear" images: #((ModulePath + "icons\\" + "Box8x8" + ".png" as string), undefined, 1,1,1,1,1) 
	
	subRollout test1 "test1" pos:[0,170] width:400 height:400
	
	on create pressed do 
	(
		ModuleList = loadModules ModulePath
		
		for ro in test1.rollouts do
			removeSubRollout test1 ro
		
		addSubRollout test1 (dynamic_buttons ModuleList)
		
		/*for ro in function_pointer amount.value do
			addSubRollout test1 ro rolledUp:true*/
	)
)
	
createDialog test