#ifndef INC_RENDER_UTILS_H_
#define INC_RENDER_UTILS_H_

#include "mcv_platform.h"
#include "ctes\shader_ctes.h"

class CCamera;
struct TCompPointLight;
struct TCompShadows;
class VPlanes;

struct CVertexPos {
	XMFLOAT3 Pos;
};

struct CVertexPosColor {
  XMFLOAT3 Pos;
  XMFLOAT4 Color;
}; 

struct CVertexPosUVNormal {
  XMFLOAT3 Pos;
  XMFLOAT2 UV;
  XMFLOAT3 Normal;
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
bool createViewVolume(CMesh& mesh);
bool createUnitWiredCube(CMesh& mesh, XMFLOAT4 color);
bool createTexturedQuadXY(CMesh& mesh);
bool createIcosahedron(CMesh& mesh);
bool create3x3Plane(CMesh& mesh);

bool createCube(CMesh& mesh, float size);
bool createCamera(CMesh& mesh);
bool createString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension);
bool createFullString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension, float width);

void drawViewVolume(const CCamera& camera);
void drawLine(XMVECTOR src, XMVECTOR target);
void drawTexture2D(int x0, int y0, int w, int h, const CTexture* texture, const char* tech_name = nullptr);
void drawTexture3D(CCamera& camera, XMVECTOR world_p3d, int w, int h, const CTexture* texture, const char* tech_name = nullptr);
void drawTexture3DDynamic(CCamera& camera, XMVECTOR world_p3d, int w, int h, const CTexture* texture, const char* tech_name = nullptr);
void drawDialogBox(int x0, int y0, int w, int h, const CTexture* texture, const char* tech_name = nullptr);
void drawDialogBox3D(CCamera& camera, XMVECTOR world_p3d, int w, int h, const CTexture* texture, const char* tech_name = nullptr);
void drawDialogBox3DDynamic(CCamera& camera, XMVECTOR world_p3d, int w, int h, const CTexture* texture, const char* tech_name = nullptr);
void drawPlane(XMVECTOR plane);
void drawVPlanes(const VPlanes& planes);

// Render world
void setWorldMatrix(XMMATRIX world);
void activateWorldMatrix(int slot);
void setCinematicBands(float band);
void activateCamera(const CCamera& camera, int slot);
void activateLight(const CCamera& camera, int slot);
void activatePointLight(const TCompPointLight* plight, XMVECTOR light_pos, int slot);
void activateDirLight(const TCompShadows* dir_light, XMVECTOR light_pos, XMVECTOR light_dir, float angle, int slot);

// Provisional
void setTransformType(float type);
void setTint(XMVECTOR tint);
void activateTint(int slot);

void activateTextureSamplers();

// ---------------------------------------
enum ZConfig {
	ZCFG_DEFAULT
	, ZCFG_DISABLE_ALL
	, ZCFG_TEST_BUT_NO_WRITE
	, ZCFG_INVERSE_TEST_NO_WRITE
	, ZCFG_COUNT
};
void activateZConfig(enum ZConfig cfg);

// ---------------------------------------
enum RSConfig {
	RSCFG_DEFAULT
	, RSCFG_REVERSE_CULLING
	, RSCFG_SHADOWS
	, RSCFG_COUNT
};
void activateRSConfig(enum RSConfig cfg);

// ---------------------------------------
enum BlendConfig {
	BLEND_CFG_DEFAULT
	, BLEND_CFG_BY_SRC_ALPHA
	, BLEND_CFG_ADDITIVE
	, BLEND_CFG_ADDITIVE_BY_SRC_ALPHA
	, BLEND_CFG_COMBINATIVE
	, BLEND_CFG_COMBINATIVE_BY_SRC_ALPHA
	, BLEND_CFG_COUNT
};
void activateBlendConfig(enum BlendConfig cfg);

// ---------------------------------------
// Shared meshes
extern CMesh        grid;
extern CMesh        axis;
extern CMesh        mesh_icosahedron;
extern CMesh        mesh_view_volume;
extern CMesh        mesh_textured_quad_xy_centered;
extern CMesh        plane3x3;

// Post process
extern  CShaderCte<TCtesGlobal> ctes_global;
extern	CShaderCte<TCtesBlur> ctes_blur;
extern	CShaderCte<TCtesBlurCamera> ctes_blur_camera;
extern	CShaderCte<TCtesSharpen> ctes_sharpen;
extern	CShaderCte<TCtesSSAO> ctes_ssao;
extern  CShaderCte<TCtesChromaticAberration> ctes_chromatic_aberration;
extern  CShaderCte<TCtesGlow> ctes_glow;
extern  CShaderCte<TCtesUnderwater> ctes_underwater;
extern  CShaderCte<TCtesSSRR> ctes_ssrr;

#endif
