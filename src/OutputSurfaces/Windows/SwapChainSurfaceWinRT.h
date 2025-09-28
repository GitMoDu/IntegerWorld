#ifndef _INTEGER_WORLD_DIRECTX_SWAP_CHAIN_SURFACE_WINRT_h
#define _INTEGER_WORLD_DIRECTX_SWAP_CHAIN_SURFACE_WINRT_h


#if defined(WINRT_ASSERT)

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Core.h>

#include <windows.ui.xaml.media.dxinterop.h>

#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

#include "../../Framework/Interface.h"
#include "SwapChainDirectX.h"

#include <windows.h>
#include <Unknwn.h>
#include <dxgi1_2.h>

namespace IntegerWorld
{
	namespace SwapChainDirectX
	{

		template<int16_t SurfaceWidth, int16_t SurfaceHeight>
		class SwapChainSurface : public IOutputSurface
		{
		private:
			winrt::Windows::UI::Xaml::Controls::SwapChainPanel m_swapChainPanel{};

		protected:

			winrt::event_token m_sizeChangedToken;

			Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext;
			Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

			Microsoft::WRL::ComPtr<ID3D11Query> frameQuery;
			bool frameQueryIssued = false;

			std::vector<uint32_t> frameBuffer;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> frameTexture;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> frameSRV;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

			struct Vertex {
				float position[3];
				float texcoord[2];
			};

			static constexpr Vertex quadVertices[4]
			{
				{ {-1, -1, 0}, {0, 1} },
				{ {-1,  1, 0}, {0, 0} },
				{ { 1, -1, 0}, {1, 1} },
				{ { 1,  1, 0}, {1, 0} }
			};


			std::mutex surfaceMutex;
			std::condition_variable surfaceCv;

			std::mutex m_swapChainPanelMutex;

		public:
			SwapChainSurface()
				: IOutputSurface()
			{
				frameBuffer.resize(SurfaceWidth * SurfaceHeight, 0xFF000000);
			}

			void SetSwapChainPanel(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel& panel)
			{
				std::lock_guard<std::mutex> lock(m_swapChainPanelMutex);

				if (m_swapChainPanel != nullptr)
				{
					m_swapChainPanel.SizeChanged(m_sizeChangedToken);
				}

				m_swapChainPanel = panel;

				m_sizeChangedToken = m_swapChainPanel.SizeChanged(
					winrt::Windows::UI::Xaml::SizeChangedEventHandler(
						[this](winrt::Windows::Foundation::IInspectable const& inner, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& events)
						{
							std::lock_guard<std::mutex> panelLock(m_swapChainPanelMutex);
							ResizeSwapChain();
						}
					));

			}

			void OnSwapChainPanelSizeChanged(
				winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender,
				winrt::Windows::UI::Xaml::SizeChangedEventArgs const& e)
			{
				ResizeSwapChain();
			}

		public: // Buffer management interface.
			bool StartSurface() final
			{
				std::lock_guard<std::mutex> panelLock(m_swapChainPanelMutex);
				if (m_swapChainPanel == nullptr)
					return false;

				bool setupOk = D3dCreateDeviceAndSwapChain();
				OutputDebugStringA(setupOk ? "D3D OK\n" : "D3D FAIL\n");
				if (setupOk)
				{
					setupOk &= CreateFrameResources();

					{
						std::unique_lock<std::mutex> lock(surfaceMutex);
						surfaceCv.wait_for(lock, std::chrono::seconds(2), [&setupOk] { return setupOk; });
					}
				}

				return setupOk;
			}

			void StopSurface() final
			{
				// Release DirectX resources
				renderTargetView.Reset();
				swapChain.Reset();
				d3dContext.Reset();
				d3dDevice.Reset();
				frameTexture.Reset();
				frameSRV.Reset();
				vertexShader.Reset();
				pixelShader.Reset();
				inputLayout.Reset();
				vertexBuffer.Reset();
				samplerState.Reset();
				frameQuery.Reset();

				// Reset synchronization and state flags
				frameQueryIssued = false;
				frameBuffer.clear();

				// Optionally, unsubscribe from events
				{
					std::lock_guard<std::mutex> lock(m_swapChainPanelMutex);
					if (m_swapChainPanel)
					{
						m_swapChainPanel.SizeChanged(m_sizeChangedToken);
						m_swapChainPanel = nullptr;
					}
				}
			}

