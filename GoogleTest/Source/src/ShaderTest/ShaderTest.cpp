#include "ShaderTest/ShaderTest.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"


namespace ShaderTest {

	TEST_F(GameSideTest, TestSetStructuredBufferWhenToView) {

		std::shared_ptr<IStructuredBuffer> buffer = mFactory->createStructuredBuffer(4, sizeof(int));
		std::shared_ptr<IRWStructuredBuffer> rwBuffer = mFactory->createRWStructuredBuffer(4, sizeof(int));
		std::vector<int> data = { 1,2,3,4 };
		buffer->upload(data.data(), sizeof(int) * data.size());

		mComputeShader->setStructuredBuffer(*buffer.get(), 0);
		mComputeShader->setRWStructuredBuffer(*rwBuffer.get(), 0);

		mComputeShader->dispatch(4, 1, 1);

		int* checker = static_cast<int*>(rwBuffer->read());

		EXPECT_EQ(2, *checker); ++checker;
		EXPECT_EQ(3, *checker); ++checker;
		EXPECT_EQ(4, *checker); ++checker;
		EXPECT_EQ(5, *checker); ++checker;


	}
}
