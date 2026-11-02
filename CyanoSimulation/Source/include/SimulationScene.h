#pragma once
#include "Scene/Scene.h"
#include <d3dx12.h>
using Microsoft::WRL::ComPtr;

class SimulationScene :
    public Scene
{
public:
    SimulationScene(Game& game);

	void onEnter() override;
	void onExit() override;

	void drawScene() override;
	const std::string getName() const override { return "SIMULATION"; }

private:
	void createCyanoGraphicsPSO(ID3D12Device& device);

	class CyanoSimulator* mSimulator;
	ComPtr<ID3D12RootSignature> mCyanoGraphicsRootSignature;
	ComPtr<ID3D12PipelineState> mCyanoGraphicsPSO;
};

