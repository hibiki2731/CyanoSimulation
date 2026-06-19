#pragma once

//Windows API
#include <wrl/client.h>
#include <winuser.h>

///グラフィック
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")
#include <DirectXMath.h>
#include "d3dx12.h"
#include <d3d11on12.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <dxgi1_6.h>
#include <dxgitype.h>
#include <dxgiformat.h>	
#include <dwrite.h>


//よく使うスタンダードライブラリ
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

