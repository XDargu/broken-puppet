#ifndef INC_RENDER_UTILS_H_
#define INC_RENDER_UTILS_H_

#include "mcv_platform.h"

class CCamera;

struct CVertexPosColor {
  XMFLOAT3 Pos;
  XMFLOAT4 Color;
}; 

struct CVertexPosUV {
  XMFLOAT3 Pos;
  XMFLOAT2 UV;
};

bool renderUtilsCreate();
void renderUtilsDestroy();

bool createGrid(CMesh& mesh, int nsamples);
bool createAxis(CMesh& mesh);
bool createWiredCube(CMesh& mesh);
bool createUnitWiredCube(CMesh& mesh, XMFLOAT4 color);

bool createCube(CMesh& mesh, float size);
bool createCamera(CMesh& mesh);

void drawViewVolume(const CCamera& camera);

// Render world
void setWorldMatrix(XMMATRIX world);
void activateWorldMatrix(int slot);
void activateCamera(const XMMATRIX viewProjection, const XMVECTOR position, int slot);

// Provisional
void activateCamera(const XMMATRIX viewProjection, int slot);
void setTint(XMVECTOR tint);
void activateTint(int slot);

void activateTextureSamplers();

#endif
