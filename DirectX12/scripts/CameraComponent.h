#pragma once
#include <DirectXMath.h>
#include "Component.h"
using namespace DirectX;

class CameraComponent :  public Component
{
public:
	CameraComponent(Actor* owner, int updateOrder = 100);

    void inputComponent() override;
    void updateComponent() override;

    void setActive(bool state);

    //getter
    const float& getRot();
    const XMFLOAT3& getFront();

    //setter
    void setRot(float rot);

private:
    XMFLOAT3 mFocus;
    XMFLOAT3 mFront;
    XMFLOAT3 mUp;
    float mRot;

    bool isActive;

};

