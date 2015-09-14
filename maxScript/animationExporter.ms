
fn exportAnimation obj = (
	local prev_pos = null
	local anim = ""
	anim += "<animation>\n"
	for i = 1 to 100 do (
		local tpos = at time i obj.pos
		if (tpos != prev_pos) then (
			local trot = at time i obj.transform.rotationPart as quat
			local m_time = i
			local aline = "\t<keyFrame time=\"" + m_time as string + "\" position=\"" + tpos.x as string + ", " + tpos.y as string + ", " + tpos.z as string + "\""
			aline += " rotation=\"" + trot.x as string + ", " + trot.y as string + ", " + trot.z as string + ", " + trot.w as string + "\""
			aline += "/>"
			anim += aline + "\n"
		)
		prev_pos = tpos
	)
	anim += "</animation>"
	print anim
)