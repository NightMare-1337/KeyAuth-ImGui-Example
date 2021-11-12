#include <iostream>
#include "api/KeyAuth.hpp"
#include "xorstr.hpp"
#include <tlhelp32.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <windows.h>
#include <thread>
#include <random>

#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

#pragma comment(lib, "D3dx9")

IDirect3DTexture9* masterlogo;

#include "others/ImGui/imgui.h"
#include "others/ImGui/imgui_impl_dx9.h"
#include "others/ImGui/imgui_impl_win32.h"

using namespace KeyAuth;

/*
*
*
* WATCH THIS VIDEO FOR SETUP TUTORIAL: https://youtube.com/watch?v=uJ0Umy_C6Fg
* DO NOT CONTACT DISMAIL WITHOUT WATCHING VIDEO FIRST
*
*/

std::string name = XorStr("");
std::string ownerid = XorStr("");
std::string secret = XorStr("");
std::string version = XorStr("1.0");

api KeyAuthApp(name, ownerid, secret, version);

bool loader_active = true;

static int width = 350;
static int height = 200;

char PassWord[20] = "";
char Licence[50] = "";
char UserName[20] = "";
char RgPassWord[20] = "";
char RgUserName[20] = "";

HWND main_hwnd = nullptr;

LPDIRECT3DDEVICE9        g_pd3dDevice;
D3DPRESENT_PARAMETERS    g_d3dpp;
LPDIRECT3D9              g_pD3D;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;
	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

bool LoginCheck = false;

void InfLog()
{
	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);
	auto x = float(screen_rect.right - width) / 2.f;
	auto y = float(screen_rect.bottom - height) / 2.f;

	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 70));
	ImGui::Begin("User Information", &loader_active, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
	{
		ImGui::Columns(1, NULL, true);
		{
			ImGui::Text("UserName : %s", KeyAuthApp.user_data.username);

			time_t currtime;
			struct tm* tminfo;
			time(&currtime);
			tminfo = localtime(&currtime);

			std::time_t x = std::mktime(tminfo);
			std::time_t y = std::mktime(&KeyAuthApp.user_data.expiry);
			if (x != (std::time_t)(-1) && y != (std::time_t)(-1))
			{
				double difference = std::difftime(y, x) / (60 * 60 * 24);
				ImGui::Text("%f day(s) left", difference);
			}

			ImGui::EndChild();
		}
	}
	ImGui::End();
}

