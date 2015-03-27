#ifndef INC_MCV_PLATFORM_H_
#define INC_MCV_PLATFORM_H_

#define _WIN32_WINNT  _WIN32_WINNT_WIN7

#include "targetver.h"

#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdint.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

// C++
#include <map>
#include <vector>

#include <DirectXMath.h>

/*using DirectX::XMMATRIX;
using DirectX::XMVECTOR;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;*/

using namespace DirectX;

#include "app.h"
#include "render/render.h"
#include "utils.h"
#include "memory/data_provider.h"
#include "memory/data_saver.h"
#include "items_by_name.h"
#include "error/error_context.h"
#include <queue>

#endif
