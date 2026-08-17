/*****************************************************************//**
 * @file   CommandAllocatorBuilder.h
 * @brief  ID3D12CommandAllocatorを作成するビルダーのクラスを記述。
 * 
 * @author h_ka1
 * @date   August 2026
 *********************************************************************/
#pragma once
#include <d3dx12.h>
using Microsoft::WRL::ComPtr;

/**
 * @brief	ID3D12CommandAllocatorを作成するビルダーです。
 */
class CommandAllocatorBuilder
{
public:
	/**
	 * @brief	デフォルトコンストラクタです。\n
	 *			D3D12_COMMAND_LIST_TYPEをDIRECTで初期化します。
	 */
	CommandAllocatorBuilder() :
		mType(D3D12_COMMAND_LIST_TYPE_DIRECT) {}

	/**
	 * @brief			作成するコマンドアロケータがどの種類のコマンドを格納するのか設定します。
	 * 
	 * @param[in] type	格納したいコマンドの種類
	 * @return			ビルダー自身を返す。
	 */
	CommandAllocatorBuilder&		setCommandListType(D3D12_COMMAND_LIST_TYPE& type);

	/**
	 * @brief				コマンドアロケータを作成し、返します。
	 * 
	 * @param[in] device	コマンドアロケータを作成するデバイス
	 * @return				作成したコマンドアロケータ
	 */
	ComPtr<ID3D12CommandAllocator>	build(ID3D12Device& device);

private:
	D3D12_COMMAND_LIST_TYPE mType;
};

