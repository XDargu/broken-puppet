#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#include "mcv_platform.h"

class CCamera {
protected:
	XMMATRIX       view;            // Where is and where is looking at
	XMMATRIX       projection;      // Prespective info
	XMMATRIX       view_projection;

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

  float    getFov() const { return fov_in_radians; }
  float    getAspectRatio() const { return aspect_ratio; }
  float    getZNear() const { return znear; }
  float    getZFar() const { return zfar; }

  // 
  void setPerspective(float new_fov_in_rad, float new_znear, float new_zfar);
  void setViewport(float x0, float y0, float xmax, float ymax);

  // helpers
  bool getScreenCoords(XMVECTOR world_coord, float *x, float *y) const;
};

#endif

