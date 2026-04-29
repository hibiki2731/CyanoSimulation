#include "DebugCamera.h"
#include "CameraComponent.h"
#include "SpotLightComponent.h"
#include "input.h"
#include "timer.h"
#include "Math.h"
#include <Windows.h>

DebugCamera::DebugCamera(Scene& scene)
	:Actor(scene)
{
	mSpeed = 5.0f;
	mRotSpeed = 1.0f;
	mPosition = { 0.0f, 1.0f, 0.0f };

	//カメラコンポーネントの生成
	auto camera = std::make_unique<CameraComponent>(*this);
	camera->setActive(true);
	addComponent(std::move(camera));

	auto light = std::make_unique<SpotLightComponent>(*this);
	light->setActive(true);
	addComponent(std::move(light));

}

void DebugCamera::inputActor()
{
	//鉛直方向の移動
	if (isKeyPressed(VK_SHIFT)) {
		if(isKeyPressed(VK_UP))	moveHeight(Direction::UP);
		else if(isKeyPressed(VK_DOWN)) moveHeight(Direction::DOWN);

		return;
	}

	if (isKeyPressed(VK_CONTROL)) {
		if (isKeyPressed(VK_RIGHT)) {
			moveRotation(Direction::RIGHT);
		}
		if (isKeyPressed(VK_LEFT)) {
			moveRotation(Direction::LEFT);
		}
		if (isKeyPressed(VK_UP)) {
			moveRotation(Direction::UP);
		}
		if (isKeyPressed(VK_DOWN)) {
			moveRotation(Direction::DOWN);
		}

		return;
	}
	//水平方向の移動
	if (isKeyPressed(VK_RIGHT)) {
		move(Direction::RIGHT);
	}
	if (isKeyPressed(VK_LEFT)) {
		move(Direction::LEFT);
	}
	if (isKeyPressed(VK_UP)) {
		move(Direction::UP);
	}
	if (isKeyPressed(VK_DOWN)) {
		move(Direction::DOWN);
	}


}

void DebugCamera::move(Direction direction)
{
	//前方ベクトルの取得
	XMFLOAT3 front = { 0.0f, 0.0f, 1.0f };
	front = Math::rotate(front, mRotation);

	//右方向ベクトルの取得
	static XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	XMVECTOR vRight = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&front))); //front * up
	XMFLOAT3 right;
	XMStoreFloat3(&right, vRight);

	//移動前のy座標を保存
	float yPos = mPosition.y;

	//移動
	switch (direction) {
	case Direction::RIGHT:
		mPosition = mPosition + right * (mSpeed * deltaTime);
		break;
	case Direction::LEFT:
		mPosition = mPosition - right * (mSpeed * deltaTime);
		break;
	case Direction::UP:
		mPosition = mPosition + front * (mSpeed * deltaTime);
		break;
	case Direction::DOWN:
		mPosition = mPosition - front * (mSpeed * deltaTime);
		break;
	}

	//y座標は変化させない
	mPosition.y = yPos;
	
}

void DebugCamera::moveHeight(Direction direction)
{
	static XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };

	switch (direction) {
	case Direction::UP:
		mPosition = mPosition + up * (mSpeed * deltaTime);
		break;
	case Direction::DOWN:
		mPosition = mPosition - up * (mSpeed * deltaTime);
		break;
	}

}

void DebugCamera::moveRotation(Direction direction)
{
	switch (direction) {
	case Direction::RIGHT:
		mRotation.y = mRotation.y + mRotSpeed * deltaTime;
		break;
	case Direction::LEFT:
		mRotation.y = mRotation.y - mRotSpeed * deltaTime;
		break;
	case Direction::UP:
		mRotation.x = mRotation.x - mRotSpeed * deltaTime;
		break;
	case Direction::DOWN:
		mRotation.x = mRotation.x + mRotSpeed * deltaTime;
		break;
	}
}