int APIENTRY WindownsMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL,L"Loaders", NULL };
	RegisterClassEx(&wc);
	main_hwnd = CreateWindow(wc.lpszClassName, L"Loaders", WS_POPUP, 0, 0, 5, 5, NULL, NULL, wc.hInstance, NULL);

	if (!CreateDeviceD3D(main_hwnd)) {
		CleanupDeviceD3D();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}
	ShowWindow(main_hwnd, SW_HIDE);
	UpdateWindow(main_hwnd);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
	{
		return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
	};

	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
	const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
	const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

	const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
	const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
	const ImVec4 panelActiveColor = ColorFromBytes(0, 224, 255);

	const ImVec4 textColor = ColorFromBytes(255, 255, 255);
	const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
	const ImVec4 borderColor = ColorFromBytes(78, 78, 78);

	colors[ImGuiCol_Text] = textColor;
	colors[ImGuiCol_TextDisabled] = textDisabledColor;
	colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
	colors[ImGuiCol_WindowBg] = bgColor;
	colors[ImGuiCol_ChildBg] = ColorFromBytes(121, 121, 121);;
	colors[ImGuiCol_PopupBg] = bgColor;
	colors[ImGuiCol_Border] = borderColor;
	colors[ImGuiCol_BorderShadow] = borderColor;
	colors[ImGuiCol_FrameBg] = panelColor;
	colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
	colors[ImGuiCol_FrameBgActive] = panelActiveColor;
	colors[ImGuiCol_TitleBg] = bgColor;
	colors[ImGuiCol_TitleBgActive] = bgColor;
	colors[ImGuiCol_TitleBgCollapsed] = bgColor;
	colors[ImGuiCol_MenuBarBg] = panelColor;
	colors[ImGuiCol_ScrollbarBg] = panelColor;
	colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
	colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
	colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
	colors[ImGuiCol_CheckMark] = panelActiveColor;
	colors[ImGuiCol_SliderGrab] = panelHoverColor;
	colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
	colors[ImGuiCol_Button] = panelColor;
	colors[ImGuiCol_ButtonHovered] = panelHoverColor;
	colors[ImGuiCol_ButtonActive] = panelHoverColor;
	colors[ImGuiCol_Header] = panelColor;
	colors[ImGuiCol_HeaderHovered] = panelHoverColor;
	colors[ImGuiCol_HeaderActive] = panelActiveColor;
	colors[ImGuiCol_Separator] = borderColor;
	colors[ImGuiCol_SeparatorHovered] = borderColor;
	colors[ImGuiCol_SeparatorActive] = borderColor;
	colors[ImGuiCol_ResizeGrip] = bgColor;
	colors[ImGuiCol_ResizeGripHovered] = panelColor;
	colors[ImGuiCol_ResizeGripActive] = lightBgColor;
	colors[ImGuiCol_PlotLines] = panelActiveColor;
	colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
	colors[ImGuiCol_PlotHistogram] = panelActiveColor;
	colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
	colors[ImGuiCol_DragDropTarget] = bgColor;
	colors[ImGuiCol_NavHighlight] = bgColor;
	colors[ImGuiCol_DockingPreview] = panelActiveColor;
	colors[ImGuiCol_Tab] = bgColor;
	colors[ImGuiCol_TabActive] = panelActiveColor;
	colors[ImGuiCol_TabUnfocused] = bgColor;
	colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
	colors[ImGuiCol_TabHovered] = panelHoverColor;

	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;

	ImGui_ImplWin32_Init(main_hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	DWORD window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);
	auto x = float(screen_rect.right - width) / 2.f;
	auto y = float(screen_rect.bottom - height) / 2.f;

	static int Tabs = 2;

	bool InfWindow = false;

	KeyAuthApp.init();

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT && !LoginCheck)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(width, height));
			ImGui::SetNextWindowBgAlpha(1.0f);

			ImGui::Begin("Key Auth Example", &loader_active, window_flags);
			{
				if (ImGui::Button("Regster", ImVec2(100, 20)))
				{
					Tabs = 1;
				}

				ImGui::SameLine();

				if (ImGui::Button("Login", ImVec2(100, 20)))
				{
					Tabs = 2;
				}

				if (Tabs == 1)
				{
					ImGui::Text("Please Enter Username & Password & License");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::InputText("UserName##rg", RgUserName, IM_ARRAYSIZE(RgUserName));

					ImGui::InputText("PassWord##rg", RgPassWord, IM_ARRAYSIZE(RgPassWord));

					ImGui::InputText("Licence##rg", Licence, IM_ARRAYSIZE(Licence));

					if (ImGui::Button("Regst", ImVec2(320, 25)))
					{
						KeyAuthApp.regstr(RgUserName, RgPassWord, Licence);

						MessageBox(NULL, TEXT("Regst successful!"), TEXT("Regst"), MB_OK);
					}
				}

				if (Tabs == 2)
				{
					ImGui::Text("Please Enter Username & Password");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::InputText("UserName##rg", UserName, IM_ARRAYSIZE(UserName));

					ImGui::InputText("PassWord##rg", PassWord, IM_ARRAYSIZE(PassWord), ImGuiInputTextFlags_Password);

					if(ImGui::Button("Login##Log", ImVec2(320, 25)))
					{
						KeyAuthApp.login(UserName, PassWord);

						MessageBox(NULL, TEXT("Login successful!"),TEXT("Login"), MB_OK);

						InfWindow = true;
					}
				}
			}
			ImGui::End();

			if (InfWindow)
			{
				InfLog();
			}
		}
		ImGui::EndFrame();

		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
			ResetDevice();
		}
		if (!loader_active) {
			msg.message = WM_QUIT;
		}
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupDeviceD3D();
//	DestroyWindow(main_hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
//	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR lpCmdLine,int nCmdShow)
{

//	SetConsoleTitleA(XorStr("Loader").c_str());
//	std::cout << XorStr("\n\n Connecting..");

	WindownsMain(0, 0, 0, 0);

//	MessageBox(NULL, TEXT("Login successful! %d"), TEXT("Login"), MB_OK);

	//KeyAuthApp.init();
	//system(XorStr("cls").c_str());
	//
	//std::cout << XorStr("\n\n [1] Login\n [2] Register\n [3] Upgrade\n [4] License key only\n\n Choose option: ");

	//int option;
	//std::string username;
	//std::string password;
	//std::string key;

	//std::cin >> option;
	//switch (option)
	//{
	//	case 1:
	//		std::cout << XorStr("\n\n Enter username: ");
	//		std::cin >> username;
	//		std::cout << XorStr("\n Enter password: ");
	//		std::cin >> password;
	//		KeyAuthApp.login(username, password);
	//		break;
	//	case 2:
	//		std::cout << XorStr("\n\n Enter username: ");
	//		std::cin >> username;
	//		std::cout << XorStr("\n Enter password: ");
	//		std::cin >> password;
	//		std::cout << XorStr("\n Enter license: ");
	//		std::cin >> key;
	//		KeyAuthApp.regstr(username,password,key);
	//		break;
	//	case 3:
	//		std::cout << XorStr("\n\n Enter username: ");
	//		std::cin >> username;
	//		std::cout << XorStr("\n Enter license: ");
	//		std::cin >> key;
	//		KeyAuthApp.upgrade(username, key);
	//		break;
	//	case 4:
	//		std::cout << XorStr("\n Enter license: ");
	//		std::cin >> key;
	//		KeyAuthApp.license(key);
	//		break;
	//	default:
	//		std::cout << XorStr("\n\n Status: Failure: Invalid Selection");
	//		Sleep(3000);
	//		exit(0);
	//}
	
	/*
	// download file
    std::vector<std::uint8_t> bytes = KeyAuthApp.download("123456");
    std::ofstream file("file.exe", std::ios_base::out | std::ios_base::binary);
    file.write((char*)bytes.data(), bytes.size());
    file.close();
	*/
	
	// KeyAuthApp.log("user logged in"); // send event to logs. if you set discord webhook in app settings, it will send there too
	// KeyAuthApp.webhook("HDb5HiwOSM", "&type=black&ip=1.1.1.1&hwid=abc"); // webhook request to securely send GET request to API, here's what it looks like on dashboard https://i.imgur.com/jW74Hwe.png
	// KeyAuthApp.ban(); // ban the current user, must be logged in

	//#pragma region
	//time_t rawtime = mktime(&KeyAuthApp.user_data.expiry);
	//struct tm* timeinfo;
	//timeinfo = localtime(&rawtime);
	//printf(XorStr("\n Your Subscription Expires At: %s").c_str(), asctime(timeinfo));
	//
	//time_t currtime;
	//struct tm* tminfo;
	//time(&currtime);
	//tminfo = localtime(&currtime);

	//std::time_t x = std::mktime(tminfo);
	//std::time_t y = std::mktime(&KeyAuthApp.user_data.expiry);
	//if (x != (std::time_t)(-1) && y != (std::time_t)(-1))
	//{
	//	double difference = std::difftime(y, x) / (60 * 60 * 24);
	//	std::cout << "\n " << difference << " day(s) left" << std::endl;
	//}
	//#pragma endregion Display Expiration Date and Days Left Until Expiry
	//
	//Sleep(-1); // this is to keep your application open for test purposes. it pauses your application forever, remove this when you want.
}
