#include "mcv_platform.h"
#include "shaders.h"

// ----------------------------------------------------
CVertexShader::CVertexShader() : vs(nullptr), vtx_layout(nullptr) {}

bool CVertexShader::compile(
    const char* szFileName
  , const char* szEntryPoint
  , const CVertexDecl &decl) {

  // A blob is a representation of a buffer in dx, a void pointer + size
  ID3DBlob* pVSBlob = NULL;
  bool is_ok = render.compileShaderFromFile(szFileName, szEntryPoint
                , "vs_4_0", &pVSBlob);
  if (!is_ok)
    return false;

  // Create the vertex shader
  HRESULT hr;
  hr = render.device->CreateVertexShader(
                         pVSBlob->GetBufferPointer()
                       , pVSBlob->GetBufferSize()
                       , NULL
                       , &vs);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return false;
  }

  // Create the input layout
  hr = render.device->CreateInputLayout(decl.elems, decl.nelems, pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), &vtx_layout);
  pVSBlob->Release();
  if (FAILED(hr))
    return false;

  return true;
}

void CVertexShader::destroy() {
  SAFE_RELEASE(vs);
  SAFE_RELEASE(vtx_layout);
}

void CVertexShader::activate() const {
  assert(vs);
  render.ctx->VSSetShader( vs, NULL, 0);

  // Set the input layout
  render.ctx->IASetInputLayout( vtx_layout );
}


// ----------------------------------------------------
CPixelShader::CPixelShader() : ps(nullptr) {}

bool CPixelShader::compile(const char* szFileName, const char* szEntryPoint) {

  ID3DBlob* pVSBlob = NULL;
  bool is_ok = render.compileShaderFromFile(szFileName, szEntryPoint, "ps_4_0", &pVSBlob);
  if (!is_ok)
    return false;

  // Create the vertex shader
  HRESULT hr;
  hr = render.device->CreatePixelShader(
    pVSBlob->GetBufferPointer()
    , pVSBlob->GetBufferSize()
    , NULL
    , &ps);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return false;
  }

  pVSBlob->Release();
  return true;
}

void CPixelShader::destroy() {
  SAFE_RELEASE(ps);
}


void CPixelShader::activate() const {
  assert(ps);
  render.ctx->PSSetShader(ps, NULL, 0);
}

