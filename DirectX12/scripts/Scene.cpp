#include "Scene.h"
#include "Game.h"
#include "SceneManager.h"

Scene::Scene(Game& game)
	: mGame(game)
{
}

void Scene::addActor(std::unique_ptr<Actor> actor)
{
	//アクターを待ち配列に追加
	mPendingActors.emplace_back(std::move(actor));
}

void Scene::joinActors()
{
	for (auto& pending : mPendingActors) {
		mActors.emplace_back(std::move(pending));
	}
	mPendingActors.clear();
}

void Scene::removeActors()
{
	//死んだアクターの終了処理
	for (auto& actor : mActors) {
		if (actor->getState() == Actor::Dead) {
			actor->endProccess();
		}
	}

	//配列からアクターを削除
	std::erase_if(mActors, [](const std::unique_ptr<Actor>& actor) {
		return !actor || actor->getState() == Actor::State::Dead;
		});
}

void Scene::refreshActors()
{
	for (auto& actor : mActors) {
		actor->setState(Actor::State::Dead);
	}
}


void Scene::fastUpdate()
{
	fastUpdateScene();
	fastUpdateActors();
}

void Scene::update()
{
	updateScene();
	updateActors();
}

void Scene::lateUpdate()
{
	lateUpdateScene();
	lateUpdateActors();
}

void Scene::input()
{
	inputScene();
	inputActors();
}

void Scene::fastUpdateActors()
{
	for (auto& actor : mActors) {
		actor->fastUpdate();
	}
}

void Scene::updateActors()
{
	for (auto& actor : mActors) {
		actor->update();
	}
}

void Scene::lateUpdateActors()
{
	for (auto& actor : mActors) {
		actor->lateUpdate();
	}
}

void Scene::inputActors()
{
	for (auto& actor : mActors) {
		actor->input();
	}
}

