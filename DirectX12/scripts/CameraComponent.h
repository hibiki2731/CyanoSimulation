#pragma once
#include <DirectXMath.h>
#include "Component.h"
using namespace DirectX;

class DungeonScene;

class CameraComponent :  public Component
{
public:
	CameraComponent(Actor& owner, DungeonScene& scene, int updateOrder = 100);
    DECLARE_COMPONENT_NAME(CameraComponent)

    void inputComponent() override;
    void updateComponent() override;

    void setActive(bool state);

    //エフェクト
    void startShake();

    //getter
    const float& getRot();
    const XMFLOAT3& getFront();

    //setter
    void setRot(float rot);

private:
    XMFLOAT3 applyShake();

	//画面揺れ用構造体
	struct Shake {
		float time = 0.0f;
		float duration = 0.5f;
		float intensity = 0.1f;
		float frequency = 10.0f;
	} mShake;

    XMFLOAT3 mFocus;
    XMFLOAT3 mFront;
    XMFLOAT3 mUp;
    float mRot;

    bool isActive;
	DungeonScene& mDungeonScene;

    //画面揺れ
	static float AccumulatedTime;

	std::unique_ptr<class PerlinNoise1D> mPerlinNoise;

};

