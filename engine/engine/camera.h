#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#include "mcv_platform.h"

class CCamera {
protected:
	XMVECTOR	   front;			// Local Z in world coords
	XMVECTOR       up;              // Local up in world coords
	XMVECTOR       right;           // Local X in world coords

	XMVECTOR	   position;
	XMMATRIX       view;            // Where is and where is looking at
	XMMATRIX       projection;      // Prespective info
	XMMATRIX       view_projection;
	XMMATRIX       prev_view_projection;

	float          fov_in_radians;  // Field of view in radians
	float          aspect_ratio;
	float          znear, zfar;

  D3D11_VIEWPORT viewport;

public:
  CCamera();

  // get'..
  XMMATRIX getView() const { return view; }
  XMMATRIX getProjection() const { return projection; }
  XMMATRIX getViewProjection() const { return view_projection; }
  XMMATRIX getPrevViewProjection() const { return prev_view_projection; }

  XMVECTOR getPosition() const { return position; }
  XMVECTOR getFront() const { return front; }
  XMVECTOR getUp() const { return up; }
  XMVECTOR getRight() const { return right; }

  float    getFov() const { return fov_in_radians; }
  float    getAspectRatio() const { return aspect_ratio; }
  float    getZNear() const { return znear; }
  float    getZFar() const { return zfar; }

  // 
  void setPerspective(float new_fov_in_rad, float new_znear, float new_zfar);
  void setViewport(float x0, float y0, float xmax, float ymax);
  D3D11_VIEWPORT getViewport() const { return viewport; }

  // helpers
  XMVECTOR CCamera::getWorldCoords(float norm_x, float norm_y);
  bool getScreenCoords(XMVECTOR world_coord, float *x, float *y) const;

  void setOcclusionProjection(float width, float height, float znear, float zfar);
  void setOcclusionViewport(float x0, float y0, float width, float height, float new_znear, float new_zfar);
  void setOcclusionViewProjection(XMVECTOR eye_position, XMVECTOR target_position, XMVECTOR the_front);
};

#endif

