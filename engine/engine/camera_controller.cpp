#include "mcv_platform.h"
#include "camera_controller.h"
#include "camera.h"
#include "camera_pivot_controller.h"
#include "iostatus.h"

using namespace DirectX;

float anglex;
float angley;
float old_pitch;
float top_angle_x = 0.5f;
float down_angle_x = -1.5f;

CCamera_controller::CCamera_controller()
	: mouse_sensivity(0.01f),
	overlap_y(1.5f),
	overlap_z(2.f)
{
}


CCamera_controller::~CCamera_controller()
{
}

void CCamera_controller::setMouseSentivility(float newSensivility){
	mouse_sensivity = newSensivility;
}

void CCamera_controller::setOverlaps(float over_y, float over_z){
	overlap_y = over_y;
	overlap_z = over_z;
}

void CCamera_controller::update(CCamera* cam, camera_pivot_controller* CPC, float deltaTime)
{
	float yaw = getYawFromVector(CPC->getCamPivot()->getPosition());
	float pitch = getPitchFromVector(CPC->getCamPivot()->getPosition());
	yaw = deg2rad(yaw);
	pitch = deg2rad(pitch);
	anglex = anglex - io.getMouse().dx*mouse_sensivity;
	angley = angley + io.getMouse().dy*mouse_sensivity;
	yaw = yaw + anglex;


	if (clapAngleY(angley)){
		old_pitch = pitch + angley;
	}else{
		rangeAngles();
	}

	XMVECTOR newRotation = XMQuaternionRotationRollPitchYaw(old_pitch, yaw, 0.f);
	XMVECTOR RotationWithOutPitch = XMQuaternionRotationRollPitchYaw(0.f, yaw, 0.f);
	newRotation = XMQuaternionNormalize(newRotation);
	CPC->getCamPivot()->setRotation(newRotation);
	CPC->getPlayerPivot()->setRotation(RotationWithOutPitch);
	cam->getCamEntity()->setPosition(CPC->getCamPivot()->getPosition() + CPC->getCamPivot()->getUp()*overlap_y - CPC->getCamPivot()->getFront()*overlap_z);
	dbg("Y angle: %f\n", angley);
	cam->lookAt(cam->getCamEntity()->getPosition(), CPC->getCamPivot()->getPosition() + CPC->getCamPivot()->getFront() / 2, CPC->getCamPivot()->getUp());
	io.update(deltaTime);
}

bool CCamera_controller::clapAngleY(float angley){
	bool result = false;
	if ((angley < top_angle_x) && (angley > down_angle_x)){
		result = true;
	}
	return result;
}

void CCamera_controller::rangeAngles(){
	if (angley > top_angle_x)
		angley = top_angle_x;
	if (angley < down_angle_x)
		angley = down_angle_x;
}
