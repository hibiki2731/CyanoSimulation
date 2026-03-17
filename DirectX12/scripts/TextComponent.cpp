#include "TextComponent.h"
#include "Actor.h"
#include "Game.h"
#include "AssetManager.h"

TextComponent::TextComponent(Actor* owner, float zDepth) : Component(owner)
{
	mGraphic = mOwner->getGame()->getGraphic();
	mAssetManager = mOwner->getGame()->getAssetManager();
	isActive = false;
	mBaseLineX = 0.0f;
	mBaseLineY = 0.0f;
	mFontSize = 32;
	mFontName = L"MS P明朝";
	mMaxRow = 20;
	mTextRect = D2D1::RectF(
		mBaseLineX,
		mBaseLineY,
		mMaxRow * mFontSize,
		mFontSize
	);
	mTextColor = D2D1::ColorF(D2D1::ColorF::White);
	mText = L"";
	isLineSpaceDefault = true;
	mLineSpace = 0;
	mBaseLineSpace = 0;

	mOwner->getGame()->addText(this);

	createEmptyTexture();
	wrapTexture();
	createSprite(zDepth);

}

TextComponent::~TextComponent()
{
}

//要マルチスレッド化
void TextComponent::drawTextTexture()  
{  
	//テクスチャの状態をshader resourceに遷移
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mTexture.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mGraphic->getCommandList()->ResourceBarrier(1, &barrier);


	mGraphic->getD3D11On12Device()->AcquireWrappedResources(mWrappedTexture.GetAddressOf(), 1);
	mGraphic->getD2DDeviceContext()->SetTarget(mD2DTarget.Get());
	mGraphic->getD2DDeviceContext()->BeginDraw();

	mGraphic->getD2DDeviceContext()->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

	mGraphic->getD2DDeviceContext()->SetTransform(D2D1::Matrix3x2F::Identity());  
	mGraphic->getD2DDeviceContext()->DrawTextW(  
	   mText.c_str(), 
       static_cast<UINT32>(mText.size() - 1), 
       mTextFormat.Get(),  
       &mTextRect,
       mTextBrush.Get()  
   );  

	mGraphic->getD2DDeviceContext()->EndDraw();
	//バックバッファを表示用に切り替えてくれる
	mGraphic->getD3D11On12Device()->ReleaseWrappedResources(mWrappedTexture.GetAddressOf(), 1);

	mGraphic->getD3D11DeviceContext()->Flush();

	mGraphic->waitGPU();
}

void TextComponent::draw()
{

	//頂点をセット
	mGraphic->getCommandList()->IASetVertexBuffers(0, 1, &mVertexBufView);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	auto hCbvTbvHeap = mGraphic->getHeapHandle();
	UINT CbvTbvSize = mGraphic->getCbvTbvIncSize();
	hCbvTbvHeap.ptr += (mHeapIndex + mGraphic->getBackBufIdx()) * CbvTbvSize;

	mGraphic->getCommandList()->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	hCbvTbvHeap = mGraphic->getHeapHandle();
	hCbvTbvHeap.ptr += (mHeapIndex + 2) * CbvTbvSize;
	mGraphic->getCommandList()->SetGraphicsRootDescriptorTable(1, hCbvTbvHeap);
	//描画。インデックスを使用
	mGraphic->getCommandList()->IASetIndexBuffer(&mIndexBufView);
	mGraphic->getCommandList()->DrawIndexedInstanced(mAssetManager->getSpriteIndicesSize(), 1, 0, 0, 0);

}

void TextComponent::endProccess()
{
	//テキストの描画処理が終わるのを待つ。
	mGraphic->waitGPU();

	mOwner->getGame()->removeText(this);
}

void TextComponent::showText()
{
	mTextRect = D2D1::RectF(
		mBaseLineX,
		mBaseLineY,
		mBaseLineX + mMaxRow * mFontSize,
		mBaseLineY + mFontSize * (mText.size() / mMaxRow + 1)
	);

    HRESULT hr = mGraphic->getD2DDeviceContext()->CreateSolidColorBrush(mTextColor, &mTextBrush);
	assert(SUCCEEDED(hr));
    hr = mGraphic->getDWriteFactory()->CreateTextFormat(
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

    hr = mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); //左揃え
	assert(SUCCEEDED(hr));

	hr = mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR); //上揃え
	assert(SUCCEEDED(hr));
	
	if(!isLineSpaceDefault)
	hr = mTextFormat->SetLineSpacing(
		DWRITE_LINE_SPACING_METHOD_UNIFORM,
		mLineSpace,
		mBaseLineSpace
	);

	isActive = true;

	drawTextTexture();
}