			void FlipSurface() final
			{
				UploadFrameBuffer();
				RenderFrameTexture();

				auto asyncOp = m_swapChainPanel.Dispatcher().TryRunAsync(
					winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
					winrt::Windows::UI::Core::DispatchedHandler([this]
						{
							if (swapChain && frameQuery)
							{
								swapChain->Present(0, 0);
							}
							{
								std::lock_guard<std::mutex> lock(surfaceMutex);
							}
							surfaceCv.notify_one();
						})
				);

				{
					std::unique_lock<std::mutex> lock(surfaceMutex);
					surfaceCv.wait(lock);
				}
			}

			bool IsSurfaceReady() final
			{
				if (!d3dDevice || !d3dContext || !swapChain || !renderTargetView)
					return false;

				if (frameQueryIssued && frameQuery)
				{
					HRESULT hr = d3dContext->GetData(frameQuery.Get(), nullptr, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH);
					return hr == S_OK;
				}
				else
				{
					d3dContext->End(frameQuery.Get());
					frameQueryIssued = true;
				}

				return false;
			}

		public: // Buffer window interface.
			void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
			{
				if (swapChain != nullptr)
				{
					width = SurfaceWidth;
					height = SurfaceHeight;
					colorDepth = 32; // 32-bit color depth (BGRA8)
				}
				else
				{
					width = 0;
					height = 0;
					colorDepth = 0;
				}
			}

		private:
			void UploadFrameBuffer()
			{
				D3D11_MAPPED_SUBRESOURCE mapped;
				HRESULT hr = d3dContext->Map(frameTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				if (SUCCEEDED(hr)) {
					memcpy(mapped.pData, frameBuffer.data(), frameBuffer.size() * sizeof(uint32_t));
					d3dContext->Unmap(frameTexture.Get(), 0);
				}
			}

			void RenderFrameTexture()
			{
				d3dContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

				D3D11_VIEWPORT vp = {};
				vp.Width = static_cast<float>(SurfaceWidth);
				vp.Height = static_cast<float>(SurfaceHeight);
				vp.MinDepth = 0.0f;
				vp.MaxDepth = 1.0f;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				d3dContext->RSSetViewports(1, &vp);

				d3dContext->VSSetShader(vertexShader.Get(), nullptr, 0);
				d3dContext->PSSetShader(pixelShader.Get(), nullptr, 0);
				d3dContext->IASetInputLayout(inputLayout.Get());

				UINT stride = sizeof(Vertex);
				UINT offset = 0;
				d3dContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
				d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				d3dContext->PSSetShaderResources(0, 1, frameSRV.GetAddressOf());
				d3dContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

				d3dContext->Draw(4, 0);
			}

			bool D3dCreateDeviceAndSwapChain()
			{
				UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				D3D_FEATURE_LEVEL featureLevels[] = {
					D3D_FEATURE_LEVEL_11_1,
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
				};

				Microsoft::WRL::ComPtr<ID3D11Device> baseDevice;
				Microsoft::WRL::ComPtr<ID3D11DeviceContext> baseContext;
				D3D_FEATURE_LEVEL featureLevel;

				HRESULT hr = D3D11CreateDevice(
					nullptr,
					D3D_DRIVER_TYPE_HARDWARE,
					0,
					creationFlags,
					featureLevels,
					ARRAYSIZE(featureLevels),
					D3D11_SDK_VERSION,
					&baseDevice,
					&featureLevel,
					&baseContext
				);
				if (FAILED(hr)) { OutputDebugStringA("D3D11CreateDevice failed\n"); return false; }

				hr = baseDevice.As(&d3dDevice);
				if (FAILED(hr)) { OutputDebugStringA("baseDevice.As failed\n"); return false; }
				hr = baseContext.As(&d3dContext);
				if (FAILED(hr)) { OutputDebugStringA("baseContext.As failed\n"); return false; }

				Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
				hr = d3dDevice.As(&dxgiDevice);
				if (FAILED(hr)) { OutputDebugStringA("d3dDevice.As IDXGIDevice1 failed\n"); return false; }

				Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
				hr = dxgiDevice->GetAdapter(&dxgiAdapter);
				if (FAILED(hr)) { OutputDebugStringA("GetAdapter failed\n"); return false; }

				Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
				hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));
				if (FAILED(hr)) { OutputDebugStringA("GetParent IDXGIFactory2 failed\n"); return false; }

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.Width = SurfaceWidth;
				swapChainDesc.Height = SurfaceHeight;
				swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				swapChainDesc.Stereo = false;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				swapChainDesc.Flags = 0;

