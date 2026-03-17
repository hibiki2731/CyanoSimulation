#include "CharacterComponent.h"
#include "Actor.h"
#include "Game.h"

CharacterComponent::CharacterComponent(Actor* owner, int updateOrder) : Component(owner, updateOrder)
{
	mMaxHP = 100;
	mHP = mMaxHP;
	mPower = 20;
	mDefense = 5;
	isAlive = true;

	mDirection = 1;
	mIndexPos = std::vector<int>{
		static_cast<int>(std::round(mOwner->getPosition().x / MAPTIPSIZE)),
		static_cast<int>(std::round(mOwner->getPosition().z / MAPTIPSIZE))
	};
	mMapManager = mOwner->getGame()->getMapManager();
}

CharacterComponent::~CharacterComponent()
{
}

void CharacterComponent::inputComponent()
{
}

void CharacterComponent::updateComponent()
{

}

void CharacterComponent::endProccess()
{
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
	return mIndexPos[1] * mMapManager->getMapSize() + mIndexPos[0];
}

MapManager* CharacterComponent::getMapManager()
{
	return mMapManager;
}

void CharacterComponent::setMaxHP(int maxHP)
{
	mMaxHP = maxHP;
	if (mHP > maxHP) mHP = maxHP;
}

void CharacterComponent::setHP(int hp)
{
	mHP = hp;
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
	mIndexPos[0] = indexPos % mMapManager->getMapSize();
	mIndexPos[1] = indexPos / mMapManager->getMapSize();
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

