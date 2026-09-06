#include <pix3.h>
#include "ShaderTest/ShaderTest.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"
#include "Compute/ComputeDevice.h"


namespace ShaderTest {

	TEST_F(GameSideComputeTest, TestSetStructuredBufferWhenToView) {
		mGame = std::make_unique<Game>();
		mGame->init();

		mFactory = &GetEngineResourceFactory();
		auto& computeDevice = GetComputeDevice();



		PIXCaptureParameters captureParams = {};
		captureParams.GpuCaptureParameters.FileName = L"GTest_CS_Debug.wpix";
		PIXBeginCapture(PIX_CAPTURE_GPU, &captureParams);

		auto ComputeShader = computeDevice.createComputeShader("../../GoogleTest/Content/Shader/cso/TestComputeShader.cso");
		std::shared_ptr<IStructuredBuffer> buffer = mFactory->createStructuredBuffer(4, sizeof(int));
		std::shared_ptr<IRWStructuredBuffer> rwBuffer = mFactory->createRWStructuredBuffer(4, sizeof(int));
		std::vector<int> data = { 1,2,3,4 };
		buffer->upload(data.data(), sizeof(int) * data.size());

		ComputeShader->setStructuredBuffer(*buffer.get(), 0);
		ComputeShader->setRWStructuredBuffer(*rwBuffer.get(), 0);

		ComputeShader->dispatch(4, 1, 1);
		ComputeShader->waitWriteBuffer(0);

		computeDevice.executeAndWaitGPU();


		PIXEndCapture(FALSE);

		int* checker = static_cast<int*>(rwBuffer->read());

		EXPECT_EQ(2, *checker); ++checker;
		EXPECT_EQ(3, *checker); ++checker;
		EXPECT_EQ(4, *checker); ++checker;
		EXPECT_EQ(5, *checker); ++checker;


	}

	TEST_F(GameSideComputeTest, TestClearBuffer) {
		mGame = std::make_unique<Game>();
		mGame->init();

		mFactory = &GetEngineResourceFactory();
		auto& computeDevice = GetComputeDevice();

		auto ComputeShader = computeDevice.createComputeShader("../../GoogleTest/Content/Shader/cso/TestComputeShader.cso");
		std::shared_ptr<IStructuredBuffer> buffer = mFactory->createStructuredBuffer(4, sizeof(int));
		std::shared_ptr<IRWStructuredBuffer> rwBuffer = mFactory->createRWStructuredBuffer(4, sizeof(int));
		std::vector<int> data = { 1,2,3,4 };
		buffer->upload(data.data(), sizeof(int) * data.size());

		ComputeShader->setStructuredBuffer(*buffer.get(), 0);
		ComputeShader->setRWStructuredBuffer(*rwBuffer.get(), 0);

		ComputeShader->dispatch(4, 1, 1);
		ComputeShader->waitWriteBuffer(0);

		computeDevice.executeAndWaitGPU();
		ComputeShader->clearRWStructuredBuffer(0);

		int* checker = static_cast<int*>(rwBuffer->read());

		EXPECT_EQ(0, *checker); ++checker;
		EXPECT_EQ(0, *checker); ++checker;
		EXPECT_EQ(0, *checker); ++checker;
		EXPECT_EQ(0, *checker); ++checker;


	}

	TEST_F(GameSideComputeTest, TestRootConstants) {
		mGame = std::make_unique<Game>();
		mGame->init();

		mFactory = &GetEngineResourceFactory();
		auto& computeDevice = GetComputeDevice();

		auto ComputeShader = computeDevice.createComputeShader("../../GoogleTest/Content/Shader/cso/TestComputeShader.cso");
		std::shared_ptr<IStructuredBuffer> buffer = mFactory->createStructuredBuffer(4, sizeof(int));
		std::shared_ptr<IRWStructuredBuffer> rwBuffer = mFactory->createRWStructuredBuffer(4, sizeof(int));
		std::vector<int> data = { 1,2,3,4 };
		buffer->upload(data.data(), sizeof(int) * data.size());

		int uploadA = 3	;

		ComputeShader->setRootConstants(&uploadA);
		ComputeShader->setStructuredBuffer(*buffer.get(), 0);
		ComputeShader->setRWStructuredBuffer(*rwBuffer.get(), 0);

		ComputeShader->dispatch(4, 1, 1);
		ComputeShader->waitWriteBuffer(0);

		int* checker = static_cast<int*>(rwBuffer->read());

		EXPECT_EQ(4.0f, *checker); ++checker;
		EXPECT_EQ(5.0f, *checker); ++checker;
		EXPECT_EQ(6.0f, *checker); ++checker;
		EXPECT_EQ(7.0f, *checker); ++checker;


	}
}
