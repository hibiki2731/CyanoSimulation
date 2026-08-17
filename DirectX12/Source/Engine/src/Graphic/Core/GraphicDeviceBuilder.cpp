#include "pch.h"
#include "Graphic/Core/GraphicDeviceBuilder.h"

ComPtr<ID3D12Device> GraphicDeviceBuilder::build()
{
	//返すID3D12Deviceを初期化
	ComPtr<ID3D12Device> device;

	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	{
		//デバッグレイヤーをオンに
		ComPtr<ID3D12Debug> debug;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		assert(SUCCEEDED(hr));
		debug->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	//対応する機能レベルの配列を用意
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	//デバイスを作成するためのfactoryを作成
	ComPtr<IDXGIFactory6> factory;
	HRESULT factoryHr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
	assert(SUCCEEDED(factoryHr));

	ComPtr<IDXGIAdapter1> adapter;
	//性能の良いハードウェアアダプタから取得して、DirectX12に対応しているか確認。対応しているアダプタが見つかったらループを抜ける。
	for (int adapterIndex = 0;
		SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
		adapterIndex++)
	{

		bool isDeviceCreated = false;
		for (auto level : featureLevels) {
			//実際にデバイスを作成せず、サポートしているかのみ確認
			HRESULT hr = D3D12CreateDevice(adapter.Get(), level, _uuidof(ID3D12Device), nullptr);

			if (SUCCEEDED(hr)) {
				//サポートが確認できたら、実際にデバイスを作成してループを抜ける
				hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(device.GetAddressOf()));
				assert(SUCCEEDED(hr));
				isDeviceCreated = true;
				break;
			}
		}

		//デバイスが作成できていたらループを抜ける
		if (isDeviceCreated) break;
	}

#ifdef _DEBUG
	//誤検知エラーを無視するフィルタの作成
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_REFLECTSHAREDPROPERTIES_INVALIDOBJECT
		};

		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;

		//フィルタを適用
		infoQueue->PushStorageFilter(&filter);
	}

#endif
	return device;
}
