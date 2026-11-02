/*****************************************************************//**
 * \file   CyanoCalculator.h
 * \brief  
 * 
 * \author h_ka1
 * \date   August 2026
 *********************************************************************/

#pragma once
#include <memory>
#include "Graphic/Core/Definition.h"
#include <wrl/client.h>
#include <winuser.h>
#include <array>
#include <d3dx12.h>
class IRWStructuredBuffer;
class IStructuredBuffer;
class IComputeShader;

class CyanoCalculator
{
public:
	CyanoCalculator(class Graphic& graphic, const UINT maxPointNum, const UINT maxNumIndividuals, std::unique_ptr<IRWStructuredBuffer>& pointsPosOut, std::unique_ptr<IRWStructuredBuffer>& interactionBuffer, struct ComputeParam& param);
	~CyanoCalculator();

	void startCalculation(UINT numPoints, UINT numIndividuals);
	void uploadIndividualBegin(std::vector<UINT>& srcData, const UINT startIndividualIndex);
	void uploadIndividualSize(std::vector<UINT>& srcData, const UINT startIndividualIndex);
	void uploadIndividualSpeed(std::vector<float>& srcData, const UINT startIndividualIndex);
	void uploadIndividualHeadIdx(std::vector<UINT>& srcData, const UINT startIndividualIndex);
	void uploadIndividualAngularVelocity(std::vector<float>& srcData, const UINT startIndividualIndex);
	void uploadAngle(std::vector<float>& srcData, const UINT startPointsIndex);
private:
	void prepareShaders(const UINT maxPointNum);

	void dispatchCellIdxHistogram(const UINT numPoints);
	void computeCellStart();
	void dispatchScatter(const UINT numPoints);
	void dispatchAngleAndMove(const UINT numIndividuals);


	//空間分割法に用いるパラメータ
	struct GridParams {
		UINT numPoints;
		UINT gridWidth;
		UINT gridHeight;
		UINT cellSize;
	};
	GridParams mUploadParams;

	//シアノの情報
	struct ComputeParam& mComputeParam;
	int mNumCyanos = 0;
	const int mMaxNumCyanos;

	//コンピュートシェーダ
	//点のセルインデックスとヒストグラムを計算するためのバッファ
	std::unique_ptr<IComputeShader> mHistogramShader;
	std::unique_ptr<IStructuredBuffer> mPointsPosInBuffer;
	std::unique_ptr<IRWStructuredBuffer> mCellIdxBuffer;	//各点のセル番号を保存
	std::unique_ptr<IRWStructuredBuffer> mHistogramBuffer;	//各セルに存在する点の数を保存

	//スキャッター用バッファ
	std::unique_ptr<IComputeShader> mScaterShader;
	std::unique_ptr<IStructuredBuffer> mCellStartBuffer;
	std::unique_ptr<IRWStructuredBuffer> mCellCursorBuffer;
	std::unique_ptr<IRWStructuredBuffer> mSortedIndexBuffer;

	//位置更新用シェーダ
	std::unique_ptr<IComputeShader> mAngleAndMoveShader;
	std::unique_ptr<IStructuredBuffer> mPointsAngleInBuffer;	//参照用角度、出力された角度からコピー
	//個体追加時に編集
	std::unique_ptr<IStructuredBuffer> mIndividualBeginBuffer;	//個体の先頭インデッククス
	std::unique_ptr<IStructuredBuffer> mIndividualSizeBuffer;	//個体の配列のサイズ
	std::unique_ptr<IStructuredBuffer> mIndividualSpeed;		//個体の速さ
	std::unique_ptr<IRWStructuredBuffer>& mPointsPosOutBuffer;
	std::unique_ptr<IRWStructuredBuffer> mPointsAngleOutBuffer;	//点毎の更新後の角度
	std::unique_ptr<IRWStructuredBuffer> mIndividualheadIdxBuffer;//個体の頭のインデックス
	std::unique_ptr<IRWStructuredBuffer> mIndividualAngularVelocityBuffer;	//個体の角速度
	std::unique_ptr<IRWStructuredBuffer>& mPointsIsInteractionBuffer;

};

