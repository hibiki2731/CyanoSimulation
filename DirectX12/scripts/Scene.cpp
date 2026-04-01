#include "Scene.h"
#include "Game.h"
#include "SceneManager.h"
#include "MeshComponent.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include <algorithm>

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

void Scene::addMesh(MeshComponent* mesh)
{
	mMeshes.emplace_back(mesh);
}

void Scene::removeMesh(MeshComponent* mesh)
{
	auto iter = std::find(mMeshes.begin(), mMeshes.end(), mesh);
	if (iter != mMeshes.end()) {
		std::iter_swap(iter, mMeshes.end() - 1);
		mSprites.pop_back();
	}
}

void Scene::addSprite(SpriteComponent* sprite)
{
	mSprites.emplace_back(sprite);
}

void Scene::removeSprite(SpriteComponent* sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	if (iter != mSprites.end()) {
		std::iter_swap(iter, mSprites.end() - 1);
		mSprites.pop_back();
	}
}

void Scene::addText(TextComponent* text)
{
	mTexts.emplace_back(text);
}

void Scene::removeText(TextComponent* text)
{
	auto iter = std::find(mTexts.begin(), mTexts.end(), text);
	if (iter != mTexts.end()) {
		std::iter_swap(iter, mTexts.end() - 1);
		mTexts.pop_back();
	}
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

void Scene::draw3D()
{
	for (auto& mesh : mMeshes) {
		mesh->draw();
	}
}

void Scene::draw2D()
{
	for (auto& sprite : mSprites) {
		sprite->draw();
	}
	for (auto& text : mTexts) {
		text->draw();
	}
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

