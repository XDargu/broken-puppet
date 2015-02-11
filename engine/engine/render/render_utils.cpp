#include "mcv_platform.h"
#include "render_utils.h"
#include "camera.h"
#include "render/render_utils.h"

using namespace DirectX;

#include "render/ctes/shader_ctes.h"
CShaderCte<TCtesObject> ctes_object;
CShaderCte<TCtesCamera> ctes_camera;
CMesh        wire_cube;

// -----------------------
template<>
CVertexDecl* getVertexDecl< CVertexPosColor >() {
  return &vdcl_position_color;
}

// --------------------------------------------
enum eSamplerType {
  SAMPLER_WRAP_LINEAR = 0
, SAMPLER_CLAMP_LINEAR
, SAMPLERS_COUNT
};

ID3D11SamplerState* all_samplers[SAMPLERS_COUNT];

bool createSamplers() {

  // Create the sample state
  D3D11_SAMPLER_DESC sampDesc;

  ZeroMemory(&sampDesc, sizeof(sampDesc));
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  HRESULT hr = ::render.device->CreateSamplerState(
    &sampDesc, &all_samplers[SAMPLER_WRAP_LINEAR]);
  if (FAILED(hr))
    return false;

  ZeroMemory(&sampDesc, sizeof(sampDesc));
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = ::render.device->CreateSamplerState(
    &sampDesc, &all_samplers[SAMPLER_CLAMP_LINEAR]);
  if (FAILED(hr))
    return false;

  return true;
}

void destroySamplers() {
  for (int i = 0; i < SAMPLERS_COUNT; ++i)
    SAFE_RELEASE(all_samplers[i]);
}

void activateTextureSamplers() {
  ::render.ctx->PSSetSamplers(0, SAMPLERS_COUNT, all_samplers);
}

// -----------------------------------------------
bool renderUtilsCreate() {
  bool is_ok = ctes_object.create();
  is_ok &= ctes_camera.create();
  is_ok &= createWiredCube(wire_cube);
  is_ok &= createSamplers();
  return is_ok;
}

void renderUtilsDestroy() {
  destroySamplers();
  ctes_object.destroy();
  ctes_camera.destroy();
  wire_cube.destroy();
}

void activateWorldMatrix( int slot ) {
  ctes_object.activateInVS(slot);
}

void activateCamera(const CCamera& camera, int slot) {
  ctes_camera.activateInVS(slot);    // as set in the shader.fx!!
  ctes_camera.get()->ViewProjection = camera.getViewProjection();
  ctes_camera.uploadToGPU();
}

// -----------------------------------------------------
bool createGrid(CMesh& mesh, int nsamples) {

  std::vector< CVertexPosColor > vtxs;

  XMFLOAT4 c2 = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
  XMFLOAT4 c1 = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

  float fsamples = (float)nsamples;
  CVertexPosColor v;

  // The lines from -x to +x
  for (int z = -nsamples; z <= nsamples; ++z) {
    v.Color = (z % 5) ? c2 : c1;
    v.Pos = XMFLOAT3(-fsamples, 0.f, (float)z);
    vtxs.push_back(v);
    v.Pos = XMFLOAT3(fsamples, 0.f, (float)z);
    vtxs.push_back(v);
  }  

  // The lines from -z to +z
  for (int x = -nsamples; x <= nsamples; ++x) {
    v.Color = (x % 5) ? c2 : c1;
    v.Pos = XMFLOAT3((float)x, 0.f, -fsamples);
    vtxs.push_back(v);
    v.Pos = XMFLOAT3((float)x, 0.f, fsamples);
    vtxs.push_back(v);
  }

  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createAxis(CMesh& mesh) {

  std::vector< CVertexPosColor > vtxs;
  vtxs.resize(6);
  CVertexPosColor *v = &vtxs[0];

  // Axis X
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(1.f, 0.f, 0.f); v->Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f); ++v;

  // Axis Y
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 2.f, 0.f); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;

  // Axis Z
  v->Pos = XMFLOAT3(0.f, 0.f, 0.f); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(0.f, 0.f, 3.f); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;

  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 0, nullptr, CMesh::LINE_LIST);
}

