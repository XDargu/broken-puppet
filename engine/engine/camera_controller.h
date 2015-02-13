#ifndef _CAMERA_CONTROLLER_H
#define _CAMERA_CONTROLLER_H

class CCamera;
class camera_pivot_controller;

class CCamera_controller
{
float mouse_sensivity;
public:
	CCamera_controller();
	~CCamera_controller();
	void update(CCamera* cam, camera_pivot_controller* CPC, float deltaTime);
};
#endif

