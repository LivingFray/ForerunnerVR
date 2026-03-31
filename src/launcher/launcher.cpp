// Forerunner
#include "launcher.h"
#include "common/utils/inject.h"
// ImGui
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include "imgui_extensions.h"
// System
#include <d3d11.h>
#include <chrono>
#include <filesystem>
#include <shellapi.h>

void Launcher::Initialise(HWND Window, ID3D11Device* InDevice, ID3D11DeviceContext* InContext, IDXGISwapChain* InSwapChain)
{
	Device = InDevice;
	Context = InContext;
	SwapChain = InSwapChain;

	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.08f, 0.13f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_InputTextCursor] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_TreeLines] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_DragDropTargetBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_UnsavedMarker] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

	style.WindowBorderSize = 0.0f;
	style.WindowPadding = ImVec2(5.0f, 5.0f);
	style.FramePadding = ImVec2(5.0f, 5.0f);

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX11_Init(Device, Context);

	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);
	BackBuffer->Release();

	// Find DLL file to inject
	CalculatePayloadPath();
}

void Launcher::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Launcher::Update()
{
	// Handle window resize (we don't resize directly in the WM_SIZE handler)
	if (ResizeWidth != 0 && ResizeHeight != 0)
	{
		if (RenderTargetView)
		{
			RenderTargetView->Release();
		}
		SwapChain->ResizeBuffers(0, ResizeWidth, ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		ResizeWidth = ResizeHeight = 0;

		ID3D11Texture2D* pBackBuffer;
		SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		Device->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
		pBackBuffer->Release();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::Begin("Forerunner", 0, WindowFlags))
	{
		ImGui::BeginChild("Sidebar", ImVec2(400, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
		DrawSidebar();
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("MainPanel");
		switch (ActiveWindow)
		{
			case Launcher::ELauncherWindow::Main:
				DrawMainWindow();
				break;
			case Launcher::ELauncherWindow::HaloCE:
				DrawHaloCEWindow();
				break;
			case Launcher::ELauncherWindow::Settings:
				DrawSettingsWindow();
				break;
			default:
				break;
		}
		ImGui::EndChild();
	}
	ImGui::End();

	ImGui::Render();

	const float clear_color_with_alpha[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	Context->OMSetRenderTargets(1, &RenderTargetView, nullptr);
	Context->ClearRenderTargetView(RenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Launcher::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& OutResult)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		OutResult = 1;
		return true;
	}

	switch (msg)
	{
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
			{
				OutResult = 0;
				return true;
			}
			ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
			ResizeHeight = (UINT)HIWORD(lParam);
			OutResult = 0;
			return true;
	}
	return false;
}

void Launcher::DrawSidebar()
{
	static ImVerticalTabBarState TabState;
	static std::vector<std::string> TabNames = {"Home", "HaloCEVR", "Project Delta", "Settings"};

	if (RenderVerticalTabBar("Sidebar", TabNames, TabState, ImVec2(-1, -1)))
	{
		ActiveWindow = static_cast<ELauncherWindow>(TabState.SelectedIndex);
	}
}

void Launcher::DrawMainWindow()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushFont(nullptr, style.FontSizeBase * 4.0f);
	ImGui::SeparatorText("Forerunner");
	ImGui::PopFont();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
	ImGui::BeginChild("MainWindow_Sub", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);

	RefreshHaloProcess();

	if (bIsHaloRunning)
	{
		ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Halo: Master Chief Collection is running (PID: %d)", LastHaloProcessId);
	}
	else
	{
		ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Halo: Master Chief Collection is not running");
	}

	if (PayloadPath.empty())
	{
		ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Payload DLL not found, check Forerunner was installed correctly");
	}

	if (bAutoLaunch && !bIsLaunching && !bHasAutoLaunched)
	{
		StartLaunch(3.0f);
		bHasAutoLaunched = true;
	}

	ImGui::BeginDisabled(bIsHaloRunning);
	if (bIsLaunching)
	{
		if (ImGui::Button(std::format("Cancel Launch ({}s)", round(LaunchDelay)).c_str()))
		{
			CancelLaunch();
		}
	}
	else
	{
		if (ImGui::Button("Launch Game"))
		{
			StartLaunch(1.0f);
		}
	}
	ImGui::EndDisabled();
	
	if (bIsLaunching)
	{
		UpdateLaunchTime(ImGui::GetIO().DeltaTime);
	}

	const bool bCanInject = bIsHaloRunning && !PayloadPath.empty() && InjectedHaloProcessId != LastHaloProcessId;
	const bool bShouldAutoInject = bAutoInject && bCanInject; // This will be a frame behind from any changes to the Auto Inject checkbox, but that is fine

	ImGui::BeginDisabled(!bCanInject);
	if (ImGui::Button("Inject") || bShouldAutoInject)
	{
		if (Inject::InjectDLL(PayloadPath.c_str(), LastHaloProcessId))
		{
			InjectedHaloProcessId = LastHaloProcessId;
			
			if (bCloseOnInject)
			{
				PostQuitMessage(0);
			}
		}
	}
	ImGui::EndDisabled();

	ImGui::Checkbox("Auto launch", &bAutoLaunch);
	ImGui::Checkbox("Auto inject", &bAutoInject);
	ImGui::Checkbox("Close on inject", &bCloseOnInject);

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void Launcher::DrawHaloCEWindow()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushFont(nullptr, style.FontSizeBase * 4.0f);
	ImGui::SeparatorText("HaloCEVR");
	ImGui::PopFont();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
	ImGui::BeginChild("HaloCEVR_Sub", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
	ImGui::TextWrapped("HaloCEVR is a VR mod for the original PC release of Halo: Combat Evolved, rather than the version included in the Master Chief Collection.");
	ImGui::TextWrapped("More information about how to play this version can be found in the readme on its github page");
	ImGui::TextLinkOpenURL("Download HaloCEVR here", "https://github.com/LivingFray/HaloCEVR");
	ImGui::EndChild();
	ImGui::PopStyleVar();
}


void Launcher::DrawSettingsWindow()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushFont(nullptr, style.FontSizeBase * 4.0f);
	ImGui::SeparatorText("Settings");
	ImGui::PopFont();
}

void Launcher::RefreshHaloProcess()
{
	std::chrono::high_resolution_clock::time_point CurrentTime = std::chrono::high_resolution_clock::now();

	constexpr auto CheckInterval = std::chrono::milliseconds(100);
	static const char* HaloProcessName = "MCC-Win64-Shipping.exe";

	if ((CurrentTime - LastCheckedForHalo) > CheckInterval)
	{
		LastCheckedForHalo = CurrentTime;
		bIsHaloRunning = Inject::FindProcess(HaloProcessName, LastHaloProcessId);
	}
}

void Launcher::CalculatePayloadPath()
{
	// Find Forerunner.dll in the same directory as this .exe
	// Set PayloadPath to the full path, or an empty string if the file doesn't exist
	std::filesystem::path DLLPath = std::filesystem::current_path() / "Forerunner.dll";
	if (std::filesystem::exists(DLLPath))
	{
		PayloadPath = DLLPath.string();
	}
	else
	{
		PayloadPath = "";
	}
}

void Launcher::StartLaunch(float Delay)
{
	bIsLaunching = true;
	LaunchDelay = Delay;
	if (LaunchDelay <= 0.0f)
	{
		LaunchHalo();
	}
}

void Launcher::CancelLaunch()
{
	bIsLaunching = false;
	LaunchDelay = 0.0f;
}

void Launcher::UpdateLaunchTime(float DeltaTime)
{
	if (!bIsLaunching)
	{
		return;
	}

	LaunchDelay -= DeltaTime;

	if (LaunchDelay <= 0.0f)
	{
		LaunchHalo();
	}
}

void Launcher::LaunchHalo()
{
	bIsLaunching = false;
	LaunchDelay = 0.0f;

	// TODO: Support non-steam options (xbox launcher, directly from exe)
	const char* steamCommand = "steam://launch/976730/option2";

	ShellExecuteA(NULL, "open", steamCommand, NULL, NULL, SW_SHOWNORMAL);
}
