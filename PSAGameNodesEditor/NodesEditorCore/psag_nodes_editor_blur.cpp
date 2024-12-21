// psag_nodes_editor_blur.
#include <iostream>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include "opengl_imgui_bloom.h"

using namespace std;

#ifdef ENABLE_WINDOWS_BLUR_API
// ���崰���������ö��
enum WINDOWCOMPOSITIONATTRIB {
	WCA_UNDEFINED     = 0,
	WCA_ACCENT_POLICY = 19,
	WCA_BLURBEHIND    = 20,
};

// ���� ACCENT_STATE ö��
enum ACCENT_STATE {
	ACCENT_DISABLED                   = 0,
	ACCENT_ENABLE_GRADIENT            = 1,
	ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
	ACCENT_ENABLE_BLURBEHIND          = 3, // ë����Ч��
	ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4, // �ǿ���Ч��
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

// �ֶ����� SetWindowCompositionAttribute ����.
using pSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

// �������ڱ���ģ��Ч��.
bool EnableWindowsBlur(HWND hwnd) {
	// ��̬���� "user32.dll" �е� SetWindowCompositionAttribute ����/
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
	// ���� ACCENT_POLICY Ϊë����Ч��.
	ACCENT_POLICY Accent = {};
	Accent.AccentState   = ACCENT_ENABLE_BLURBEHIND; // ë����Ч��.
	Accent.AccentFlags   = 1;                        // ����ģ���ĳ̶� (1,2 �ǳ���ֵ)
	Accent.GradientColor = 0xAA00FF00;               // ��͸����.

	// ACCENT_POLICY => WINDOW.
	WINDOWCOMPOSITIONATTRIBDATA data = {};
	data.Attrib = WCA_ACCENT_POLICY;
	data.pvData = &Accent;
	data.cbData = sizeof(Accent);

	SetWindowCompositionAttribute(hwnd, &data);
	FreeLibrary(hModule);
	return true;
}

// ����͸����ʽ.
void EnableWindowsTransparency(HWND hwnd) {
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
}
#endif