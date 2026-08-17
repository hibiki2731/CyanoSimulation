/*****************************************************************//**
 * @file   CommandQueueBuilder.h
 * @brief  CPUからGPUへ命令を送るコマンドキューのビルダーを記述しています。
 * 
 * @author h_ka1
 * @date   August 2026
 *********************************************************************/
#pragma once
#include <d3dx12.h>
using Microsoft::WRL::ComPtr;

/**
 * @brief	ID3D12CommandQueueを作成するビルダーです。
 */
class CommandQueueBuilder
{
public:
	/**
	 * @brief	デフォルトコンストラクタ。\n\n
	 *			コマンドリストタイプをDIRECT、フラグはなし、ノードマスクは0で初期化します。\n
	 * 
	 */
	CommandQueueBuilder()
		:mDesc(D3D12_COMMAND_QUEUE_DESC{
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAG_NONE,
			0 }
		) {}

	/**
	 * @brief			命令の種類を指定します。
	 * 
	 * @param[in] type	GPUへ送る命令の種類
	 * @return			ビルダー自身
	 */
	CommandQueueBuilder& setCommandListType(D3D12_COMMAND_LIST_TYPE& type);

	/**
	 * @brief			コマンドキューのフラグを指定します。
	 * 
	 * @param[in] flag	指定するフラグ
	 * @return			ビルダー自身
	 */
	CommandQueueBuilder& setFlag(D3D12_COMMAND_QUEUE_FLAGS& flag);

	/**
	 * @brief			使用する物理GPUのビットマスクを指定します。
	 * 
	 * @param nodeMask	使用する物理GPUのビットマスク
	 * @return			ビルダー自身
	 */
	CommandQueueBuilder& setNodeMask(UINT nodeMask);

	/**
	 * @brief				コマンドキューを作成します。\n\n
	 *						作成するコマンドキューに渡したいコマンドリストの命令の種類が一致するようにしてください。\n
	 * 
	 * @param[in] device	作成に使用するデバイス
	 * @return				作製したコマンドキュー
	 */
	ComPtr<ID3D12CommandQueue> build(ID3D12Device& device);

private:
	D3D12_COMMAND_QUEUE_DESC mDesc;
};

