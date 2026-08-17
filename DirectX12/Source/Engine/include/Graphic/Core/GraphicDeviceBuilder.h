#pragma once
/*****************************************************************//**
 * @file   GraphicDeviceBuilder.h
 * @brief  動作環境に適したID3D12Deviceを作成するビルダーです。
 * 
 * @author h_ka1
 * @date   August 2026
 *********************************************************************/
#include <d3dx12.h>
using Microsoft::WRL::ComPtr;

/**
 * @brief 動作環境に適したID3D12Deviceを作成するビルダーです。
 */
class GraphicDeviceBuilder
{
public:
	/**
	 * @brief	最も性能の良いハードウェアアダプタを取得し、ID3D12Deviceを作成します。\n
	 *			DirectX12に対応していなければassertを発生させます。\n
	 *			デバッグモードではデバッグレイヤーを有効にし、誤検知エラーを無視するフィルタを適用させます。\n
	 * 
	 * @return	作製したComPtr<ID3D12Device>を返します。
	 */
	ComPtr<ID3D12Device> build();
};

