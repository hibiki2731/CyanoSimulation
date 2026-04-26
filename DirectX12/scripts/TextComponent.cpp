#include "TextComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Scene.h"
#include "AssetManager.h"
#include "MyUtility.h"
#include "json.hpp"
#include <fstream>

TextComponent::TextComponent(Actor& owner, float zDepth) 
	: Component(owner),
	mGraphic(owner.getScene().getGame().getGraphic()),
	mAssetManager(owner.getScene().getGame().getAssetManager())
{
	mOwner.getScene().addText(this);

	//各種パラメータの初期化
	isActive = true;
	mPosX = 0.0f;
	mPosY = 0.0f;
	mFontSize = 32;
	mFontName = L"MS Gothic";
	mMaxRow = 20;
	mTextColor = D2D1::ColorF(D2D1::ColorF::White);
	mText = L"empty";
	isLineSpaceDefault = true;
	mLineSpace = 60.0f;
	mBaseLineSpace = 0;
	isCenter = false;
	mTextMaxWidth = 10000.0f;

	//ブラシの初期化
    HRESULT hr = mGraphic.getD2DDeviceContext()->CreateSolidColorBrush(mTextColor, &mTextBrush);
	assert(SUCCEEDED(hr));

	//テクスチャの初期化
	createEmptyTexture();
	wrapTexture();
	createSprite(zDepth);

	//テキストフォーマットの初期化
	initDWriteFactory();
	applyTextFormat();

	mColorFloat.push_back(mTextColor.r);
	mColorFloat.push_back(mTextColor.g);
	mColorFloat.push_back(mTextColor.b);
	mColorFloat.push_back(mTextColor.a);
}

void TextComponent::loadFileAndCreate(const std::string& structName)
{
	//テキストデータの取得
	nlohmann::json textJson;
	std::ifstream textfile("assets\\data\\textData.json");
	textfile >> textJson;

	//構造体が存在しない場合、作成する
	if (!textJson.contains(structName)) {
		textJson[structName] = {
			{"x", mPosX},
			{"y", mPosY},
			{"fontSize", mFontSize},
			{"lineSpace", mLineSpace},
			{"text", Utility::wstringToString(mText)}
		};
		std::ofstream textfileOut("assets\\data\\textData.json");
		textfileOut << textJson.dump(4);
	}

	mFontSize = textJson[structName].value("fontSize", mFontSize);
	setLineSpace(textJson[structName].value("lineSpace", mLineSpace));	//ここでテキストフォーマットが更新される
	mPosX = textJson[structName].value("x", mPosX);
	mPosY = textJson[structName].value("y", mPosY);
	mText = Utility::stringToWString(textJson[structName].value("text", "empty"));
	std::vector<float> defaultColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	mColorFloat = textJson[structName].value("color", defaultColor);
	setTextColor(D2D1::ColorF(mColorFloat[0], mColorFloat[1], mColorFloat[2], mColorFloat[3]));

	applyTextTexture();
}

//要マルチスレッド化
void TextComponent::applyTextTexture()  
{  
	//テクスチャの状態をshader resourceに遷移
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mTexture.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mGraphic.getCommandList()->ResourceBarrier(1, &barrier);

	//テキストレイアウトの更新	
	HRESULT hr = mDWriteFactory->CreateTextLayout(
    mText.c_str(),								//描画する文字列
    (UINT32)mText.length(),						//文字列の長さ
    mTextFormat.Get(),							//使用するテキストフォーマット
    mTextMaxWidth,									//最大高さ
    10000.0f,									//最大幅
    mTextLayout.ReleaseAndGetAddressOf()		//出力先
	);
	assert(SUCCEEDED(hr));

	//テキストレイアウトからテキストの実際のサイズを取得
	DWRITE_TEXT_METRICS textMetrics;
	hr = mTextLayout->GetMetrics(&textMetrics);
	assert(SUCCEEDED(hr));
	mTextWidth = textMetrics.width;
	mTextHeight = textMetrics.height;

	mGraphic.getD3D11On12Device()->AcquireWrappedResources(mWrappedTexture.GetAddressOf(), 1);
	mGraphic.getD2DDeviceContext()->SetTarget(mD2DTarget.Get());
	mGraphic.getD2DDeviceContext()->BeginDraw();

	mGraphic.getD2DDeviceContext()->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

	mGraphic.getD2DDeviceContext()->SetTransform(D2D1::Matrix3x2F::Identity());  
	mGraphic.getD2DDeviceContext()->DrawTextLayout(
		D2D1::Point2F(mPosX, mPosY),
		mTextLayout.Get(),
		mTextBrush.Get()
	);

	mGraphic.getD2DDeviceContext()->EndDraw();
	//バックバッファを表示用に切り替えてくれる
	mGraphic.getD3D11On12Device()->ReleaseWrappedResources(mWrappedTexture.GetAddressOf(), 1);

	mGraphic.getD3D11DeviceContext()->Flush();

	mGraphic.waitGPU();
}

