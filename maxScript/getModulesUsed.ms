fn getModulesUsed = (
	local allObjects = $*
	
	local modules = #()
	
	for obj in allObjects do 
	(
		if ((findItem modules (obj.name)) == 0) then
			append modules obj.name
	)
	
	print modules
)