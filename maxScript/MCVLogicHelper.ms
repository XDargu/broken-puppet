macroScript MCVLogicHelper
	category:	"MCV tools"
	buttonText:	"MCV Logic Helper"
	toolTip:	"Herramienta de lógica, Brocken Puppet"
	icon:#("ViewportNavigationControls",8)
(
	
	/**********************************************************************************************
	Object  structure NotChangePivot
	 Params:
	  - NotChangePivot: Indicate that the object is a RecastAABB
	***********************************************************************************************/
	objectDataNotChange = attributes objectNotChange
	(
		parameters main rollout:params
		(
			NotChange type:#boolean ui:NotChange default:true
		)

		rollout params "Object Custom Recast AABB"
		(
			checkbox NotChange "Not Change Pivot" type: #boolean
		)
	)
	
	fn addObjectsNotChange = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #NotChange != true) then
		(
			custAttributes.add objSelected objectDataNotChange	
		)	
		
	)

	(
		
		global TestFloater2
		
		-- Módulos
		rollout SelectTool "Selecciones"
		(				
			edittext searchNameText "" 
			button searchNameButton "Buscar por nombre"  across: 2
			on searchNameButton pressed do (
				local mPattern = #(searchNameText.text)
				mPattern = mPattern[1] + "*"
				print mPattern
				local selectedList= for i in $*  where matchpattern i.name pattern:mPattern collect i	
				select selectedList
			)			
		)
		rollout PhysxTool "Fisicas"
		(				
			button addDynamicsButton "Dynamics"  across: 3
			button addBoxButton "Box Collider"  across: 3
			button addSphereButton "Sphere Collider"  across: 3
			button addNotChangePivot "No change pivot" across:1
			
			on addDynamicsButton pressed do (
				macros.run "nvpxMenu" "nvpxMenu9"
			)	
			on addBoxButton pressed do (
				local allObj = $
				for obj in allObj do(
					obj.modifiers[#PhysX_Rigid_Body].meshType = 2
				)
				
			)	
			on addSphereButton pressed do (				
				local allObj = $
				for obj in allObj do(
					obj.modifiers[#PhysX_Rigid_Body].meshType = 1
				)
			)	
			on addNotChangePivot pressed do (
				addObjectsNotChange()
			)
		)

		
		try (closerolloutfloater TestFloaster2) catch ()
		
		TestFloater2 = NewRolloutFloater "Tools" 500 340
		cui.RegisterDialogBar TestFloater2 style: #(#cui_handles, #cui_dock_left , #cui_dock_right, #cui_floatable)
		AddRollout SelectTool TestFloater2 rolledUp:off
		AddRollout PhysxTool TestFloater2 rolledUp:off

	)
)