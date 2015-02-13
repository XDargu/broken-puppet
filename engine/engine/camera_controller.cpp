#include "mcv_platform.h"
#include "camera_controller.h"
#include "camera.h"
#include "entity.h"
#include "camera_pivot_controller.h"
#include "iostatus.h"

using namespace DirectX;

float anglex;
float angley;

CCamera_controller::CCamera_controller()
	: mouse_sensivity(0.01f)
{
}


CCamera_controller::~CCamera_controller()
{
}

void CCamera_controller::update(CCamera* cam, camera_pivot_controller* CPC, float deltaTime)
{
	float yaw = getYawFromVector(CPC->getCamPivot()->getPosition());
	float pitch = getPitchFromVector(CPC->getCamPivot()->getPosition());
	yaw = deg2rad(yaw);
	pitch = deg2rad(pitch);
	anglex = anglex + io.getMouse().dx*mouse_sensivity;
	angley = angley + io.getMouse().dy*mouse_sensivity;
	yaw = yaw + anglex;
	pitch = pitch + angley;
	XMVECTOR newRotation = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.f);
	XMVECTOR RotationWithOutPitch = XMQuaternionRotationRollPitchYaw(0.f, yaw, 0.f);
	newRotation = XMQuaternionNormalize(newRotation);
	CPC->getCamPivot()->setRotation(newRotation);
	CPC->getPlayerPivot()->setRotation(RotationWithOutPitch);

	cam->lookAt(cam->getPosition(), CPC->getCamPivot()->getPosition(), XMVectorSet(0, 1, 0, 0));
	io.update(deltaTime);
}
