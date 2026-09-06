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
	CyanoCalculator(class Graphic& graphic, const UINT maxPointNum);
	~CyanoCalculator();

	void startCalculation(std::vector<XMFLOAT4>& pointsPos);
private:
	void prepareShaders(const UINT maxPointNum);

	void dispatchCellIdxHistogram(const UINT numPoints);
	void computeCellStart();
	void dispatchScatter(const UINT numPoints);


	//空間分割法に用いるパラメータ
	struct GridParams {
		UINT numPoints;
		UINT gridWidth;
		UINT gridHeight;
		UINT cellSize;
	};
	GridParams mUploadParams;
	static const float PIXEL_AREA_WIDTH;
	static const float PIXEL_AREA_HEIGHT;
	static const int GRID_WIDTH;
	static const int GRID_HEIGHT;
	static const int CELL_SIZE;

	//シアノの情報
	int mNumCyanos = 0;

	//コンピュートシェーダ
	//点のセルインデックスとヒストグラムを計算するためのバッファ
	std::unique_ptr<IComputeShader> mHistogramShader;
	std::unique_ptr<IStructuredBuffer> mPointsPosBuffer;
	std::unique_ptr<IRWStructuredBuffer> mCellIdxBuffer;	//各点のセル番号を保存
	std::unique_ptr<IRWStructuredBuffer> mHistogramBuffer;	//各セルに存在する点の数を保存

	//スキャッター用バッファ
	std::unique_ptr<IComputeShader> mScaterShader;
	std::unique_ptr<IStructuredBuffer> mCellStartBuffer;
	std::unique_ptr<IRWStructuredBuffer> mCellCursorBuffer;
	std::unique_ptr<IRWStructuredBuffer> mSortedIndexBuffer;

	//位置更新用シェーダ
	struct SimlationParams {
		UINT numIndividuals;
	} mSimParams;
	std::unique_ptr<IComputeShader> mAngleAndMoveShader;
	std::unique_ptr<IStructuredBuffer> mPointsAngleInBuffer;	//参照用角度、出力された角度からコピー
	//個体追加時に編集
	std::unique_ptr<IStructuredBuffer> mIndividualBeginBuffer;	//個体の先頭インデッククス
	std::unique_ptr<IStructuredBuffer> mIndividualSizeBuffer;	//個体の配列のサイズ
	std::unique_ptr<IStructuredBuffer> mIndividualSpeed;		//個体の速さ

	std::unique_ptr<IRWStructuredBuffer> mPointsPosOutBuffer;	//最終的に出力する点の位置のバッファ
	std::unique_ptr<IRWStructuredBuffer> mPointsAngleOutBuffer;	//点毎の更新後の角度
	std::unique_ptr<IRWStructuredBuffer> mIndividualheadIdxBuffer;//個体の頭のインデックス
	std::unique_ptr<IRWStructuredBuffer> mIndividualAngularVelocityBuffer;	//個体の角速度

};

