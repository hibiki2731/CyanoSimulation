#pragma once
#include "Component.h"
#include "DirectXMath.h"
using namespace DirectX;

class DungeonScene;
class Player;
enum Direction;

class PlayerMoveComponent : public Component
{
public:
	PlayerMoveComponent(DungeonScene& scene, Player& player);
	DECLARE_COMPONENT_NAME(PlayerMoveComponent)

	void updateComponent() override;
	void move(Direction direction);

private:
	DungeonScene& mScene;
	Player& mPlayer;
	XMFLOAT3 mTargetPos;	//移動先の座標
	bool isMoving;			//移動中かどうか
	static const float MoveSpeed;		//移動速度

	std::vector<int> calcTargetIndexPos(Direction moveDirection);
};

