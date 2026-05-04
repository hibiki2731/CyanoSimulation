#include "FireParticleComponent.h"
#include "Actor.h"
#include "Game.h"
#include "AssetManager.h"
#include "Scene.h"
#include "Random.h"
#include "DungeonScene.h"
#include "myJson.h"

FireParticleComponent::FireParticleComponent(Actor& owner)
	:Component(owner),
	mAssetManager(owner.getScene().getGame().getAssetManager()),
	mGraphic(owner.getScene().getGame().getGraphic())
{
	mHeapSize = 4; //コンスタントバッファ×2, テクスチャ×2
	mHeapIndex = mOwner.getScene().getGame().getAssetManager().getHeapEndIndex(mHeapSize);

	//vertexBuffer関連の初期化
	for (int i = 0; i < 2; i++) {
		//vertexBufferの作成
		mGraphic.createBuf(SizeInByte, mVertexBuf[i]);
		//vertexBufferViewの作成
		mVertexBufView[i].BufferLocation = mVertexBuf[i]->GetGPUVirtualAddress();
		mVertexBufView[i].SizeInBytes = SizeInByte;
		mVertexBufView[i].StrideInBytes = sizeof(UploadDataForGPU);
	}

	//マップ
    for (int i = 0; i < 2; i++) {
        mVertexBuf[i]->Map(
            0, nullptr,
            reinterpret_cast<void**>(&mMappedData[i])
        );
    }

	//テクスチャ
	mTextureBuf = mAssetManager.getShaderResource("assets/picture/fire_ember.png");

    // ヒープにビュー作成
    int heapIdx = mHeapIndex;
    mGraphic.createConstantBufferView(mGraphic.getBCIndex(), 256, heapIdx, 2); heapIdx++;
    mGraphic.createShaderResourceView(mTextureBuf, heapIdx); heapIdx += 2;
    mGraphic.createShaderResourceView(mTextureBuf, heapIdx);

    //ダンジョンシーンからのみ生成！！
    //パーティクル配列に追加
    static_cast<DungeonScene&>(mOwner.getScene()).addParticle(this);

    //エミッタの位置を初期化
    mEmitterPosition = {0.0f, 0.0f, 0.0f};
}

void FireParticleComponent::loadFromJson(const nlohmann::json& json)
{
	setEmitterPosition(json.value("particleEmitPos", XMFLOAT3(0.0f, 0.0f, 0.0f)));
}

void FireParticleComponent::endProcess()
{ 
    for (int i = 0; i < 2; i++) mVertexBuf[i]->Unmap(0, nullptr);
    //パーティクル配列から除去
    static_cast<DungeonScene&>(mOwner.getScene()).removeParticle(this);

    //メモリの解放
    mAssetManager.deleteHeap(mHeapIndex, mHeapSize);

    //バッファの解放
	ComPtr<IUnknown> vertex1, vertex2;
	if (mVertexBuf[0]) mVertexBuf[0].As(&vertex1);
	if (mVertexBuf[1]) mVertexBuf[1].As(&vertex2);

	if (vertex1) mGraphic.delayRelease(vertex1);
	if (vertex2) mGraphic.delayRelease(vertex2);
}

void FireParticleComponent::updateComponent()
{
    //パーティクルの生成
    mSpawnTimer += deltaTime;
    if (mSpawnTimer >= mConfig.spawnRate) {
        mSpawnTimer -= mConfig.spawnRate;
        for (int i = 0; i < mConfig.spawnCount; i++) spawnParticle();
    }

    //パーティクルの更新処理
    for (auto& p : mParticles) {
        p.elapsed += deltaTime;

        float ratio = 1.0f - (p.elapsed / p.lifeTime); // 1→0

        //位置更新
        XMFLOAT3 delta = p.velocity * deltaTime;
        delta.y = max(delta.y, 0.01f);
        p.gpu.position = p.gpu.position + delta;

        //揺らぎ（炎らしいランダムな横方向の動き）
        float wiggle = (Random::dist(-1.0f, 1.0f)) * 0.02f;
        p.gpu.position.x += wiggle;

        //サイズ補間（最初大きく、消える時小さく）
        p.gpu.size = p.sizeStart * ratio;

        //寿命割合を更新
        p.gpu.lifeRatio = ratio;
    }

    //死亡パーティクルの除去
    std::erase_if(mParticles, [](const ParticleData& p) {
        return p.elapsed >= p.lifeTime;
    });

    //GPU転送
    uploadToGPU();
}

void FireParticleComponent::draw()
{
    if (mParticles.empty()) return;

    auto* comandList = mGraphic.getCommandList();
    int bufIdx = mGraphic.getBackBufIdx();

    comandList->IASetVertexBuffers(0, 1, &mVertexBufView[bufIdx]);

    auto hHeap = mGraphic.getHeapHandle();
    hHeap.ptr += (mHeapIndex + bufIdx * 2) * mGraphic.getCbvTbvIncSize();
    comandList->SetGraphicsRootDescriptorTable(0, hHeap);

    comandList->DrawInstanced(1, (UINT)mParticles.size(), 0, 0);
}

void FireParticleComponent::spawnParticle()
{
    if ((int)mParticles.size() >= MaxParticles) return;

    ParticleData p;
   
    //アクターの位置と加算する
    XMFLOAT3 emitPos = mEmitterPosition + mOwner.getPosition();
    //発生位置 エミッター中心から少しランダムにばらつかせる
    p.gpu.position = XMFLOAT3(
        emitPos.x + Random::dist(-mConfig.spreadRadius, mConfig.spreadRadius),
        emitPos.y,
        emitPos.z + Random::dist(-mConfig.spreadRadius, mConfig.spreadRadius)
    );

    //速度 主に上方向、少し横に揺れる
    p.velocity = XMFLOAT3(
        Random::dist(-0.1f, 0.1f),
        Random::dist(mConfig.speedMin, mConfig.speedMax),
        Random::dist(-0.1f, 0.1f)
    );

    p.lifeTime = Random::dist(mConfig.lifeTimeMin, mConfig.lifeTimeMax);
    p.elapsed = 0.0f;
    p.sizeStart = mConfig.sizeStart + Random::dist(-0.03f, 0.03f);
    p.gpu.size = p.sizeStart;
    p.gpu.lifeRatio = 1.0f;

    mParticles.emplace_back(std::move(p));
}

void FireParticleComponent::uploadToGPU()
{ 
    mGPUData.clear();
    for (auto& p : mParticles) mGPUData.emplace_back(p.gpu);

    int idx = mGraphic.getBackBufIdx();
    if (!mGPUData.empty()) {
        memcpy(mMappedData[idx], mGPUData.data(), mGPUData.size() * sizeof(UploadDataForGPU));
    }
}
