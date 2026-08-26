#include "ShaderTest/ShaderTest.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"


namespace ShaderTest {

	TEST_F(GameSideTest, TestSetStructuredBufferWhenToView) {

		auto ComputeShader = mFactory->createComputeShader("../../GoogleTest/Content/Shader/cso/TestComputeShader.cso");
		std::shared_ptr<IStructuredBuffer> buffer = mFactory->createStructuredBuffer(4, sizeof(int));
		std::shared_ptr<IRWStructuredBuffer> rwBuffer = mFactory->createRWStructuredBuffer(4, sizeof(int));
		std::vector<int> data = { 1,2,3,4 };
		buffer->upload(data.data(), sizeof(int) * data.size());

		ComputeShader->setStructuredBuffer(*buffer.get(), 0);
		ComputeShader->setRWStructuredBuffer(*rwBuffer.get(), 0);

		ComputeShader->dispatch(4, 1, 1);
		ComputeShader->waitWriteBuffer(*rwBuffer);

		int* checker = static_cast<int*>(rwBuffer->read());

		EXPECT_EQ(2, *checker); ++checker;
		EXPECT_EQ(3, *checker); ++checker;
		EXPECT_EQ(4, *checker); ++checker;
		EXPECT_EQ(5, *checker); ++checker;


	}
}
