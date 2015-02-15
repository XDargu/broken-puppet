#ifndef _CAMERA_CONTROLLER_H
#define _CAMERA_CONTROLLER_H

class CCamera;
class camera_pivot_controller;

class CCamera_controller
{
private:
	float mouse_sensivity;
	float overlap_y;
	float overlap_z;
public:
	CCamera_controller();
	~CCamera_controller();
	void setMouseSentivility(float newSensivility);
	void setOverlaps(float over_y, float over_z);
	bool clapAngleY(float angley);
	void rangeAngles();
	void update(CCamera* cam, camera_pivot_controller* CPC, float deltaTime);
};
#endif

