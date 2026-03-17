#include "DamageText.h"
#include "Game.h"
#include "AssetManager.h"


DamageText::DamageText(XMFLOAT3& position, int digit, float maxLifeTime, XMFLOAT3 velocity)
{
	mCenterPosition = position;
	mVelocity = velocity;
	mDigit = digit;
	mLifeTime = maxLifeTime;
}

DamageText::~DamageText()
{
}

void DamageText::update()
{
	mLifeTime -= deltaTime;	//寿命の更新
	mCenterPosition = mCenterPosition + mVelocity * deltaTime;	//位置の更新
}

float DamageText::getPosX()
{
	return mCenterPosition.x;
}

float DamageText::getPosY() {
	return mCenterPosition.y;
}

float DamageText::getPosZ() {
	return mCenterPosition.z;
}

XMFLOAT3 DamageText::getPos()
{
	return mCenterPosition;
}

float DamageText::getLifeTime()
{
	return mLifeTime;
}

DamageTextManager::DamageTextManager(Game* game)
{
	mGame = game;
	mCBSize = 256; //使用するコンスタントバッファは一つだけ
	mHeapSize = 4;
	mCBIndex = mGame->getAssetManager()->getCBEndIndex(mCBSize);
	mHeapIndex = mGame->getAssetManager()->getHeapEndIndex(mHeapSize);
	mNextInstanceIndex = 0;

	mInstanceRawData.reserve(MaxNum);
	////vertexBuffer作成
	HRESULT hr;
	for(int i = 0; i < 2; i ++) hr = mGame->getGraphic()->createBuf(SizeInByte, mVertexBuf[i]);
	assert(SUCCEEDED(hr));

	////vertexBufferView作成
	for (int i = 0; i < 2; i++) {
		mVertexBufView[i].BufferLocation = mVertexBuf[i]->GetGPUVirtualAddress();
		mVertexBufView[i].SizeInBytes = SizeInByte;
		mVertexBufView[i].StrideInBytes = sizeof(float) * NumElementsPerVertex; //頂点ごとのバイト数
	}
	
	for(int i = 0; i < 2; i++) 
	HRESULT hr = mVertexBuf[i]->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData[i]));

	//コンスタントバッファの初期化
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, mGame->getGraphic()->getAspect(), 0.01f, 50.0f);
	mBC.proj = proj;
	memcpy(mGame->getGraphic()->getConstantData(0) + mCBIndex, &mBC, sizeof(BillboardConstBuf));
	memcpy(mGame->getGraphic()->getConstantData(1) + mCBIndex, &mBC, sizeof(BillboardConstBuf));

	////ファイルを読み込み、テクスチャバッファをつくる
	mTextureBuf = mGame->getAssetManager()->getShaderResource("assets\\picture\\digits.png");

	////ディスクリプタヒープにビューを作成
	auto heapIndex = mHeapIndex;
	mGame->getGraphic()->createConstantBufferView(mCBIndex, mCBSize, heapIndex, 2); heapIndex ++;
	mGame->getGraphic()->createShaderResourceView(mTextureBuf, heapIndex); heapIndex += 2;
	mGame->getGraphic()->createShaderResourceView(mTextureBuf, heapIndex); 


}

DamageTextManager::~DamageTextManager()
{
	for (int i = 0; i < 2; i++) mVertexBuf[i]->Unmap(0, nullptr);
}

void DamageTextManager::update()
{
	std::vector<int> deadTextsIndex;

	XMVECTOR vvelocity = XMLoadFloat3(&Velocity);
	for (int t = 0; t < mInstanceRawData.size(); t++) {

		//位置の更新
		XMVECTOR vpos = XMLoadFloat3(&mInstanceRawData[t].pos);
		vpos = vpos + vvelocity * deltaTime;
		XMStoreFloat3(&mInstanceRawData[t].pos, vpos);
		//alpha値の更新
		float alpha = (mInstanceRawData[t].alpha - deltaTime) / MaxLifeTime;
		mInstanceRawData[t].alpha = alpha;

		//寿命が切れたら削除待ち配列に追加
		if (alpha <= 0.0f) {
			deadTextsIndex.emplace_back(t);
		}
	}

	//削除待ち配列中の要素を削除
	for (int index = deadTextsIndex.size() - 1; index >= 0; index--) {
		//最後尾のデータをindexに入れ、最後尾を除去
		mInstanceRawData[deadTextsIndex[index]] = mInstanceRawData.back();
		mInstanceRawData.pop_back();

	}

	//VertexBufferを更新
	memcpy(mMappedData[mGame->getGraphic()->getBackBufIdx()], mInstanceRawData.data(), mInstanceRawData.size() * sizeof(DamageTextInstance));

}

void DamageTextManager::draw()
{
	//描画するダメージテキストがなければ、描画処理を抜ける
	if (mInstanceRawData.size() == 0)  return; 

	mGame->getGraphic()->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	//頂点をセット
	int backBufIdx = mGame->getGraphic()->getBackBufIdx();
	mGame->getGraphic()->getCommandList()->IASetVertexBuffers(0, 1, &mVertexBufView[backBufIdx]);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	auto hDescHeap = mGame->getGraphic()->getHeapHandle();
	hDescHeap.ptr += (mHeapIndex + backBufIdx * 2) * mGame->getGraphic()->getCbvTbvIncSize();
	mGame->getGraphic()->getCommandList()->SetGraphicsRootDescriptorTable(0, hDescHeap);
	//描画。インデックスを使用しない
	mGame->getGraphic()->getCommandList()->DrawInstanced(1, mInstanceRawData.size(), 0, 0);
	
}

void DamageTextManager::createDamageText(XMFLOAT3& position, int digit)
{
	//生データを配列に追加
	DamageTextInstance text;
	text.pos = position;
	text.size = DTSize;
	text.digit = static_cast<float>(digit);
	text.alpha = 1.0f;

	//damageTextを配列に追加
	//ダメージテキストが最大数に達していた場合、alpha値が最も小さいものと入れ替える
	if (mInstanceRawData.size() >= MaxNum) {
		auto it = std::min_element(mInstanceRawData.begin(), mInstanceRawData.end(), [](const DamageTextInstance& a, const DamageTextInstance& b) {
			return a.alpha < b.alpha;
			});
	
		int index = std::distance(mInstanceRawData.begin(), it);
		mInstanceRawData[index] = text;
	}
	//最大数でない場合、配列の後に追加
	else mInstanceRawData.emplace_back(text);
}

void DamageTextManager::updateView(XMMATRIX& view)
{
	mBC.view = view;
	memcpy(mGame->getGraphic()->getConstantData() + mCBIndex, &mBC, sizeof(BillboardConstBuf));
}

float DamageTextManager::getSize()
{
	return DTSize;
}
