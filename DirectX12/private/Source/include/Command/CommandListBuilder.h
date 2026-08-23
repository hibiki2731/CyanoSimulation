/*****************************************************************//**
 * @file   CommandListBuilder.h
 * @brief  ID3D12GraphicsCommandListを作成するビルダーを記述しています。
 * 
 * @author h_ka1
 * @date   August 2026
 *********************************************************************/
#pragma once
#include <d3dx12.h>
using Microsoft::WRL::ComPtr;

/**
 * @brief	ID3D12GraphicsCommandListを作成するビルダーです。
 */
class CommandListBuilder
{
public:
	/**
	 * @brief	デフォルトコンストラクタ。\n
	 *			ノードマスクは0、コマンドリストタイプはDIRECT、初期PSOはnullptrで初期化します。\n
	 */
	CommandListBuilder() :
		mNodeMask(0),
		mType(D3D12_COMMAND_LIST_TYPE_DIRECT),
		mCommandAllocator(nullptr),
		mPSO(nullptr) {}

	/**
	 * @brief				どの物理GPUに対してコマンドを記録するかを指定します。
	 * @param[in] nodeMask	使用する物理GPUのビットマスク
	 * @return				ビルダー自身
	 */
	CommandListBuilder& setNodeMask(UINT nodeMask);

	/**
	 * @brief				どのような命令を記録するかを指定します。
	 * 
	 * @param[in] type		記録する命令の種類		
	 * @return				ビルダー自身 
	 */
	CommandListBuilder& setCommandListType(D3D12_COMMAND_LIST_TYPE& type);

	/**
	 * @brief						コマンドリストが命令を記録するメモリを割り当てるコマンドアロケータを指定します。\n
	 *								コマンドアロケータを指定しなければ、build()関数でアサーションが発生します。\n
	 *								コマンドアロケータとコマンドリストのコマンドリストタイプが違うとbuild()でアサーションが発生します。\n
	 * 
	 * @param[in] commandAllocator	コマンドリストの命令を記録するメモリを割り当てるコマンドアロケータ
	 * @return						ビルダー自身
	 */
	CommandListBuilder& setCommandAllocator(ComPtr<ID3D12CommandAllocator>& commandAllocator);

	/**
	 * @brief			コマンドリストの記録開始時にデフォルトでセットしておくPSOを指定します。
	 * 
	 * @param[in] pso	デフォルトでセットするPSO
	 * @return			ビルダー自身
	 */
	CommandListBuilder& setInitialPSO(ComPtr<ID3D12PipelineState>& pso);

	/**
	 * @brief				コマンドリストを作成します。\n
	 *						コマンドアロケータを指定していなければアサーションが発生します。\n
	 *						コマンドアロケータとコマンドリストのD3D12_COMMAND_LIST_TYPEが違うとアサーションが発生します。\n
	 * 
	 * @param[in] device	コマンドリストを作成するためのデバイス
	 * @return				作成したコマンドリスト
	 */
	ComPtr<ID3D12GraphicsCommandList> build(ID3D12Device& device);


private:
	UINT mNodeMask;
	D3D12_COMMAND_LIST_TYPE mType;
	ID3D12CommandAllocator* mCommandAllocator;
	ID3D12PipelineState* mPSO;
};

