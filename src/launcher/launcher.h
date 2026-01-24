#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

class Launcher
{
public:

	void Initialise(HWND Window, struct ID3D11Device* InDevice, struct ID3D11DeviceContext* InContext, struct IDXGISwapChain* InSwapChain);
	void Shutdown();

	void Update();
	bool WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	struct ID3D11Device* Device;
	struct ID3D11DeviceContext* Context;
	struct IDXGISwapChain* SwapChain;
	struct ID3D11RenderTargetView* RenderTargetView;
};