				hr = dxgiFactory->CreateSwapChainForComposition(
					dxgiDevice.Get(),
					&swapChainDesc,
					nullptr,
					&swapChain
				);
				if (FAILED(hr)) { OutputDebugStringA("CreateSwapChainForComposition failed\n"); return false; }

				IDXGISwapChain* baseSwapChain = reinterpret_cast<IDXGISwapChain*>(swapChain.GetAddressOf());

				// capture swapChain into a local ComPtr to use in the UI thread
				auto swapCopy = swapChain; // ComPtr<IDXGISwapChain1>

				// Run QueryInterface/SetSwapChain on the UI thread
				m_swapChainPanel.Dispatcher().RunAsync(
					winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
					winrt::Windows::UI::Core::DispatchedHandler([this, swapCopy]()
						{
							Microsoft::WRL::ComPtr<ISwapChainPanelNative> panelNative;
							::IUnknown* nativeInterface = reinterpret_cast<::IUnknown*>(winrt::get_abi(m_swapChainPanel));
							HRESULT hr = nativeInterface->QueryInterface(__uuidof(ISwapChainPanelNative),
								reinterpret_cast<void**>(panelNative.GetAddressOf()));
							if (SUCCEEDED(hr) && panelNative)
							{
								// pass the real COM pointer (Get()).
								panelNative->SetSwapChain(static_cast<IDXGISwapChain*>(swapCopy.Get()));
								// panelNative will be released automatically when ComPtr goes out of scope
							}
						}));


				Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
				hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
				if (FAILED(hr)) { OutputDebugStringA("GetBuffer failed\n"); return false; }

				hr = d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
				if (FAILED(hr)) { OutputDebugStringA("CreateRenderTargetView failed\n"); return false; }

				D3D11_QUERY_DESC queryDesc = {};
				queryDesc.Query = D3D11_QUERY_EVENT;
				queryDesc.MiscFlags = 0;
				Microsoft::WRL::ComPtr<ID3D11Query> query;
				hr = d3dDevice->CreateQuery(&queryDesc, &query);
				if (SUCCEEDED(hr)) {
					frameQuery = query;
				}

				return true;
			}

			bool CreateFrameResources()
			{
				frameBuffer.resize(SurfaceWidth * SurfaceHeight, 0xFF000000);

				D3D11_TEXTURE2D_DESC desc = {};
				desc.Width = SurfaceWidth;
				desc.Height = SurfaceHeight;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = frameBuffer.data();
				initData.SysMemPitch = SurfaceWidth * sizeof(uint32_t);

				HRESULT hr = d3dDevice->CreateTexture2D(&desc, &initData, &frameTexture);
				if (FAILED(hr)) { return false; }

				d3dDevice->CreateShaderResourceView(frameTexture.Get(), nullptr, &frameSRV);

				D3D11_SAMPLER_DESC samplerDesc = {};
				samplerDesc.Filter = D3D11_FILTER_MAXIMUM_ANISOTROPIC;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.MipLODBias = 0;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDesc.BorderColor[0] = 0;
				samplerDesc.BorderColor[1] = 0;
				samplerDesc.BorderColor[2] = 0;
				samplerDesc.BorderColor[3] = 0;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

				d3dDevice->CreateSamplerState(&samplerDesc, &samplerState);

				Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
				D3DCompile(SwapChainDirectX::vsSource, strlen(SwapChainDirectX::vsSource), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vsBlob, nullptr);
				d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);

				Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
				D3DCompile(SwapChainDirectX::psSource, strlen(SwapChainDirectX::psSource), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &psBlob, nullptr);
				d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

				D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
				};
				d3dDevice->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

				D3D11_BUFFER_DESC vbDesc = {};
				vbDesc.Usage = D3D11_USAGE_DEFAULT;
				vbDesc.ByteWidth = sizeof(quadVertices);
				vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				D3D11_SUBRESOURCE_DATA vbData = {};
				vbData.pSysMem = quadVertices;

				hr = d3dDevice->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
				if (FAILED(hr)) { return false; }

				return true;
			}

			void ResizeSwapChain()
			{
				if (swapChain)
				{
					auto width = static_cast<UINT>(m_swapChainPanel.ActualWidth());
					auto height = static_cast<UINT>(m_swapChainPanel.ActualHeight());

					renderTargetView.Reset();

					HRESULT hr = swapChain->ResizeBuffers(
						2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
					if (SUCCEEDED(hr))
					{
						Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
						hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
						if (SUCCEEDED(hr))
						{
							d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
						}
					}
				}
			}
		};
	}
}
#endif
#endif