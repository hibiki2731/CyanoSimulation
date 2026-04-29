#pragma once
#include "Actor.h"

class DebugCamera :
    public Actor
{
public:
    DebugCamera(Scene& scene);
    DECLARE_CLASS_NAME(DebugCamera)

    void inputActor() override;


private:
    enum class Direction {
        RIGHT,
        LEFT,
        UP,
        DOWN
    };
    void move(Direction direction);
    void moveHeight(Direction direction);
    void moveRotation(Direction direction);


    float mSpeed;
    float mRotSpeed;

};

