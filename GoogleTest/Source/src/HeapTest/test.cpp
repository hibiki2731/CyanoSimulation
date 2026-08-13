#include "pch.h"
#include <Memory/UploadHeap.h>

namespace HeapTest {
	TEST(UploadHeapTest, TestEmptyData) {
		UploadHeap uploadHeap(nullptr, nullptr);

		uploadHeap.uploadData(nullptr, 1);

		EXPECT_EQ(nullptr, uploadHeap.getGPUResource());
	}
}