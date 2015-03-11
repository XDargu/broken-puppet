plugin simpleObject Suelo
name:"dvn_arqui_suelo_esqui2_ex_01"
classID:#(0x2404bd31, 0x76b3dfd7)
category:"Modulos"
(
	parameters main rollout:params
	(

	)
	rollout params "Parameters"
	(
		button shLoc "Open Location" toolTip:"plugin's file"width:94
		on shLoc pressed do (
			shellLaunch "explorer.exe" (getSourceFileName() as string)
		)
	)
	
	tool create
	(
		on mousePoint click do (
			case click of
			(
				1: (coordSys grid (
						position = gridPoint
						mergeMaxFile ("C:\\Users\\Daniel\\Documents\\Modules\\dvn_arqui_suelo_esqui2_ex_01.max")  #mergeDups #select
						module = selection
						module.pos = position
						) 
					)
					
				3: #stop
			)
		)
		
		on mouseMove click do
		case click of
		(
			2: print "hey"
		)
	)
)

