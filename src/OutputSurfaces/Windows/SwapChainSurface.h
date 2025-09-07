#ifndef _INTEGER_WORLD_DIRECTX_SWAP_CHAIN_SURFACE_h
#define _INTEGER_WORLD_DIRECTX_SWAP_CHAIN_SURFACE_h

#include "DirectX.h"

#include "../../Framework/Interface.h"
#include <wrl/client.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <windows.ui.xaml.controls.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <d3dcompiler.h>
#include <mutex>
#include <condition_variable>

namespace IntegerWorld
{
	namespace DirectX
	{
		template<int16_t SurfaceWidth, int16_t SurfaceHeight>
		class SwapChainSurface : public IOutputSurface
		{
		protected:
			Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel = nullptr;

			// DirectX device and swap chain members
		protected:
			Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext;
			Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

			// Event token for size changed event
		protected:
			Windows::Foundation::EventRegistrationToken sizeChangedToken;

			// Query for frame timing
		private:
			Microsoft::WRL::ComPtr<ID3D11Query> frameQuery;
			bool frameQueryIssued = false;

		protected:
			std::vector<uint32_t> frameBuffer;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> frameTexture;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> frameSRV;

			// Additional members for shaders and full-screen quad
		protected:
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

			// Vertex structure for a full-screen quad
			struct Vertex {
				float position[3];
				float texcoord[2];
			};

			// Full-screen quad vertices
			static constexpr Vertex quadVertices[4]
			{
				{ {-1, -1, 0}, {0, 1} },
				{ {-1,  1, 0}, {0, 0} },
				{ { 1, -1, 0}, {1, 1} },
				{ { 1,  1, 0}, {1, 0} }
			};

			std::mutex surfaceMutex;
			std::condition_variable surfaceCv;

		public:
			SwapChainSurface() : IOutputSurface()
			{
				frameBuffer.resize(SurfaceWidth * SurfaceHeight, 0xFF000000);
			}

