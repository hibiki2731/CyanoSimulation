#include "Scene.h"
#include "Game.h"
#include "SceneManager.h"
#include "MeshComponent.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "GUIDebugger.h"
#include <algorithm>
#include "Object.h"

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

void Scene::createObjects()
{
		//シーンデータの取得
		nlohmann::json sceneJson = mGame.getAssetManager().getSceneJson();
		std::string name = getName();
		std::vector<std::string> objectIDs = sceneJson.at(name).get<std::vector<std::string>>();

		//オブジェクトID配列からオブジェクトを生成
		for (auto objID : objectIDs) {
			auto obj = std::make_unique<Object>(*this, objID);
			addActor(std::move(obj));
		}
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

void Scene::refreshActors()
{
	for (auto& actor : mActors) {
		actor->setState(Actor::State::Dead);
	}

	//デバッグ時、編集用のオブジェクト配列を空にする
#ifdef _DEBUG
	mDebugObjects.clear();
#endif
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

#ifdef _DEBUG
void Scene::drawDebugGUI()
{

	if (mDebugFlag) {
		//フレームレートを可視化
		mGame.getGUIDebugger().draeFrameRate();
		//シアノの位置を可視化
		mGame.getGUIDebugger().drawCyanoHeadPos();
		//オブジェクトの編集
		mGame.getGUIDebugger().drawObjectDebugGUI(mDebugObjects);
	}
}
void Scene::addDebugObject(Object* object)
{
	mDebugObjects.push_back(object);
}

void Scene::removeDebugObject(Object* object)
{
	std::erase_if(mDebugObjects, [object](const Object* o) {
		return o == object;
		});
}

void Scene::clearDebugObject()
{
	mDebugObjects.clear();
}
#endif

