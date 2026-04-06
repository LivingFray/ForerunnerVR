#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <chrono>

class Launcher
{
public:

	void Initialise(HWND Window, struct ID3D11Device* InDevice, struct ID3D11DeviceContext* InContext, struct IDXGISwapChain* InSwapChain);
	void Shutdown();

	void Update();
	bool WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& OutResult);

private:
	enum class ELauncherWindow
	{
		Main,
		HaloCE,
		Delta,
		Settings
	};

	void DrawSidebar();
	void DrawMainWindow();
	void DrawHaloCEWindow();
	void DrawSettingsWindow();

	void RefreshHaloProcess();
	void CalculatePayloadPath();

	void StartLaunch(float Delay);
	void CancelLaunch();
	void UpdateLaunchTime(float DeltaTime);
	void LaunchHalo();

	struct ImFont* HaloFont = nullptr;

	ELauncherWindow ActiveWindow = ELauncherWindow::Main;

	struct ID3D11Device* Device;
	struct ID3D11DeviceContext* Context;
	struct IDXGISwapChain* SwapChain;
	struct ID3D11RenderTargetView* RenderTargetView;
	UINT ResizeWidth = 0;
	UINT ResizeHeight = 0;

	bool bIsHaloRunning = false;
	DWORD LastHaloProcessId = 0;
	DWORD InjectedHaloProcessId = 0;
	std::chrono::high_resolution_clock::time_point LastCheckedForHalo;

	std::string PayloadPath;

	bool bHasAutoLaunched = false;
	bool bIsLaunching = false;
	float LaunchDelay = 0.0f;
};