#pragma once
#include <memory>
#include "gtest/gtest.h"
#include <d3dx12.h>
#include <DirectXMath.h>
#include "Memory/UploadBuffer.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Memory/ReadBackBuffer.h"
#include "Builder/GraphicDeviceBuilder.h"
#include "Builder/EngineResourceFactory.h"
#include "Command/Command.h"
#include "GameLoopCore/Game.h"

class GraphicCoreTest : public ::testing::Test {
	friend class UploadBuffer;
protected:
	virtual void SetUp() {
		mDevice = GraphicDeviceBuilder().build();
		mCommandManager = std::make_unique<CommandManager>(*mDevice.Get(), 1);

		uploadBuffer = std::make_unique<UploadBuffer>(*mDevice.Get(), sizeof(int) * 8);
		defaultBuffer = std::make_unique<LinearDefaultBuffer>(*mDevice.Get(), *mCommandManager.get(), sizeof(int) * 8, D3D12_RESOURCE_STATE_COMMON);
		readbackBuffer = std::make_unique<ReadBackBuffer>(*mDevice.Get(), *mCommandManager.get(), sizeof(int) * 8);
	}

	ComPtr<ID3D12Device> mDevice;
	std::unique_ptr<CommandManager> mCommandManager;

	std::unique_ptr<UploadBuffer> uploadBuffer;
	std::unique_ptr<LinearDefaultBuffer> defaultBuffer;
	std::unique_ptr<ReadBackBuffer> readbackBuffer;
	


};

class GameSideTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		::testing::GTEST_FLAG(break_on_failure) = true;
		mGame = std::make_unique<Game>();
		mGame->init();

		mFactory = std::make_unique<EngineResourceFactory>(mGame->createFactory());
	}

	std::unique_ptr<Game> mGame;
	std::unique_ptr<EngineResourceFactory> mFactory;

};
