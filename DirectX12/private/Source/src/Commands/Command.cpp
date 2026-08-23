#include "pch.h"
#include "Command/Command.h"
#include "Command/CommandListBuilder.h"
#include "Command/CommandQueueBuilder.h"
#include "Command/CommandAllocatorBuilder.h"

Command::Command(ID3D12Device& device, UINT commandNum)
{
	//コマンドアロケータ作成 (GPU、CPUの非同期処理のためにフレーム数分確保)
	for (UINT i = 0; i < commandNum; i++) {
		mCommandAllocators.push_back(CommandAllocatorBuilder().build(device));
	}

	//コマンドリスト作成
	mCommandList = CommandListBuilder().
		setCommandAllocator(mCommandAllocators[0]).
		build(device);

	//コマンドキュー作成
	mCommandQueue = CommandQueueBuilder().build(device);
}
