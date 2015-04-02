#ifndef INC_RENDER_UTILS_H_
#define INC_RENDER_UTILS_H_

#include "mcv_platform.h"

class CCamera;

struct CVertexPosColor {
  XMFLOAT3 Pos;
  XMFLOAT4 Color;
}; 

struct CVertexPosUVNormal {
  XMFLOAT3 Pos;
  XMFLOAT2 UV;
  XMFLOAT3 Normal;
};

bool renderUtilsCreate();
void renderUtilsDestroy();

bool createGrid(CMesh& mesh, int nsamples);
bool createAxis(CMesh& mesh);
bool createWiredCube(CMesh& mesh);
bool createUnitWiredCube(CMesh& mesh, XMFLOAT4 color);

bool createCube(CMesh& mesh, float size);
bool createCamera(CMesh& mesh);
bool createString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension);
bool createFullString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension, float width);

void drawViewVolume(const CCamera& camera);
void drawLine(XMVECTOR src, XMVECTOR target);

// Render world
void setWorldMatrix(XMMATRIX world);
void activateWorldMatrix(int slot);
void activateCamera(const XMVECTOR position, int slot);
void activateCamera(const CCamera* camera, int slot);

// Provisional
void setTint(XMVECTOR tint);
void activateTint(int slot);

void activateTextureSamplers();

#endif