void TextComponent::closeText()
{
	isActive = false;
}

void TextComponent::setText(const std::wstring& text)
{
	mText = text;
}

void TextComponent::setBaseLine(float x, float y)
{
	mBaseLineX = x;
	mBaseLineY = y;
}

void TextComponent::setFontSize(FLOAT size)
{
	mFontSize = size;
}

void TextComponent::setTextColor(const D2D1::ColorF& color)
{
	mTextColor = color;
}

void TextComponent::setLineSpace(float space)
{
	isLineSpaceDefault = false;
	mLineSpace = space + mFontSize; //行間の大きさ
	mBaseLineSpace = mFontSize; //文字のベースライン
}

bool TextComponent::getIsActive()
{
	return isActive;
}

void TextComponent::createEmptyTexture()
{
	//キャンバスのサイズ
	UINT textWidth = mGraphic->getClientWidth();
	UINT textHeight = mGraphic->getClientHeight();

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

	mGraphic->getDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&textDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(mTexture.ReleaseAndGetAddressOf())
	);
	
	mTexture->SetName(L"TextComponent_Texture");

}

void TextComponent::wrapTexture()
{
	//D3D11のリソースとしてラップ（変換）
	D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
	mGraphic->getD3D11On12Device()->CreateWrappedResource(
		mTexture.Get(),
		&d3d11Flags,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		IID_PPV_ARGS(mWrappedTexture.ReleaseAndGetAddressOf())
	);

	//ラップしたテクスチャをDirect2Dのレンダーターゲットにする
	ComPtr<IDXGISurface> surface;
	mWrappedTexture.As(&surface);

	D2D1_BITMAP_PROPERTIES1 bitmapProps = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	HRESULT hr = mGraphic->getD2DDeviceContext()->CreateBitmapFromDxgiSurface(
		surface.Get(),
		&bitmapProps,
		mD2DTarget.ReleaseAndGetAddressOf()
	);
	assert(SUCCEEDED(hr));

	////テクスチャの状態をpixel shader resourceに遷移
	//D3D12_RESOURCE_BARRIER barrier;
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//barrier.Transition.pResource = mTexture.Get();
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//mGraphic->getCommandList()->ResourceBarrier(1, &barrier);



}

void TextComponent::createSprite(float zDepth)
{
	//コンスタントバッファとディスクリプタヒープのインデックスを取得
	mCBSize = 256 * 2; //SpriteConstantBuf + テクスチャ
	mHeapSize = 3;
	mCBIndex = mAssetManager->getCBEndIndex(mCBSize);
	mHeapIndex = mAssetManager->getHeapEndIndex(mHeapSize);

	//各種Viewの取得
	SpriteData spriteData = mAssetManager->getSpriteData();
	mVertexBufView = spriteData.VertexBufView;
	mIndexBufView = spriteData.IndexBufView;

	//SpriteConstantBufの初期化
	Cb3.world = XMMatrixIdentity()
		*XMMatrixTranslation(0.0f, 0.0f, zDepth);
	Cb3.windowSize = XMFLOAT2(
		(float)mGraphic->getClientWidth(),
		(float)mGraphic->getClientHeight()
	);
	Cb3.spriteSize = XMFLOAT2(
		(float)mGraphic->getClientWidth(),
		(float)mGraphic->getClientHeight()
	);
	Cb3.textureSize = XMFLOAT2(
		(float)mGraphic->getClientWidth(),
		(float)mGraphic->getClientHeight()
	);
	Cb3.bordarSize = 0.0f;
	memcpy(mGraphic->getConstantData(0) + mCBIndex, &Cb3, sizeof(SpriteConstBuf));
	memcpy(mGraphic->getConstantData(1) + mCBIndex, &Cb3, sizeof(SpriteConstBuf));

	//ディスクリプタヒープにViewを作成
	int heapIndex = mHeapIndex;
	mGraphic->createConstantBufferView(mCBIndex, 256, heapIndex, 1); heapIndex += 2;
	mGraphic->createShaderResourceView(mTexture.Get(), heapIndex);
}