// -----------------------------------------------------
bool createCube(CMesh& mesh, float size) {

	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(8);
	CVertexPosColor *v = &vtxs[0];

	// Top
	v->Pos = XMFLOAT3(-1.0f * size, 1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, 1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, 1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-1.0f * size, 1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f); ++v;

	// Bottom
	v->Pos = XMFLOAT3(-1.0f * size, -1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, -1.0f * size, -1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(1.0f * size, -1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-1.0f * size, -1.0f * size, 1.0f * size); v->Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f); ++v;

	// Indices
	CMesh::TIndex indices[] =
	{
		3, 0, 1,
		2, 3, 1,

		0, 4, 5,
		1, 0, 5,

		3, 7, 4,
		0, 3, 4,

		1, 5, 6,
		2, 1, 6,

		2, 6, 7,
		3, 2, 7,

		6, 5, 4,
		7, 6, 4,
	};

	return mesh.create((unsigned)vtxs.size(), &vtxs[0], ARRAYSIZE(indices), indices, CMesh::TRIANGLE_LIST);
}

// -----------------------------------------------------
bool createWiredCube(CMesh& mesh) {

  std::vector< CVertexPosColor > vtxs;
  vtxs.resize(8);
  CVertexPosColor *v = &vtxs[0];

  // Set the color to white
  for (auto& it : vtxs)
    it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

  // Axis X
  v->Pos = XMFLOAT3(-1.f, -1.f, 0.f); ++v;
  v->Pos = XMFLOAT3( 1.f, -1.f, 0.f); ++v;
  v->Pos = XMFLOAT3(-1.f,  1.f, 0.f); ++v;
  v->Pos = XMFLOAT3( 1.f,  1.f, 0.f); ++v;
  v->Pos = XMFLOAT3(-1.f, -1.f, 1.f); ++v;
  v->Pos = XMFLOAT3( 1.f, -1.f, 1.f); ++v;
  v->Pos = XMFLOAT3(-1.f,  1.f, 1.f); ++v;
  v->Pos = XMFLOAT3( 1.f,  1.f, 1.f); ++v;

  const CMesh::TIndex idxs[] = {
    0, 1, 2, 3, 4, 5, 6, 7
    , 0, 2, 1, 3, 4, 6, 5, 7
    , 0, 4, 1, 5, 2, 6, 3, 7
  };
  return mesh.create((unsigned)vtxs.size(), &vtxs[0], 24, idxs, CMesh::LINE_LIST);
}

// -----------------------------------------------------

bool createCamera(CMesh& mesh)
{
	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(16);
	CVertexPosColor *v = &vtxs[0];

	// Set the color to white
	for (auto& it : vtxs)
		it.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

	// Top
	v->Pos = XMFLOAT3(-.25f, .25f, -1.0f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, .25f, -1.0f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, .25f, .7f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.25f, .25f, .7f); v->Color = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f); ++v;

	// Bottom
	v->Pos = XMFLOAT3(-.25f, -0.25f, -1.0f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, -0.25f, -1.0f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.25f, -0.25f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.25f, -0.25f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;

	// Camera front
	v->Pos = XMFLOAT3(.2f, 0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.2f, 0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.2f, -0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.2f, -0.2f, .7f); v->Color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); ++v;

	v->Pos = XMFLOAT3(.5f, .5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.5f, .5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(.5f, -0.5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;
	v->Pos = XMFLOAT3(-.5f, -0.5f, 1.f); v->Color = XMFLOAT4(0.f, 0.5f, 1.f, 1.f); ++v;


	const CMesh::TIndex idxs[] = {
		3, 0, 1,
		2, 3, 1,

		0, 4, 5,
		1, 0, 5,

		3, 7, 4,
		0, 3, 4,

		1, 5, 6,
		2, 1, 6,

		6, 5, 4,
		7, 6, 4,

		9, 7, 3,
		9, 11, 7,

		2, 8, 9,
		2, 9, 3,

		2, 6, 10,
		2, 10, 8,

		6, 7, 10,
		10, 7, 11,

		8, 10, 14,
		8, 14, 12,

		8, 12, 9,
		9, 12, 13,

		9, 13, 11,
		11, 13, 15,

		14, 10, 15,
		15, 10, 11,

		12, 14, 13,
		13, 14, 15

	};
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], ARRAYSIZE(idxs), idxs, CMesh::TRIANGLE_LIST);
}

// -----------------------------------------------------
void drawViewVolume(const CCamera& camera) {
  XMVECTOR det;
  for (int i = 0; i < 10; ++i) {
    float ratio = (i + 1) / 10.0f;
    XMMATRIX scale = XMMatrixScaling(1.f, 1.f, ratio);
    XMMATRIX inv_view_proj = XMMatrixInverse(&det, camera.getViewProjection());
    ctes_object.get()->World = scale * inv_view_proj;
    ctes_object.uploadToGPU();
    wire_cube.activateAndRender();
  }
}

void setWorldMatrix(XMMATRIX world) {
  ctes_object.get()->World = world;
  ctes_object.uploadToGPU();
}
