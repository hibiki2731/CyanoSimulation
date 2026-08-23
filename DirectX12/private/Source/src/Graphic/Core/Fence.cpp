
#include "Graphic/Core/Fence.h"

Fence::Fence(ID3D12Device& device, ID3D12CommandQueue& commandQueue, const int frameCount)
	:mCommandQueue(commandQueue)
{
	prepareFence(device, frameCount);
}

void Fence::waitGPU()
{
	//現在のFence値がコマンド中菱後にFenceに書き込まれるようにス
	UINT64 fvalue = mFenceValue;
	mCommandQueue.Signal(mFence.Get(), fvalue);
	mFenceValue++;

	//まだコマンドキューが終了していないことを確認する
	if (mFence->GetCompletedValue() < fvalue) {
		//このFenceにおいて、fvalueの値になったらイベントを発生させる
		mFence->SetEventOnCompletion(fvalue, mFenceEvent);
		//イベントが発生するまで待つ
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

void Fence::waitCompleteNextFrame(const int currentFrame, const int nextFrame)
{
	//現フレームのフェンス値を記録
	mFrameFenceValues[currentFrame] = mFenceValue;
	mCommandQueue.Signal(mFence.Get(), mFenceValue); //GPUの描画が終わったらmFenceValueを出力
	mFenceValue++;

	//次フレームのバッファをGPUがまだ使っていれば待機
	if (mFence->GetCompletedValue() < mFrameFenceValues[nextFrame]) {
		//前のフレームのフェンス値になるまで待つ
		mFence->SetEventOnCompletion(mFrameFenceValues[nextFrame], mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

void Fence::prepareFence(ID3D12Device& device, const int frameCount)
{
	//GPUの処理完了をチェックするフェンスを作る
	HRESULT hr = device.CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetAddressOf()));
	mFenceValue = 1;
	mFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	assert(SUCCEEDED(hr));
	mFrameFenceValues.resize(frameCount);
}
