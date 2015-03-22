clearlistener()

(
	local engine_path = "C:\\Code\\engine\\"
   	local out_path = engine_path + "data\\"
	local prefab_path = out_path + "prefabs\\"
	local scene_path = out_path + "scenes\\"
	local mesh_path = out_path + "meshes\\"
	local tools_path = engine_path + "tools\\"
	local collider_path = mesh_path + "colliders\\"
	
	
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
	
	
	fn getConstraint obj = (
		
		if (classof obj != UConstraint) then return ""
		
		-- translation limits
		print obj.linearModeX -- 1 lock, 2 limited, 3 free
		print obj.linearModeY
		print obj.linearModeZ 
		
		-- swing z limits
		print obj.swing1mode -- 1 lock, 2 limited, 3 free
		if(obj.swing1mode == 2) then
		print obj.swing1Angle 
		
		-- swing y limits
		print obj.swing2mode -- 1 lock, 2 limited, 3 free
		if(obj.swing2mode == 2) then
		print obj.swing2Angle 
		
		-- twist limits (x)
		print obj.twistMode -- 1 lock, 2 limited, 3 free
		if(obj.twistMode == 2) then
		print obj.twistAngleHigh
		print obj.twistAngleLow 
		
		-- spring 
		print obj.posSpring
		print obj.posDamping
		print obj.swingSpring
		print obj.swingDamping
		print obj.twistSpring
		print obj.twistDamping
		
		--Advanced
		print obj.Collision
		print obj.posSpring
		
		print obj.breakable
		if (obj.breakable) then (
		print obj.maxForce
		print obj.maxTorque
		)
		
		
	)
	
	-- Create a file stream....
	
	--gc()
	--local out_filename = out_path + "RagdollPrueba.xml"
	--fs = createfile out_filename
	--if fs == undefined then throw ("Can't create scene filename" + out_filename )	
	
	local allObj = $*
	
	for obj in allObj do (
		getBoneRigidbody obj
		getConstraint obj
		
		-- get contraings
		
	)
	

)

