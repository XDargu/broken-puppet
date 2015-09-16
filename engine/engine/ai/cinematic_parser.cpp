#include "mcv_platform.h"
#include "cinematic_parser.h"

void CCinematicParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	
	if (elem == "animation") {
		animation.setTargetTransfrom(target_transform);
	}
	if (elem == "keyFrame") {
		XMVECTOR position = atts.getPoint("position");
		XMVECTOR rotation = atts.getQuat("rotation");
		float time = 1 / 30.0f; // 30 FPS animations
		animation.addKeyframe(position, rotation, time);
	}
}