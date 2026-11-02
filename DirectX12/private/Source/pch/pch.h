#pragma once


///グラフィック
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3d11on12.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d2d1_3.h>
#include <dxgi1_6.h>
#include <dxgitype.h>
#include <dxgiformat.h>	
#include <dwrite.h>

//Windows API
#include <wrl/client.h>
#include <winuser.h>

//FBX SDK
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")
#include <fbxsdk.h>

//XAudio2
#pragma comment(lib, "xaudio2.lib")
#include <xaudio2.h>

//よく使うスタンダードライブラリ
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <fstream>
#include <memory>
#include <algorithm>
#include <string>
#include <cassert>
#include <json.hpp>
#include <random>
#include <deque>

