#pragma once
#include "Scene.h"
#include <memory>

class DungeonScene :
    public Scene
{
public:
    DungeonScene(class Game& game, class SceneManager* sceneManager);

    void fastUpdate() override;
    void update() override;
    void onEnter() override;
    void onExit() override;
    
    const std::string& getName() const override {
        return "DUNGEON";
    };
private:
    std::unique_ptr<class MapManager> mMapManager;
};

