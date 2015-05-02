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
		
		local obj_rot  = biped.getTransform obj #rotation
		
		local mObj = max2mcvMatrix mJoint
		local qObj = mObj.rotationPart as quat
		qObj = quat -qObj.x -qObj.y -qObj.z qObj.w

		local mString = "<BoneRigidbody "
		
		mString += "name= \"" + obj.name + "\" "
		mString += "rotation= \"" + obj_rot.x as string+" "\
												+ obj_rot.y as string+" "\
												+ obj_rot.z as string+" "\
												+ obj_rot.w as string+"\" "
		
		mString += "density= \"" + obj.modifiers[#PhysX_Rigid_Body].density as string + "\" "				
		mString += "staticfriction= \"" + obj.modifiers[#PhysX_Rigid_Body].StaticFriction as string + "\" "
		mString += "dynamicfriction= \"" + obj.modifiers[#PhysX_Rigid_Body].DynamicFriction as string + "\" "
		mString += "bounciness= \"" + obj.modifiers[#PhysX_Rigid_Body].Bounciness as string + "\" "
		mString += "meshtype= \"" + obj.modifiers[#PhysX_Rigid_Body].MeshType as string + "\" "
		mString += "radius= \"" + obj.modifiers[#PhysX_Rigid_Body].meshradius as string + "\" "
		mString += "height= \"" + obj.modifiers[#PhysX_Rigid_Body].meshheight as string + "\" "
		mString += " />\n"
		
		return mString
	)	
	
	
	fn getConstraint obj = (
		
		if (classof obj != UConstraint) then return ""
		
		mJoint = obj
		
		mString = "<jointD6 "
	
		local mObj = max2mcvMatrix mJoint
		local qObj = mObj.rotationPart as quat
		qObj = quat -qObj.x -qObj.y -qObj.z qObj.w
		
		-- Translations limits
		-- mode 1 = Locked, 2 = Limited, 3 = Free
		
		i = 1;
		-- Actors
		if (mJoint.body0 != undefined) then (		
			mString += "actor" + i as string + " = \"" + mJoint.body0.name + "\" "
			i += 1;
		)
		
		if (mJoint.body1 != undefined) then 
		mString += "actor" + i as string + " = \"" + mJoint.body1.name + "\" "
		
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
		/*
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
		*/
		
	)
	
	-- Create a file stream....
	
	--gc()
	--local out_filename = out_path + "RagdollPrueba.xml"
	--fs = createfile out_filename
	--if fs == undefined then throw ("Can't create scene filename" + out_filename )	
	
	local allObj = $*
	
	local mString = ""
	for obj in allObj do (
		mString += getBoneRigidbody obj
	)
	
	for obj in allObj do (	
		mString += getConstraint obj
	)
	
	print mString
)

