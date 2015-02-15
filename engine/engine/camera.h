#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#include "mcv_platform.h"

class CCamera {

  XMMATRIX       view;            // Where is and where is looking at
  XMMATRIX       projection;      // Prespective info
  XMMATRIX       view_projection; // 
  
  // View matrix info
  XMVECTOR       position;        // Current position
  XMVECTOR       front;           // Local Z in world coords
  XMVECTOR       up;              // Local up in world coords
  XMVECTOR       right;           // Local X in world coords
  XMVECTOR       target;          // Where we are looking at
  XMVECTOR       up_aux;          // Used while creating the view matrix

  // Projection information
  float          fov_in_radians;  // Field of view in radians
  float          aspect_ratio;
  float          znear, zfar;

  D3D11_VIEWPORT viewport;

  void updateViewProjection();

public:
  CCamera();

  // get'..
  XMMATRIX getView() const { return view; }
  XMMATRIX getProjection() const { return projection; }
  XMMATRIX getViewProjection() const { return view_projection; }
  XMVECTOR getPosition() const { return position; }
  XMVECTOR getFront() const { return front; }
  XMVECTOR getUp() const { return up; }
  XMVECTOR getRight() const { return right; }
  XMVECTOR getTarget() const { return target; }
  XMVECTOR getAuxUp() const { return up_aux; }

  float    getFov() const { return fov_in_radians; }
  float    getAspectRatio() const { return aspect_ratio; }
  float    getZNear() const { return znear; }
  float    getZFar() const { return zfar; }

  // 
  void lookAt(XMVECTOR new_eye, XMVECTOR new_target, XMVECTOR new_up_aux);
  void setPerspective(float new_fov_in_rad, float new_znear, float new_zfar);
  void setViewport(float x0, float y0, float xmax, float ymax);

  // helpers
  bool getScreenCoords(XMVECTOR world_coord, float *x, float *y) const;
  CEntity* getCamEntity();

private:
	CEntity* cam_entity;
};

#endif

