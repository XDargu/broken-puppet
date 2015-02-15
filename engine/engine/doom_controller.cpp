#include "mcv_platform.h"
#include "doom_controller.h"
#include "camera_pivot_controller.h"
#include "entity.h"
#include "iostatus.h"

using namespace DirectX;

float angle_x = 0.f;
float angle_y=0.f;

CThirdPersonController::CThirdPersonController()
: movement_velocity(5.0f)
, rotation_velocity(deg2rad( 90.f ))
{ }

void CThirdPersonController::update(CEntity* e, camera_pivot_controller* CPC, float delta_time) {

	assert(e != nullptr);
	assert(CPC->getPlayerPivot() != nullptr);
	XMVECTOR delta_pos = XMVectorZero();
	XMVECTOR delta_q = XMQuaternionIdentity();
	// Que teclas se pulsan -> que cambios hacer
	if (io.isPressed(io.DIGITAL_UP)){
		e->setRotation(CPC->getPlayerPivot()->getRotation());
		delta_pos += delta_time * movement_velocity * e->getFront();
	}
	else if (io.isPressed(io.DIGITAL_DOWN)){
		e->setRotation(CPC->getPlayerPivot()->getRotation());
		delta_pos -= delta_time * movement_velocity * e->getFront();
	}
	if (io.isPressed(io.DIGITAL_LEFT)){
		e->setRotation(CPC->getPlayerPivot()->getRotation());
		delta_pos += delta_time * movement_velocity * e->getLeft();
	}
	else if (io.isPressed(io.DIGITAL_RIGHT)){
		e->setRotation(CPC->getPlayerPivot()->getRotation());
		delta_pos -= delta_time * movement_velocity * e->getLeft();
	}
    e->setPosition( e->getPosition() + delta_pos );
    e->setRotation(XMQuaternionMultiply(e->getRotation(), delta_q));

	if (io.isPressed(io.MOUSE_MIDDLE)){
		XMVECTOR rotCamPivot = CPC->getCamPivot()->getRotation();
		XMVECTOR rotPlayerPivot = CPC->getPlayerPivot()->getRotation();
		CPC->getCamPivot()->setRotation(rotCamPivot*rotPlayerPivot);
	}
}

// -------------------------------
void CLookAtController::update(CEntity* who, CEntity* target, float delta_time) {
  // MatrixLookAt is used for the camera view matrix
  // converts world coords to camera local coords
  // so, we need to invert the matrix (or invert the quaternion)
  // because for the entity we want a local to world transform

  // target and who and inverted because cameras look at the Z negative axis
  // but the entities we want to look at the Z positive axis
  XMMATRIX m = XMMatrixLookAtRH(target->getPosition(), who->getPosition(), XMVectorSet(0, 1, 0, 0));
  XMVECTOR q = XMQuaternionRotationMatrix(m);
  who->setRotation(XMQuaternionInverse(q));
}

// -------------------------------
// Rotates around axis Y only!
CAimToController::CAimToController()
  : rotation_velocity(deg2rad(90.f))
{}

void CAimToController::update(CEntity* who, CEntity* target, float delta_time) {
  XMVECTOR delta = target->getPosition() - who->getPosition();
  delta = XMVectorSetY(delta, 0.f);
  XMVECTOR my_front = who->getFront();
  my_front = XMVectorSetY(my_front, 0.f);

  // Angle returned is always positive!
  float angle = XMVectorGetX( XMVector3AngleBetweenVectors(my_front, delta) );

  // If the target is on my right, rotate to the right 
  if (!who->isInLeft(target->getPosition()))
    angle = -angle;

  // The max angle I can rotate with my current speed
  float max_angle = rotation_velocity * delta_time;
  if (angle > max_angle)
    angle = max_angle;
  else if (angle < -max_angle)
    angle = -max_angle;

  // Delta rotation around world axis Y
  XMVECTOR q = XMQuaternionRotationAxis( XMVectorSet(0,1,0,0), angle);
  who->setRotation(XMQuaternionMultiply( who->getRotation(), q) );
}

