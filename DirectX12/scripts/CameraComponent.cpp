#include "CameraComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Math.h"
#include "DamageText.h"
#include <windows.h>


CameraComponent::CameraComponent(Actor* owner, int updateOrder) : Component(owner, updateOrder)
{
	mFront = { 0, 0, 1.0f };  mUp = { 0, 1, 0 };
	mFocus = mOwner->getPosition() + mFront;
	mRot = 0;
	isActive = false;
}

void CameraComponent::inputComponent()
{
}

void CameraComponent::updateComponent()
{
	if (isActive) {
		mFront = { 0.0f, 0.0f, 1.0f };
		mFront = Math::rotateX(mFront, mOwner->getRotation().x);
		mFront = Math::rotateY(mFront, mOwner->getRotation().y);
		mFront = Math::rotateZ(mFront, mOwner->getRotation().z);
		mFocus = mOwner->getPosition() + mFront;
		XMFLOAT3 eye = mOwner->getPosition();


		XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&mFocus), XMLoadFloat3(&mUp));
		//プロジェクションマトリックス
		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, mOwner->getGame()->getGraphic()->getAspect(), 0.01f, 50.0f);
		XMMATRIX viewProj = view * proj;
		mOwner->getGame()->getDamageTextManager()->updateView(view);
		mOwner->getGame()->getGraphic()->updateViewProj(viewProj);

		XMFLOAT4 cameraPos;
		cameraPos.x = eye.x;
		cameraPos.y = eye.y;
		cameraPos.z = eye.z;
		cameraPos.w = 1;
		mOwner->getGame()->getGraphic()->updateCameraPos(cameraPos);
	}

}

void CameraComponent::setActive(bool state)
{
	isActive = state;
}

const float& CameraComponent::getRot()
{
	return mRot;
}

const XMFLOAT3& CameraComponent::getFront()
{
	return mFront;
}

void CameraComponent::setRot(float rot)
{
	mRot = rot;
}
