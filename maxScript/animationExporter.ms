
fn max2mcvMatrixTransform trans = (
		local max_world = trans
		local max2mcv = rotatexmatrix -90
		local mcv2max = inverse max2mcv
		local mcv_world = mcv2max * max_world * max2mcv
		return mcv_world
)

fn inverseZCameraMatrixTransform trans = (
	return (rotatexmatrix 90) * trans
)
	
fn exportAnimation obj init_frame end_frame = (
	local prev_pos = null
	local anim = ""
	anim += "<animation>\n"
	for i = init_frame to end_frame do (
		local mpos = at time i obj.pos
		if (tpos != prev_pos) then (
			
			local timeTrans = at time i obj.transform
				
			local timeObj = timeTrans			
			timeObj = max2mcvMatrixTransform timeTrans			
			local tpos = timeObj.pos			
			
			if ((classof obj == Freecamera) or (classof obj == Targetcamera)) then (
				timeObj = inverseZCameraMatrixTransform timeObj
			)
			
			local qObj = timeObj.rotationPart as quat
			local trot = quat -qObj.x -qObj.y -qObj.z qObj.w
			
			local m_time = i
			local aline = "\t<keyFrame time=\"" + m_time as string + "\" position=\"" + tpos.x as string + " " + tpos.y as string + " " + tpos.z as string + "\""
			aline += " rotation=\"" + trot.x as string + " " + trot.y as string + " " + trot.z as string + " " + trot.w as string + "\""
			aline += "/>"
			anim += aline + "\n"
		)
		prev_pos = mpos
	)
	anim += "</animation>"
	return anim
)