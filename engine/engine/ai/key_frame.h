#ifndef _KEY_FRAME_H_
#define _KEY_FRAME_H_

#include <DirectXMath.h>
using DirectX::XMFLOAT3;
#include "handle\handle.h"

struct TKeyFrame {
	CHandle target_transform;
	XMFLOAT3 target_position;
	XMFLOAT4 target_rotation;
	float time;
	float elapsed_time;

	TKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);

	bool update(float elapsed);

	bool operator==(const TKeyFrame& kf) {
		bool a = kf.target_transform == target_transform;
		bool b = target_position.x == kf.target_position.x && target_position.y == kf.target_position.y && target_position.z == kf.target_position.z;
		bool c = target_rotation.x == kf.target_rotation.x && target_rotation.y == kf.target_rotation.y && target_rotation.z == kf.target_rotation.z && target_rotation.w == kf.target_rotation.w;
		bool d = kf.time == time;
		return a && b  && c && d;
	}
};

#endif