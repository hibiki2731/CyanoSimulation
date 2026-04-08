#include "Scene.h"
#include "Game.h"
#include "SceneManager.h"
#include "MeshComponent.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "GUIDebugger.h"
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
	//配列からアクターを削除
	std::erase_if(mActors, [](const std::unique_ptr<Actor>& actor) {
		if (!actor || actor->getState() == Actor::State::Dead) {
			//アクターの終了処理
			actor->endProcess();
			return true;
		}
		});
}

void Scene::addMesh(MeshComponent* mesh)
{
	mMeshes.emplace_back(mesh);
}

void Scene::removeMesh(MeshComponent* mesh)
{
	std::erase_if(mMeshes, [mesh](const MeshComponent* m) {
		return m == mesh;
		});
}

void Scene::addSprite(SpriteComponent* sprite)
{
	mSprites.emplace_back(sprite);
}

void Scene::removeSprite(SpriteComponent* sprite)
{
	std::erase_if(mSprites, [sprite](const SpriteComponent* s) {
		return s == sprite;
		});
}

void Scene::addText(TextComponent* text)
{
	mTexts.emplace_back(text);
}

void Scene::removeText(TextComponent* text)
{
	std::erase_if(mTexts, [text](const TextComponent* t) {
		return t == text;
		});
}

void Scene::addPointLight(PointLightComponent* light)
{
	if (mPointLights.size() > MAX_LIGHT_NUM) return;
	mPointLights.emplace_back(light);
}

void Scene::removePointLight(PointLightComponent* light)
{
	std::erase_if(mPointLights, [light](const PointLightComponent* l) {
		return l == light;
		});
}

void Scene::addSpotLight(SpotLightComponent* light)
{
	if (mSpotLights.size() > MAX_LIGHT_NUM) return;
	mSpotLights.emplace_back(light);
}

void Scene::removeSpotLight(SpotLightComponent* light)
{
	std::erase_if(mSpotLights, [light](const SpotLightComponent* l) {
		return l == light;
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

void Scene::draw3D()
{
	for (auto& mesh : mMeshes) {
		if (!mesh) continue;
		mesh->draw();
	}
}

void Scene::draw2D()
{
	for (auto& sprite : mSprites) {
		if (!sprite) continue;
		sprite->draw();
	}
	for (auto& text : mTexts) {
		if (!text) continue;
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

#ifdef _DEBUG
void Scene::drawDebugGUI()
{
	//mGame.getGUIDebugger().begin();

	for (auto sprite : mSprites) {
		if (sprite->getActiveControll()) {
			mGame.getGUIDebugger().drawSpriteDebugGUI(*sprite);
		}
	}

	//mGame.getGUIDebugger().end();
}
#endif

