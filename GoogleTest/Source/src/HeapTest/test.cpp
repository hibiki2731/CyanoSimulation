#pragma warning(disable: 26495)
#include "pch.h"
#include <Memory/UploadBuffer.h>
#include <vector>
#include "HeapTest/HeapTest.h"
#include "Graphic/Core/EngineResourceFactory.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"

namespace HeapTest {
	TEST_F(GraphicCoreTest, TestEmptyData) {
		EXPECT_DEATH(uploadBuffer->upload(nullptr, 0, sizeof(int)), "");
	}

	TEST_F(GraphicCoreTest, TestUploadData) {

		std::vector<int> data = { 1, 2, 3, 5 };
		uploadBuffer->upload(data.data(), 0, data.size() * sizeof(int));

		int* check = static_cast<int*>(uploadBuffer->getCPUResource());

		EXPECT_EQ(1, *check); ++check;
		EXPECT_EQ(2, *check); ++check;
		EXPECT_EQ(3, *check); ++check;
		EXPECT_EQ(5, *check); ++check;

	}

	TEST_F(GraphicCoreTest, TestCopyToDefaultBuffer) {
		std::vector<int> data = { 1,2,3,5 };
		uploadBuffer->upload(data.data(), 0, data.size() * sizeof(int));

		defaultBuffer->copyFromUploadBuffer(*uploadBuffer.get());
		readbackBuffer->read(*defaultBuffer.get());

		int* check = static_cast<int*>(readbackBuffer->getCPUResource());

		EXPECT_EQ(1, *check); ++check;
		EXPECT_EQ(2, *check); ++check;
		EXPECT_EQ(3, *check); ++check;
		EXPECT_EQ(5, *check); ++check;
	}

	TEST_F(GameSideTest, TestStructuredBuffer) {
		std::shared_ptr<IStructuredBuffer> structuredBuffer = mFactory->createStructuredBuffer(5, sizeof(int));

		std::vector<int> data = { 1,2,3,5 };
		//アップロードバッファが解放されないようにしなければならない！！
		structuredBuffer->upload(data.data(), sizeof(int) * data.size());

		EXPECT_TRUE(std::dynamic_pointer_cast<StructuredBuffer>(structuredBuffer)->getGPUResource());
	}

	TEST_F(GameSideTest, TestRWStructuredBuffer) {
		std::shared_ptr<IRWStructuredBuffer> rwBuffer = mFactory->createRWStructuredBuffer(5, sizeof(int));

		std::vector<int> data = { 1,2,3,5 };
		//アップロードバッファが解放されないようにしなければならない！！
		rwBuffer->upload(data.data(), sizeof(int) * data.size());

		int* check = static_cast<int*>(rwBuffer->read());
		EXPECT_EQ(1, *check); ++check;
		EXPECT_EQ(2, *check); ++check;
		EXPECT_EQ(3, *check); ++check;
		EXPECT_EQ(5, *check); ++check;



	}


}