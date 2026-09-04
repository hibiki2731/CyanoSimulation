#pragma once
#include <d3d12.h>
#include "Compute/ComputeShader.h"
#include "Builder/GraphicDeviceBuilder.h"
#include "Builder/EngineResourceFactory.h"
#include "GameLoopCore/Game.h"

class GameSideComputeTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		::testing::GTEST_FLAG(break_on_failure) = true;
	}

	std::unique_ptr<Game> mGame;
	std::unique_ptr<EngineResourceFactory> mFactory;
	std::shared_ptr<ComputeShader> mComputeShader;

};
