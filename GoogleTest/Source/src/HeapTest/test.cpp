#pragma warning(disable: 26495)
#include "pch.h"
#include <Memory/UploadHeap.h>
#include <vector>
#include "HeapTest/HeapTest.h"

namespace HeapTest {
	TEST_F(GraphicCoreTest, TestEmptyData) {
		UploadHeap uploadHeap(nullptr, nullptr);

		uploadHeap.uploadData(nullptr, 1);

		EXPECT_EQ(nullptr, uploadHeap.getGPUResource());
	}

	TEST_F(GraphicCoreTest, TestUploadData) {
		UploadHeap uploadHeap(mDevice.Get(), mCommandList.Get());

		std::vector<int> data = { 1, 2, 3, 5 };
		uploadHeap.uploadData(data.data(), data.size() * sizeof(int));

	}

}