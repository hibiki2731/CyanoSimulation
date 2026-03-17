#pragma once
#include "Component.h"
#include "Graphic.h"
#include <string>

class AssetManager;

class TextComponent : public Component
{
public:
	TextComponent(Actor* owner, float zDepth = 100.0f);
	~TextComponent();

	void drawTextTexture();
	void draw();

	void endProccess() override;
	//テキストの表示、非表示
	void showText();
	void closeText();

	//セッター
	void setText(const std::wstring& text);
	void setBaseLine(float x, float y);
	void setFontSize(FLOAT size);
	void setTextColor(const D2D1::ColorF& color);
	void setLineSpace(float space);

	//ゲッター
	bool getIsActive();

private:
	D2D1::ColorF mTextColor = D2D1::ColorF(0, 0, 0);
	Graphic* mGraphic;
	AssetManager* mAssetManager;
	ComPtr<ID2D1SolidColorBrush> mTextBrush;
	ComPtr<IDWriteTextFormat> mTextFormat;
	ComPtr<ID3D12Resource> mTexture;
	bool isTextureInitialized = false;
	ComPtr<ID3D11Resource> mWrappedTexture;
	ComPtr<ID2D1Bitmap1> mD2DTarget;
	//コンスタントバッファ1(World Matrix)
    SpriteConstBuf Cb3;
	int mCBIndex;
	int mCBSize;
	int mHeapIndex;
	int mHeapSize;

    //頂点バッファ
    D3D12_VERTEX_BUFFER_VIEW mVertexBufView;
    D3D12_INDEX_BUFFER_VIEW mIndexBufView;

	std::wstring mText;
	bool isActive;

	float mBaseLineX;
	float mBaseLineY;
	FLOAT mFontSize;
	const WCHAR* mFontName;
	D2D1_RECT_F mTextRect;
	bool isLineSpaceDefault;
	float mLineSpace; //行の高さ
	float mBaseLineSpace; //行の上端からベースラインまでの距離

	int mMaxRow;

	//D2D11からD3D12へテクスチャを渡すための準備
	void createEmptyTexture();
	void wrapTexture();
	void createSprite(float zDepth);

};

