#pragma once
#include "pch.h"

class Graphic;

class FadeGenerator
{
public:
	FadeGenerator(Graphic& graphic);

	//---フェードイン、フェードアウト---
	void startFadeIn(float duration);
	void startFadeOut(float duration);
	void renderFade();	//画面を覆う三角形のα値を制御してフェードイン、フェードアウトを実現する。描画処理の最後に呼び出す必要がある。

	bool isFading();
	bool isFinishedFade();

private:
	Graphic& mGraphic;

	//フェード処理
	float currentFadeAlpha;
	float fadeTimer;
	float fadeOutDuration;
	float fadeInDuration;
	bool isFadingIn;
	bool isFadingOut;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureFade;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineStateFade;
};

