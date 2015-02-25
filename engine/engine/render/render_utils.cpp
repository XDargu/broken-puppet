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

void activateTint(int slot) {
	ctes_object.activateInPS(slot);
}

void activateCamera(const CCamera& camera, int slot) {
  ctes_camera.activateInVS(slot);    // as set in the shader.fx!!
  ctes_camera.get()->ViewProjection = camera.getViewProjection();  
  ctes_camera.uploadToGPU();
}

void activateCamera(const XMMATRIX viewProjection, const XMVECTOR position, int slot) {
	ctes_camera.activateInVS(slot);    // as set in the shader.fx!!
	ctes_camera.activateInPS(slot);    // as set in the shader.fx!!
	ctes_camera.get()->ViewProjection = viewProjection;
	ctes_camera.get()->CameraPosition = position;
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
bool createUnitWiredCube(CMesh& mesh, XMFLOAT4 color) {

	std::vector< CVertexPosColor > vtxs;
	vtxs.resize(8);
	CVertexPosColor *v = &vtxs[0];

	// Set the color to white
	for (auto& it : vtxs)
		it.Color = color;

	// Axis X
	v->Pos = XMFLOAT3(-.5f, -.5f, -.5f); ++v;
	v->Pos = XMFLOAT3(.5f, -.5f, -.5f); ++v;
	v->Pos = XMFLOAT3(-.5f, .5f, -.5f); ++v;
	v->Pos = XMFLOAT3(.5f, .5f, -.5f); ++v;
	v->Pos = XMFLOAT3(-.5f, -.5f, .5f); ++v;
	v->Pos = XMFLOAT3(.5f, -.5f, .5f); ++v;
	v->Pos = XMFLOAT3(-.5f, .5f, .5f); ++v;
	v->Pos = XMFLOAT3(.5f, .5f, .5f); ++v;

	const CMesh::TIndex idxs[] = {
		0, 1, 2, 3, 4, 5, 6, 7
		, 0, 2, 1, 3, 4, 6, 5, 7
		, 0, 4, 1, 5, 2, 6, 3, 7
	};
	return mesh.create((unsigned)vtxs.size(), &vtxs[0], 24, idxs, CMesh::LINE_LIST);
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
bool createString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension)
{
	float dist = XMVectorGetX(XMVector3Length(initialPos - finalPos));

	const int epsilon = 25;
	CVertexPosColor ropeVertices[epsilon];
	float y = 0;
	float pow_r = 2;	// Suavidad de la onda
	int velocity = 10;	// Velocidad de movimiento
	float wave_freq = 0.9f;	// frecuencia del ruido de la onda
	float amplitude = 0.05f;	// amplitud del ruido de la onda
	float elapsed = 1 / 60.0f;

	for (int i = 0; i < epsilon; i++)
	{
		XMVECTOR midPos = XMVectorLerp(initialPos, finalPos, (float)i / (epsilon - 1));

		if (i < epsilon / 2)
			y = pow(epsilon - i, pow_r) / pow(epsilon, pow_r);
		else
			y = pow(i, pow_r) / pow(epsilon, pow_r);

		float noise = (sin(elapsed*velocity + i*wave_freq) + 1) * amplitude;
		if (i != 0 && i != epsilon - 1)
			y += noise * noise;

		y -= 1;

		y *= tension;

		ropeVertices[i].Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		ropeVertices[i].Pos = XMFLOAT3(XMVectorGetX(midPos), XMVectorGetY(midPos) + y, XMVectorGetZ(midPos));
	}

	ropeVertices[0].Pos = XMFLOAT3(XMVectorGetX(initialPos), XMVectorGetY(initialPos), XMVectorGetZ(initialPos));;
	ropeVertices[epsilon - 1].Pos = XMFLOAT3(XMVectorGetX(finalPos), XMVectorGetY(finalPos), XMVectorGetZ(finalPos));;

	XMFLOAT3 a = ropeVertices[epsilon - 1].Pos;	

	return mesh.create(epsilon, ropeVertices, 0, NULL, CMesh::LINE_LIST_ADJ, &vdcl_position_color);
}

bool createFullString(CMesh& mesh, XMVECTOR initialPos, XMVECTOR finalPos, float tension)
{
	float dist = XMVectorGetX(XMVector3Length(initialPos - finalPos));

	const int epsilon = 25;
	const int sizes = 6;
	float width = 0.03;
	XMFLOAT3 ropeReferences[epsilon];
	CVertexPosUVNormal ropeVertices[epsilon * sizes];
	CMesh::TIndex ropeIndices[(epsilon - 1) * sizes * sizes];
	float y = 0;
	float pow_r = 2;	// Suavidad de la onda
	int velocity = 10;	// Velocidad de movimiento
	float wave_freq = 0.9f;	// frecuencia del ruido de la onda
	float amplitude = 0.05f;	// amplitud del ruido de la onda
	float elapsed = 1 / 60.0f;

	for (int i = 0; i < epsilon; i++)
	{
		XMVECTOR midPos = XMVectorLerp(initialPos, finalPos, (float)i / (epsilon - 1));

		if (i < epsilon / 2)
			y = pow(epsilon - i, pow_r) / pow(epsilon, pow_r);
		else
			y = pow(i, pow_r) / pow(epsilon, pow_r);

		float noise = (sin(elapsed*velocity + i*wave_freq) + 1) * amplitude;
		if (i != 0 && i != epsilon - 1)
			y += noise * noise;

		y -= 1;

		y *= tension;
		// Guardar la referencia de posición de la cuerda
		ropeReferences[i] = XMFLOAT3(XMVectorGetX(midPos), XMVectorGetY(midPos) + y, XMVectorGetZ(midPos));
	}

	ropeReferences[0] = XMFLOAT3(XMVectorGetX(initialPos), XMVectorGetY(initialPos), XMVectorGetZ(initialPos));;
	ropeReferences[epsilon - 1] = XMFLOAT3(XMVectorGetX(finalPos), XMVectorGetY(finalPos), XMVectorGetZ(finalPos));;

	// Ángulo que hay entre cada lado
	const float angle = deg2rad( 360 / sizes);
	XMVECTOR normal;
	XMVECTOR dir;
	for (int i = 0; i < epsilon; i++)
	{
		// Calcular la normal del punto de referencia
		// Caso inicial: la normal es el producto vectorial de la dirección del segundo al primero por el vector UP
		if (i == 0) {
			dir = XMVectorSet(
				ropeReferences[i + 1].x
				, ropeReferences[i + 1].y
				, ropeReferences[i + 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir = XMVector3Normalize(dir);
			normal = XMVector3Cross(dir, XMVectorSet(0, 1, 0, 0));
		}
		// Caso final: la normal es el producto vectorial de la dirección del penúltimo al último por el vector UP
		else if (i == epsilon - 1) {
			dir = XMVectorSet(
				ropeReferences[i - 1].x
				, ropeReferences[i - 1].y
				, ropeReferences[i - 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir = XMVector3Normalize(dir);
			normal = XMVector3Cross(dir, XMVectorSet(0, 1, 0, 0));
		}
		// Resto de casos: la normal es la media del anterior y el siguiente
		else
		{
			XMVECTOR dir1 = XMVectorSet(
				ropeReferences[i - 1].x
				, ropeReferences[i - 1].y
				, ropeReferences[i - 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir1 = XMVector3Normalize(dir1);
			XMVECTOR normal1 = XMVector3Cross(dir1, XMVectorSet(0, 1, 0, 0));
			XMVECTOR dir2 = XMVectorSet(
				ropeReferences[i - 1].x
				, ropeReferences[i - 1].y
				, ropeReferences[i - 1].z, 1
				)
				- XMVectorSet(
				ropeReferences[i].x
				, ropeReferences[i].y
				, ropeReferences[i].z, 1
				);
			dir2 = XMVector3Normalize(dir2);
			XMVECTOR normal2 = XMVector3Cross(dir2, XMVectorSet(0, 1, 0, 0));
			dir = (dir1 + dir2) / 2;
			normal = (normal1 + normal2) / 2;
		}
		// Crear los vértices extra, en forma de hexágono
		for (int j = 0; j < sizes; j++) {
			// Posiciones extra, siguiendo un círculo
			// Se rota la normal en el eje de dirección
			XMVECTOR quat = XMQuaternionRotationAxis(dir, angle * j);
			XMVECTOR pos = normal * width;
			pos = XMVector3Rotate(pos, quat);

			ropeVertices[i * sizes + j].Pos = XMFLOAT3(ropeReferences[i].x + XMVectorGetX(pos), ropeReferences[i].y + XMVectorGetY(pos), ropeReferences[i].z + XMVectorGetZ(pos));
			float uvy = i % 2 ? 0 : 1;
			float uvx = (1.0f / sizes) * j;
			ropeVertices[i * sizes + j].UV = XMFLOAT2(uvx, uvy);
			XMVECTOR realNormal = XMVector3Normalize(pos);
			ropeVertices[i * sizes + j].Normal = XMFLOAT3(XMVectorGetX(realNormal), XMVectorGetY(realNormal), XMVectorGetZ(realNormal));
		}		
	}

	// Índices
	int aaa = ARRAYSIZE(ropeIndices);
	int offset = 0;
	int inner_offset = 0;
	for (int i = 0; i < epsilon - 1; i++) {
		for (int j = 0; j < sizes; j++) {
			offset = i * sizes * sizes + j * sizes;
			inner_offset = i * sizes;
			// Último caso (cierre)
			if (j == sizes - 1) {
				ropeIndices[offset + 0] = inner_offset + j;
				ropeIndices[offset + 1] = inner_offset + j + (sizes + 1) - sizes;
				ropeIndices[offset + 2] = inner_offset + j + sizes;

				ropeIndices[offset + 3] = inner_offset + j;
				ropeIndices[offset + 4] = inner_offset + j + 1 - sizes;
				ropeIndices[offset + 5] = inner_offset + j + (sizes + 1) - sizes;
			}
			else {
				ropeIndices[offset + 0] = inner_offset + j;
				ropeIndices[offset + 1] = inner_offset + j + (sizes + 1);
				ropeIndices[offset + 2] = inner_offset + j + sizes;

				ropeIndices[offset + 3] = inner_offset + j;
				ropeIndices[offset + 4] = inner_offset + j + 1;
				ropeIndices[offset + 5] = inner_offset + j + (sizes + 1);
			}
		}
	}

	XMFLOAT3 a = ropeVertices[epsilon - 1].Pos;

	return mesh.create(epsilon * sizes, ropeVertices, (epsilon - 1) * sizes * sizes, ropeIndices, CMesh::TRIANGLE_LIST, &vdcl_position_uv_normal);
	//return mesh.create(epsilon * sizes, ropeVertices, 0, NULL, CMesh::LINE_LIST_ADJ, &vdcl_position_color);
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

void setTint(XMVECTOR tint) {
	ctes_object.get()->Tint = tint;
	ctes_object.uploadToGPU();
}