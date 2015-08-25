list = #()
m_name = "suelo_dvn1"

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