			// Set the SwapChainPanel reference from your UI code
			void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel)
			{
				// Unsubscribe previous handler if any
				if (swapChainPanel != nullptr)
				{
					swapChainPanel->SizeChanged -= sizeChangedToken;
				}

				swapChainPanel = panel;

				// Subscribe to the SizeChanged event
				sizeChangedToken = swapChainPanel->SizeChanged +=
					ref new Windows::UI::Xaml::SizeChangedEventHandler(
						[this](Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
						{
							ResizeSwapChain();
						});
			}

			void OnSwapChainPanelSizeChanged(
				Windows::UI::Xaml::Controls::SwapChainPanel^ sender,
				Windows::UI::Xaml::SizeChangedEventArgs^ e)
			{
				ResizeSwapChain();
			}

		public:// Buffer managment interface.
			bool StartSurface() final
			{
				if (!swapChainPanel)
					return false;

				bool started = false;

				swapChainPanel->Dispatcher->TryRunAsync(
					Windows::UI::Core::CoreDispatcherPriority::Normal,
					ref new Windows::UI::Core::DispatchedHandler([this, &started]()
						{
							started = D3dCreateDeviceAndSwapChain();
							if (started)
							{
								CreateFrameResources();
							}
							{
								std::lock_guard<std::mutex> lock(surfaceMutex);
							}
							surfaceCv.notify_one();
						})
				);

				{
					std::unique_lock<std::mutex> lock(surfaceMutex);
					surfaceCv.wait(lock, [&started] { return started; });
				}

				return started;
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
				if (swapChainPanel != nullptr)
				{
					swapChainPanel->SizeChanged -= sizeChangedToken;
					swapChainPanel = nullptr;
				}
			}

			/// <summary>
			/// Flips the rendering surface by presenting the current frame and synchronizing with the UI thread.
			/// </summary>
			void FlipSurface() final
			{
				UploadFrameBuffer();
				RenderFrameTexture();

				swapChainPanel->Dispatcher->TryRunAsync(
					Windows::UI::Core::CoreDispatcherPriority::Normal,
					ref new Windows::UI::Core::DispatchedHandler([this]()
						{
							if (swapChain && frameQuery)
							{
								// V-Sync is not needed as a new frame will only start drawing after IsSurfaceReady() returns true.
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

			/// <summary>
			/// </summary>
			/// <returns>Returns true if the Direct3D rendering surface is ready for drawing.</returns>
			bool IsSurfaceReady() final
			{
				if (!d3dDevice || !d3dContext || !swapChain || !renderTargetView)
					return false;

				if (frameQueryIssued && frameQuery)
				{
					HRESULT hr = d3dContext->GetData(frameQuery.Get(), nullptr, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH);
					if (hr == S_OK)
					{
						d3dContext->End(frameQuery.Get());
						frameQueryIssued = true;
						return true;
					}
					else if (hr == S_FALSE)
					{
						// GPU still busy
					}
					else
					{
						// Query error, treat as not ready
					}
				}
				else
				{

					d3dContext->End(frameQuery.Get());
					frameQueryIssued = true;
				}

				return false;
			}

		public:// Buffer window interface.
			void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
			{
				if (swapChain != nullptr)
				{
					//width = static_cast<int16_t>(swapChainPanel->ActualWidth);
					//height = static_cast<int16_t>(swapChainPanel->ActualHeight);
					width = SurfaceWidth;
					height = SurfaceHeight;
					colorDepth = 32; // Assuming 32-bit color depth (BGRA8)
				}
				else
				{

					width = 0;
					height = 0;
					colorDepth = 0;
				}
			}

		private:
			// Call this in FlipSurface after rendering
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
				// Create D3D11 device and context
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

				if (FAILED(hr))
					return false;

				// Query for ID3D11Device1 and ID3D11DeviceContext1
				hr = baseDevice.As(&d3dDevice);
				if (FAILED(hr)) return false;
				hr = baseContext.As(&d3dContext);
				if (FAILED(hr)) return false;

				// Get DXGI device
				Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
				hr = d3dDevice.As(&dxgiDevice);
				if (FAILED(hr)) return false;

				// Get DXGI adapter
				Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
				hr = dxgiDevice->GetAdapter(&dxgiAdapter);
				if (FAILED(hr)) return false;

				// Get DXGI factory
				Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
				hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);
				if (FAILED(hr)) return false;

				// Describe swap chain
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.Width = swapChainPanel->Width; // Use panel size
				swapChainDesc.Height = swapChainPanel->Height; // Use panel size
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

				// Create swap chain for SwapChainPanel
				hr = dxgiFactory->CreateSwapChainForComposition(
					dxgiDevice.Get(),
					&swapChainDesc,
					nullptr,
					&swapChain
				);

				if (FAILED(hr)) return false;

				// Associate swap chain with SwapChainPanel
				Microsoft::WRL::ComPtr<ISwapChainPanelNative> panelNative;
				hr = reinterpret_cast<IUnknown*>(swapChainPanel)->QueryInterface(__uuidof(ISwapChainPanelNative), &panelNative);
				if (FAILED(hr)) return false;

				hr = panelNative->SetSwapChain(swapChain.Get());
				if (FAILED(hr)) return false;

				// Create render target view
				Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
				hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
				if (FAILED(hr)) return false;

				hr = d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
				if (FAILED(hr)) return false;

				// After d3dDevice and d3dContext are initialized
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

			void CreateFrameResources()
			{
				frameBuffer.resize(SurfaceWidth * SurfaceHeight, 0xFF000000); // Opaque black

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
				if (FAILED(hr)) { /* handle error */ }

				d3dDevice->CreateShaderResourceView(frameTexture.Get(), nullptr, &frameSRV);

				// Compile and create shaders, input layout, and vertex buffer
				// You can compile the HLSL shaders at build time or runtime
				// For runtime compilation, use D3DCompile from d3dcompiler.h

				// Create sampler state
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

				// Compile and create vertex shader
				Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
				D3DCompile(DirectX::vsSource, strlen(DirectX::vsSource), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vsBlob, nullptr);
				d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);

				// Compile and create pixel shader
				Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
				D3DCompile(DirectX::psSource, strlen(DirectX::psSource), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &psBlob, nullptr);
				d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

				// Create input layout
				D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
				};
				d3dDevice->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

				// Create vertex buffer for full-screen quad
				D3D11_BUFFER_DESC vbDesc = {};
				vbDesc.Usage = D3D11_USAGE_DEFAULT;
				vbDesc.ByteWidth = sizeof(quadVertices);
				vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				D3D11_SUBRESOURCE_DATA vbData = {};
				vbData.pSysMem = quadVertices;
				d3dDevice->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
			}

			void ResizeSwapChain()
			{
				if (swapChain)
				{
					// Get new size from swapChainPanel
					auto width = static_cast<UINT>(swapChainPanel->ActualWidth);
					auto height = static_cast<UINT>(swapChainPanel->ActualHeight);

					// Release old render target
					renderTargetView.Reset();

					// Resize swap chain buffers
					HRESULT hr = swapChain->ResizeBuffers(
						2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
					if (SUCCEEDED(hr))
					{
						// Recreate render target view
						Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
						hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
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