void TextComponent::draw()
{

	//頂点をセット
	mGraphic.getCommandList()->IASetVertexBuffers(0, 1, &mVertexBufView);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	auto hCbvTbvHeap = mGraphic.getHeapHandle();
	UINT CbvTbvSize = mGraphic.getCbvTbvIncSize();
	hCbvTbvHeap.ptr += (mHeapIndex + mGraphic.getBackBufIdx()) * CbvTbvSize;

	mGraphic.getCommandList()->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	hCbvTbvHeap = mGraphic.getHeapHandle();
	hCbvTbvHeap.ptr += (mHeapIndex + 2) * CbvTbvSize;
	mGraphic.getCommandList()->SetGraphicsRootDescriptorTable(1, hCbvTbvHeap);
	//描画。インデックスを使用
	mGraphic.getCommandList()->IASetIndexBuffer(&mIndexBufView);
	mGraphic.getCommandList()->DrawIndexedInstanced(mAssetManager.getSpriteIndicesSize(), 1, 0, 0, 0);
}

void TextComponent::endProcess()
{
	mOwner.getScene().removeText(this);
	mOwner.getScene().getGame().getAssetManager().deleteMemory(mCBIndex, mCBSize);
	mOwner.getScene().getGame().getAssetManager().deleteHeap(mHeapIndex, mHeapSize);

	if (mGraphic.getD2DDeviceContext()) {
        mGraphic.getD2DDeviceContext()->SetTarget(nullptr);
        mGraphic.getD3D11DeviceContext()->Flush();
	}

	//GPUの処理が終わってから削除する
	ComPtr<IUnknown> tex, wrap, target;
	if (mTexture) mTexture.As(&tex);
	if (mWrappedTexture) mWrappedTexture.As(&wrap);
	if (mD2DTarget) mD2DTarget.As(&target);

	if (tex) mGraphic.delayRelease(tex);
	if (wrap) mGraphic.delayRelease(wrap);
	if (target) mGraphic.delayRelease(target);

}

void TextComponent::closeText()
{
	isActive = false;
}

void TextComponent::setText(const std::wstring& text)
{
	mText = text;
	applyTextTexture();
}

void TextComponent::setPosition(float x, float y)
{
	mPosX = x;
	mPosY = y;
	applyTextTexture();
}

void TextComponent::setFontSize(FLOAT size)
{
	mFontSize = size;
	applyTextFormat();
}

void TextComponent::setTextColor(const D2D1::ColorF& color)
{
	mTextColor = color;
#ifdef _DEBUG
	mColorFloat[0] = mTextColor.r;
	mColorFloat[1] = mTextColor.g;
	mColorFloat[2] = mTextColor.b;
	mColorFloat[3] = mTextColor.a;
#endif

	//ブラシを更新
    HRESULT hr = mGraphic.getD2DDeviceContext()->CreateSolidColorBrush(mTextColor, &mTextBrush);
	assert(SUCCEEDED(hr));
	applyTextTexture();
}

void TextComponent::setLineSpace(float space)
{
	isLineSpaceDefault = false;
	mLineSpace = space; //行間の大きさ
	mBaseLineSpace = mLineSpace * 0.8f; //文字のベースライン
	applyTextFormat();
}

void TextComponent::alignCenter(float width)
{
	isCenter = true;
	mTextMaxWidth = width;
	applyTextFormat();
}

bool TextComponent::getIsActive()
{
	return isActive;
}

float TextComponent::getLineSpace()
{
	return mLineSpace;
}


