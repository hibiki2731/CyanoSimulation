#include "pch.h"
#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include "Math.h"

Actor::Actor(Scene& scene)
: mScene(scene)
{
	mPosition = { 0.0f, 0.0f, 0.0f };
	mScale = { 1.0f, 1.0f, 1.0f };
	mRotation = { 0.0f, 0.0f, 0.0f };
	mState = Active;

}

Actor::Actor(Scene& scene, float x, float y)
	:mScene(scene)
{
	mPosition = { x, 0.0f, y };
	mScale = { 1.0f, 1.0f, 1.0f };
	mRotation = { 0.0f, 0.0f, 0.0f };
	mState = Active;

}

Actor::~Actor() = default;

void Actor::input()
{
	if (mState == Active) {
		for (auto& component : mComponents) {
			component->inputComponent();
		}
		inputActor();
	}
}

void Actor::fastUpdate()
{
	if (mState == Active) {
		fastUpdateComponents();
		fastUpdateActor();
	}
}

void Actor::update()
{
	if (mState == Active) {
		updateComponents();
		updateActor();
	}
}

void Actor::lateUpdate() {
	if (mState == Active) {
		lateUpdateComponents();
		lateUpdateActor();
	}
}

void Actor::fastUpdateComponents()
{
	for (auto& component : mComponents) {
		component->fastUpdateComponent();
	}
}

void Actor::updateComponents()
{
	for (auto& component : mComponents) {
		component->updateComponent();
	}
}

void Actor::lateUpdateComponents() {
	for (auto& component : mComponents) {
		component->lateUpdateComponent();
	}
}

void Actor::endProcess()
{
	for (auto& component : mComponents) {
		component->endProcess();
	}
	endProcessActor();
}

void Actor::setState(State state)
{
	mState = state;
}

void Actor::setPosition(const XMFLOAT3& position)
{
	mPosition = position;
}

void Actor::setScale(const XMFLOAT3& scale)
{
	mScale = scale;
}

void Actor::setRotation(const XMFLOAT3& rotation)
{
	mRotation = rotation;
}

void Actor::setPosX(float x)
{
	mPosition.x = x;
}

void Actor::setPosY(float y)
{
	mPosition.y = y;
}

void Actor::setPosZ(float z)
{
	mPosition.z = z;
}

void Actor::setXRot(float x)
{
	mRotation.x = x;
}

void Actor::setYRot(float y)
{
	mRotation.y = y;
}

void Actor::setZRot(float z)
{
	mRotation.z = z;
}

void Actor::movePosition(const XMFLOAT3& diff)
{
	mPosition = mPosition + diff;
}

Actor::State Actor::getState()
{
	return mState;
}

XMFLOAT3 Actor::getPosition() const
{
	return mPosition;
}

XMFLOAT3 Actor::getScale() const
{
	return mScale;
}

XMFLOAT3 Actor::getRotation() const
{
	return mRotation;
}

Scene& Actor::getScene()
{
	return mScene;
}

void Actor::addComponent(std::unique_ptr<Component> component)
{
	mComponents.emplace_back(std::move(component));
	std::sort(mComponents.begin(), mComponents.end(),
		[](const std::unique_ptr<Component>& a, const std::unique_ptr<Component>& b) {
			return a->getUpdateOrder() < b->getUpdateOrder();
		});
}

void Actor::removeComponent(std::unique_ptr<Component>& component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end()) {
		iter->get()->endProcess();
		mComponents.erase(iter);
	}
}

void Actor::removeComponent(Component* component)
{
	auto iter = std::find_if(mComponents.begin(), mComponents.end(),
		[component](const std::unique_ptr<Component>& c) { return c.get() == component; }
	);
	if (iter != mComponents.end()) {
		iter->get()->endProcess();
		mComponents.erase(iter);
	}
}
