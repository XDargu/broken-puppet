clearlistener()

(
	engine_path = "C:\\Code\\engine\\"
	out_path = engine_path + "data\\"
	prefab_path = out_path + "prefab\\"
	scene_path = out_path + "scene\\"
	tools_path = engine_path + "tools\\"
	
	fn getBoneRigidbody obj = (
		
		if (obj.modifiers[#PhysX_Rigid_Body] == undefined) then return ""

		local mString = "<BoneRigidbody "
		
		mString += "name= \"" + obj.name + "\" "
		mString += "density= \"" + obj.modifiers[#PhysX_Rigid_Body].density as string + "\" "				
		mString += "staticfriction= \"" + obj.modifiers[#PhysX_Rigid_Body].StaticFriction as string + "\" "
		mString += "dynamicfriction= \"" + obj.modifiers[#PhysX_Rigid_Body].DynamicFriction as string + "\" "
		mString += "bounciness= \"" + obj.modifiers[#PhysX_Rigid_Body].Bounciness as string + "\" "
		mString += "meshtype= \"" + obj.modifiers[#PhysX_Rigid_Body].MeshType as string + "\" "
		mString += "radius= \"" + obj.modifiers[#PhysX_Rigid_Body].meshradius as string + "\" "
		mString += "height= \"" + obj.modifiers[#PhysX_Rigid_Body].meshheight as string + "\" "
		mString += " />\n"
		
		--print mString
	)	
	
	
	fn getConstraint obj = ()
	
	-- Create a file stream....
	
	gc()
	local out_filename = out_path + "RagdollPrueba.xml"
	fs = createfile out_filename
	if fs == undefined then throw ("Can't create scene filename" + out_filename )	
	
	local allObj = $*
	
	for obj in allObj do (
		getBoneRigidbody obj
		print obj
		-- get contraings
		

	)
	

)

