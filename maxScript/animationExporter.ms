
fn exportAnimation obj = (
	local anim = ""
	for i = 1 to 100 do (
		local tpos = at time i obj.pos
		local trot = at time i obj.transform.rotationPart as quat
		local aline = "<keyFrame position=\"" + tpos.x as string + ", " + tpos.y as string + ", " + tpos.z as string + "\""
		aline += " rotation=\"" + trot.x as string + ", " + trot.y as string + ", " + trot.z as string + ", " + trot.w as string + "\""
		aline += "/>"
		anim += aline + "\n"
	)
	print anim
)