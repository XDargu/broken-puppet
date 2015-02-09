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
bool createCube(CMesh& mesh, float size);
bool createWiredCube(CMesh& mesh);

void drawViewVolume(const CCamera& camera);

// Render world
void setWorldMatrix(XMMATRIX world);
void activateWorldMatrix(int slot);
void activateCamera(const CCamera& camera, int slot);

void activateTextureSamplers();

#endif
