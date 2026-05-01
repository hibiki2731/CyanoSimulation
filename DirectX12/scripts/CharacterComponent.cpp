#include "CharacterComponent.h"
#include "Actor.h"
#include "Game.h"
#include "DungeonScene.h"

CharacterComponent::CharacterComponent(Actor& owner, DungeonScene& scene) 
	: Component(owner),
	mScene(scene)
{
	mMaxHP = 100;
	mHP = mMaxHP;
	mPower = 20;
	mDefense = 5;
	isAlive = true;

	mDirection = 1;
	mIndexPos = std::vector<int>{
		static_cast<int>(std::round(mOwner.getPosition().x / MAPTIPSIZE)),
		static_cast<int>(std::round(mOwner.getPosition().z / MAPTIPSIZE))
	};

	if (mIndexPos[0] < 0 || mIndexPos[0] >= scene.getMapSize() ||
		mIndexPos[1] < 0 || mIndexPos[1] >= scene.getMapSize()) {
		int i = 0;
	}
}

int CharacterComponent::getHP()
{
	return mHP;
}

int CharacterComponent::getMaxHP()
{
	return mMaxHP;
}

int CharacterComponent::getPower()
{
	return mPower;
}

int CharacterComponent::getDefense()
{
	return mDefense;
}

int CharacterComponent::getDirection()
{
	return mDirection;
}

bool CharacterComponent::getAlive()
{
	return isAlive;
}

std::vector<int>& CharacterComponent::getIndexPos()
{
	return mIndexPos;
}

int CharacterComponent::getIndexPosInt()
{
	return mIndexPos[1] * mScene.getMapSize() + mIndexPos[0];
}

void CharacterComponent::setMaxHP(int maxHP)
{
	mMaxHP = maxHP;
	if (mHP > maxHP) mHP = maxHP;
}

void CharacterComponent::setHP(int hp)
{
	mHP = max(hp, 0);
}

void CharacterComponent::setPower(int power)
{
	mPower = power;
}

void CharacterComponent::setDefense(int defense)
{
	mDefense = defense;
}

void CharacterComponent::setDirection(int direction)
{
	mDirection = direction;
}

void CharacterComponent::setIndexPos(int x, int y)
{
	mIndexPos[0] = x;
	mIndexPos[1] = y;
}

void CharacterComponent::setIndexPosInt(int indexPos)
{
	mIndexPos[0] = indexPos % mScene.getMapSize();
	mIndexPos[1] = indexPos / mScene.getMapSize();
}

void CharacterComponent::addHP(int hp)
{
	mHP += hp;
	if (mHP > mMaxHP) mHP = mMaxHP;
}

void CharacterComponent::giveDamage(int damage)
{
	mHP -= damage;
	if (mHP < 0) {
		mHP = 0;
		isAlive = false;
	}
}

void CharacterComponent::turnRight()
{
	mDirection = mDirection >> 1;
	if (mDirection < Direction::DOWN) {
		mDirection = Direction::LEFT;
	}
}

void CharacterComponent::turnLeft()
{
	mDirection = mDirection << 1;
	if (mDirection > Direction::LEFT) {
		mDirection = Direction::DOWN;
	}
}