void TextComponent::createEmptyTexture()
{
	//キャンバスのサイズ
	UINT textWidth = Graphic::ClientWidth;
	UINT textHeight = Graphic::ClientHeight;

	D3D12_RESOURCE_DESC textDesc = {};
	textDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textDesc.Width = textWidth;
	textDesc.Height = textHeight;
	textDesc.DepthOrArraySize = 1;
	textDesc.MipLevels = 1;
	textDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textDesc.SampleDesc.Count = 1;
	textDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; //D2Dが書き込めるような設定

	//背景を透明にするための設定
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = textDesc.Format;
	clearValue.Color[0] = 0.0f, clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f, clearValue.Color[3] = 0.0f;

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	HRESULT hr =mGraphic.getDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&textDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(mTexture.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));
	
}

void TextComponent::wrapTexture()
{
	//D3D11のリソースとしてラップ（変換）
	D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
	HRESULT hr = mGraphic.getD3D11On12Device()->CreateWrappedResource(
		mTexture.Get(),
		&d3d11Flags,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		IID_PPV_ARGS(mWrappedTexture.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));

	//ラップしたテクスチャをDirect2Dのレンダーターゲットにする
	ComPtr<IDXGISurface> surface;
	mWrappedTexture.As(&surface);

	D2D1_BITMAP_PROPERTIES1 bitmapProps = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	hr = mGraphic.getD2DDeviceContext()->CreateBitmapFromDxgiSurface(
		surface.Get(),
		&bitmapProps,
		mD2DTarget.ReleaseAndGetAddressOf()
	);
	assert(SUCCEEDED(hr));
}

void TextComponent::createSprite(float zDepth)
{
	//コンスタントバッファとディスクリプタヒープのインデックスを取得
	mCBSize = 256 * 2; //SpriteConstantBuf + テクスチャ
	mHeapSize = 3;
	mCBIndex = mAssetManager.getCBEndIndex(mCBSize);
	mHeapIndex = mAssetManager.getHeapEndIndex(mHeapSize);

	//各種Viewの取得
	SpriteData spriteData = mAssetManager.getSpriteData();
	mVertexBufView = spriteData.VertexBufView;
	mIndexBufView = spriteData.IndexBufView;

	//SpriteConstantBufの初期化
	Cb3.world = XMMatrixIdentity()
		*XMMatrixTranslation(0.0f, 0.0f, zDepth);
	Cb3.windowSize = XMFLOAT2(
		(float)Graphic::ClientWidth,
		(float)Graphic::ClientHeight
	);
	Cb3.spriteSize = XMFLOAT2(
		(float)Graphic::ClientWidth,
		(float)Graphic::ClientHeight
	);
	Cb3.textureSize = XMFLOAT2(
		(float)Graphic::ClientWidth,
		(float)Graphic::ClientHeight
	);
	Cb3.bordarSize = 0.0f;
	memcpy(mGraphic.getConstantData(0) + mCBIndex, &Cb3, sizeof(SpriteConstBuf));
	memcpy(mGraphic.getConstantData(1) + mCBIndex, &Cb3, sizeof(SpriteConstBuf));

	//ディスクリプタヒープにViewを作成
	int heapIndex = mHeapIndex;
	mGraphic.createConstantBufferView(mCBIndex, 256, heapIndex, 1); heapIndex += 2;
	mGraphic.createShaderResourceView(mTexture.Get(), heapIndex);
}

void TextComponent::applyTextFormat()
{
	//テキストフォーマットの初期化
    HRESULT hr = mGraphic.getDWriteFactory()->CreateTextFormat(
        mFontName,
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        mFontSize,
        L"en-us",
        &mTextFormat
    );
    assert(SUCCEEDED(hr));

	//テキストの配置を設定
	if (!isCenter)
		hr = mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); //左揃え
	else
		hr = mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); //中央ぞろえ
	assert(SUCCEEDED(hr));
	hr = mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR); //上揃え
	assert(SUCCEEDED(hr));
	

	//行間の設定
	if(!isLineSpaceDefault)
	hr = mTextFormat->SetLineSpacing(
		DWRITE_LINE_SPACING_METHOD_UNIFORM,
		mLineSpace,
		mBaseLineSpace
	);

	applyTextTexture();
}

void TextComponent::initDWriteFactory()
{
	HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, //ファクトリのタイプ
        __uuidof(IDWriteFactory),   //取得したいインターフェースのIID
        reinterpret_cast<IUnknown**>(mDWriteFactory.ReleaseAndGetAddressOf()) //出力先のポインタ
    );
	assert(SUCCEEDED(hr));
}

#ifdef _DEBUG
void TextComponent::activateControll(const std::string& structName)
{
	mActiveControll = true;
	mStructName = structName;
	mTextBuffer = Utility::wstringToString(mText);
}
#endif