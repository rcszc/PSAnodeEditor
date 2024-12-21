// psag_nodes_editor_blur.
#include <iostream>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include "opengl_imgui_bloom.h"

using namespace std;

#ifdef ENABLE_WINDOWS_BLUR_API
// 定义窗口组合属性枚举
enum WINDOWCOMPOSITIONATTRIB {
	WCA_UNDEFINED     = 0,
	WCA_ACCENT_POLICY = 19,
	WCA_BLURBEHIND    = 20,
};

// 定义 ACCENT_STATE 枚举
enum ACCENT_STATE {
	ACCENT_DISABLED                   = 0,
	ACCENT_ENABLE_GRADIENT            = 1,
	ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
	ACCENT_ENABLE_BLURBEHIND          = 3, // 毛玻璃效果
	ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4, // 亚克力效果
	ACCENT_ENABLE_HOSTBACKDROP        = 5,
};

struct ACCENT_POLICY {
	ACCENT_STATE AccentState;
	int AccentFlags;
	int GradientColor;
	int AnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
	WINDOWCOMPOSITIONATTRIB Attrib;
	PVOID pvData;
	SIZE_T cbData;
};

// 手动加载 SetWindowCompositionAttribute 函数.
using pSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

// 开启窗口背景模糊效果.
bool EnableWindowsBlur(HWND hwnd) {
	// 动态加载 "user32.dll" 中的 SetWindowCompositionAttribute 函数/
	auto hModule = LoadLibrary(TEXT("user32.dll"));
	if (!hModule) {
		cerr << "failed load 'user32.dll' library." << endl;
		return false;
	}
	auto SetWindowCompositionAttribute = reinterpret_cast<pSetWindowCompositionAttribute>(
		GetProcAddress(hModule, "SetWindowCompositionAttribute")
	);
	if (!SetWindowCompositionAttribute) {
		cerr << "failed get 'SetWindowCompositionAttribute' function." << endl;
		FreeLibrary(hModule);
		return false;
	}
	// 配置 ACCENT_POLICY 为毛玻璃效果.
	ACCENT_POLICY Accent = {};
	Accent.AccentState   = ACCENT_ENABLE_BLURBEHIND; // 毛玻璃效果.
	Accent.AccentFlags   = 1;                        // 控制模糊的程度 (1,2 是常用值)
	Accent.GradientColor = 0xAA00FF00;               // 半透明黑.

	// ACCENT_POLICY => WINDOW.
	WINDOWCOMPOSITIONATTRIBDATA data = {};
	data.Attrib = WCA_ACCENT_POLICY;
	data.pvData = &Accent;
	data.cbData = sizeof(Accent);

	SetWindowCompositionAttribute(hwnd, &data);
	FreeLibrary(hModule);
	return true;
}

// 开启透明样式.
void EnableWindowsTransparency(HWND hwnd) {
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
}
#endif