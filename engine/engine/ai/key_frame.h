#ifndef _KEY_FRAME_H_
#define _KEY_FRAME_H_

#include <DirectXMath.h>
using DirectX::XMVECTOR;
#include "handle\handle.h"

struct TKeyFrame {
	CHandle target_transform;
	XMVECTOR target_position;
	XMVECTOR target_rotation;
	float time;
	float elapsed_time;

	TKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);

	bool update(float elapsed);
};

#endif