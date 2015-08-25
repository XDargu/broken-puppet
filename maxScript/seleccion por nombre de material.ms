macroScript MCVRenombar
	category:	"MCV tools"
	buttonText:	"MCV Select by material"
	toolTip:	"Herramienta que selecciona objetos por material"
	icon:#("ViewportNavigationControls",6)
(	
	-- instantiate the object
	theObj = dotNetObject "MaxCustomControls.RenameInstanceDialog" "Material"
	theobj.text ="Nombre del material"
	DialogResult = theObj.Showmodal()

	--test if the ok button was pressed
	dotnet.compareenums TheObj.DialogResult ((dotnetclass "System.Windows.Forms.DialogResult").OK)
	--get the new text string
	m_name = theobj.InstanceName	
	list = #()
	

	for o in $* do (
		if (classof o.material == Multimaterial) then (
			for m in o.material do (
				if (m != undefined) then (
					if (m.name == m_name) then (
						append list o
					)
				)
			)
		)
		else (
			if (classof o.material == StandardMaterial) then (
				if (o.material.name == m_name) then (
					append list o
				)
			)
			else (
				if (classof o.material == UndefinedClass) then (
					print ("Warning: material undefined en objeto" + o.name)
				)
				else (
					throw ("No se exportar materiales de tipo " + ( classof(  o.material )  as string ))
				)
			)
		)
		
	)

	select list
)

