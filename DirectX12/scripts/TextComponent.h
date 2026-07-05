#pragma once
#include "Component.h"
#include "Graphic.h"
#include <string>
#include "json_fwd.hpp"

class AssetManager;

class TextComponent : public Component
{
public:
	TextComponent(Actor& owner, float zDepth = 100.0f);
	DECLARE_COMPONENT_NAME(TextComponent)

	void loadFileAndCreate(const std::string& structName);
	void loadFromJson(const nlohmann::json& json) override;
	void applyTextTexture();
	void draw();

	void endProcess() override;
	//テキストの表示、非表示
	void closeText();

	//セッター
	void setText(const std::wstring& text);
	void setPosition(float x, float y);
	void setPosZ(float z);
	void setFontSize(FLOAT size);
	void setTextColor(const D2D1::ColorF& color);
	void setLineSpace(float space);
	void alignCenter(float width);

	//ゲッター
	bool getIsActive();
	float getLineSpace();
	const float getPosX() const { return mPosition.x; }
	const float getPosY() const { return mPosition.y; }
	const float getPosZ() const { return mPosition.z; }
	const XMFLOAT3& getPosition() const { return mPosition; }

private:
	D2D1::ColorF mTextColor = D2D1::ColorF(0, 0, 0);
	Graphic& mGraphic;
	AssetManager& mAssetManager;
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

	//テキストの位置、サイズ、行間など
	XMFLOAT3		mPosition;
	FLOAT			mFontSize;
	const WCHAR*	mFontName;
	bool			isLineSpaceDefault;
	float			mLineSpace;				//行の高さ
	float			mBaseLineSpace;			//行の上端からベースラインまでの距離
	float			mTextWidth;				//テキストの幅
	float			mTextHeight;			//テキストの高さ
	bool			isCenter;				//中央ぞろえか？
	float			mTextMaxWidth;
	XMFLOAT4        mColorFloat;

	ComPtr<IDWriteFactory>		mDWriteFactory;		//DWriteファクトリー
	ComPtr<IDWriteTextLayout>	mTextLayout;		//テキストレイアウト


	int mMaxRow;

	//D2D11からD3D12へテクスチャを渡すための準備
	void createEmptyTexture();
	void wrapTexture();
	void createSprite(float zDepth);

	//テキストのフォーマットの初期化
	void applyTextFormat();
	void initDWriteFactory();

	//コンスタントバッファサブアロケータ
	std::shared_ptr<class TextCBSuballocation> mCBSuballocation;
	std::unique_ptr<class DescriptorSlotRange> mDescriptorRange;

#ifdef _DEBUG
	friend class GUIDebugger;
	std::string mTextBuffer;
#endif

};

