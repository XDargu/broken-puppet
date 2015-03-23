Struct vsai_Thumbnails
(

	previous_settings=#(),
	previous_backgroundColor=undefined,
	previous_lights=#(),
	ml_dome=undefined,
	--isoCam=undefined,
	angleCam=undefined,
	polyCountTxt=undefined,

	fn setup_isometric_view zoomfactor:2.5 objs: $*=
	(
	disableSceneRedraw()
		
		--fn CenterPt objs =  (local totalPos=[0,0,0]; for obj in objs do totalPos = totalPos + obj.pos; avgPos = totalPos / selection.count; avgPos )
		fn maxof vals = (local v=vals[1]; for v1 in vals do (if v1 > v do v=v1);v)
		
		--viewport.ResetAllViews()
		--viewport.setlayout #layout_1
		
		angleCam = Freecamera fov:45 pos:[0,0,0]
		rotate angleCam (angleaxis 45 [0,0,1])
		in coordsys angleCam
		(
			rotate angleCam (angleaxis 45 [1,0,0])
			angleCam.pos.z = ( angleCam.pos.z + 100 )
		)
		
		viewport.setcamera angleCam
		cam=angleCam
		local max2, fov, asp, v, dist
		vmax=0 -- declare local variables
		totalPos=[0,0,0]
		CenterOfSelection=[0,0,0]
		fov=0 -- initialize the fov value
		asp=1
		in coordsys cam -- work in coordinate system of the camera
		(
			for obj in objs where obj != cam do -- loop across all objects except the camera
			(
				if obj.min.z >=0 do continue -- if object is behind camera, skip it
				-- get max value of the object's bounding box, correcting for the image aspect ratio
				-- in the y values
				v = maxof #((abs obj.max.x),(abs obj.min.x),(abs (obj.max.y*asp)),(abs (obj.min.y*asp)))
				vmax = maxof #(v,vmax)
				in coordsys world totalPos = totalPos + obj.center; 
				CenterOfSelection = totalPos / objs.count
--				fov = maxof #(fov,(2*atan(-v/obj.min.z))) -- increase fov if needed
			)
		)
		
		dist= (vmax/(sin(cam.fov/2)))
		in coordsys world cam.pos = CenterOfSelection
 		in coordsys cam cam.pos.z = dist*((dist/vmax)/zoomfactor)
 	enableSceneRedraw()
	),
	
	fn hide_lights_and_cameras =
	(
		hidebycategory.lights = true
		hidebycategory.cameras = true
	),

	--snapshot of current light on/off settings
	fn save_light_state =
	(
		for i in lights do
			(
			try if i.on then append previous_lights i
			catch()
			try if i.enabled then append previous_lights i
			catch()
			)
		
		for i in previous_lights do 
			(
			try i.on=false
			catch()
			try i.enabled=false
			catch()
			)
		
	),

	--restore of current light on/off settings
	fn restore_light_state =
	(
		--restore lights
		for i in previous_lights do
			(
			try i.on=true
			catch()
			try i.enabled=true
			catch()		
			)
		
	),

	fn create_skylight = (	
		--ml_dome=VRayLight dome_spherical:off size0:0 size1:0 type:1 multiplier:1.3 on:on invisible:on subdivs:16 
		ml_dome = Skylight dome_spherical:off size0:0 size1:0 type:1 multiplier:1 on:on invisible:on subdivs:16 
	),
	fn delete_temp_objects = (
		try delete ml_dome catch()
		try delete isoCam catch()
		try delete angleCam catch()
		try delete polyCountTxt catch() 
		),

	fn render_thumbnail renderFileType rWidth rHeight rPath=
	(
		-- launches render through maxscript, which has the problem of sticking the render in the middle of the 2 monitors
		-- set up path to include the object and filename
		--RenderFileName = replace maxfilename (maxfilename.count-2) 3 renderFileType
		--Renderfile = (maxfilepath + RenderFileName)
		render outputwidth:rWidth outputheight:rHeight outputfile: (rPath + "." + renderFileType) vfb: false forcew2sided: true renderType: #selection
	),

	fn ShowCountInCamera =
	(
		WhiteMat=VRayLightMtl()
		WhiteMat.color=(color 255 255 255)
		WhiteMat.multiplier = 5
		
		in coordsys angleCam 
			polyCountTxt = text size:0.18 kerning:0 leading:0 isSelected:on pos:[0,-.5,-1.5]
			
		rotate polyCountTxt (angleaxis 45 [0,0,1])
		in coordsys local rotate polyCountTxt (angleaxis 45 [1,0,0])
		addmodifier polyCountTxt (UvwMap())
		polyCountTxt.wirecolor = (color 255 255 255 0)
		
		scf=0
		for i in $* do
		(
			try (
				tempfaces = getpolygoncount i
				scf += (tempfaces[1] as integer)
			)catch()
		)
		polyCountTxt.text = ((scf as string) + " Faces")
		converttomesh polyCountTxt
		polyCountTxt.material = WhiteMat
		
	),
	
	fn RenderIt rSkylight:true filetype:"png" width:120 height:120 dr:"false" rPath:"" objs: $*=
	(
		objs = for o in objs collect o
		setup_isometric_view objs: objs
		save_light_state()		
		if (rSkylight) do crweate_skylight()
		
		select objs
		
		render_thumbnail filetype width height rPath
		
		
		delete_temp_objects()
		restore_light_state()
	)
)