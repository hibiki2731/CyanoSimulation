#pragma once
#include "SpriteComponent.h"

class Anime2DComponent :
    public SpriteComponent
{
public:
	Anime2DComponent(Actor* owner, float zDepth = 100.0f);
	~Anime2DComponent() {};

	virtual void create(const std::string filename, int textureNum);
	virtual void draw() override;

	void endProccess() override;


	void setTextureIndex(int index);

	int getTextureNum() { return mTextureNum; };

private:
	int mTextureIndex;
	int mTextureNum;

	std::vector<ID3D12Resource*> mTextureBufs;

};

