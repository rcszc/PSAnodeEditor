// psag_nodes_editor_init.
#include <fstream>
#include "psag_nodes_editor.h"

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include "opengl_imgui_bloom.h"

using namespace std;
using namespace PSAG_LOGGER;

static void GLFW_ERROR_CALLBACK(int error, const char* description) {
	// glfw error_output callback(fprintf).
	PushLogger(LogError, "GLFW_CORE", "error_code : % d Msg : % s", error, description);
}

inline COLORREF SystemConvertColorRGB(const ImVec4& color) {
	return RGB(int(255.0f * color.x), int(255.0f * color.y), int(255.0f * color.z));
}
// windows dwm api, win7? win10, win11.
inline void SetSystemWindowTitleBarColor(GLFWwindow* window, const ImVec4& border) {
	HWND Hwnd = glfwGetWin32Window(window);
	BOOL DarkMode = TRUE;

	COLORREF Titlebar = SystemConvertColorRGB(border);

	DwmSetWindowAttribute(Hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &DarkMode, sizeof(DarkMode));
	DwmSetWindowAttribute(Hwnd, DWMWA_BORDER_COLOR,  &Titlebar, sizeof(Titlebar));
	DwmSetWindowAttribute(Hwnd, DWMWA_CAPTION_COLOR, &Titlebar, sizeof(Titlebar));

	COLORREF TitleConstColor = RGB(0, 255, 255);
	DwmSetWindowAttribute(Hwnd, DWMWA_TEXT_COLOR, &TitleConstColor, sizeof(TitleConstColor));

	// Windows 未公开API.
#ifdef ENABLE_WINDOWS_BLUR_API
	EnableWindowsBlur(Hwnd);
	EnableWindowsTransparency(Hwnd);
#endif
}
// windows api.
inline bool IsOnHeap(void* ptr) {
	HANDLE HeapHandle = GetProcessHeap();
	return HeapValidate(HeapHandle, 0, ptr);
}

inline void InitSystemWindowClear(GLFWwindow* window) {
	// init clear window.
	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
}

namespace SystemResource {
	rapidjson::Document SystemRenderStarter::JsonLoader(const string& filename) {
		ifstream ReadJsonFile(filename);
		// file status.
		if (!ReadJsonFile.is_open()) {
			PushLogger(LogError, LABLogSystemResource, "load config failed open file: ", filename.c_str());
			return rapidjson::Document();
		}
		// read string file.
		string ContentString((istreambuf_iterator<char>(ReadJsonFile)), istreambuf_iterator<char>());
		ReadJsonFile.close();
		PushLogger(LogInfo, LABLogSystemResource, "load config read file: ", filename.c_str());

		// read processing json_document.
		rapidjson::Document JsonDocumentObject = {};
		JsonDocumentObject.Parse(ContentString.c_str());

		// check json object valid ?
		if (JsonDocumentObject.HasParseError() || !JsonDocumentObject.IsObject())
			PushLogger(LogError, LABLogSystemResource, "json_document decode failed.");
		return JsonDocumentObject;
	}

	bool SystemRenderStarter::CreateGraphicsContext() {
		filesystem::path Filepath = SystemConfigFolder / PSAG_EDITOR_PATHNAME;
		auto JsonDoc = JsonLoader(Filepath.string());

		bool CheckKeysFlag = false;

		CheckKeysFlag |= !JsonDoc["WindowSize"].IsArray();
		CheckKeysFlag |= !JsonDoc["WindowBorder"].IsBool();
		CheckKeysFlag |= !JsonDoc["WindowTitle"].IsString();
		CheckKeysFlag |= !JsonDoc["WindowMSAA"].IsUint();
		CheckKeysFlag |= !JsonDoc["WindowAsync"].IsBool();
		CheckKeysFlag |= !JsonDoc["ImGuiFont"].IsString();
		CheckKeysFlag |= !JsonDoc["ImGuiFontSize"].IsFloat();

		if (CheckKeysFlag) {
			PushLogger(LogError, LABLogSystemResource, "system config keys err.");
			return false;
		}
		// glfw init create.
		glfwSetErrorCallback(GLFW_ERROR_CALLBACK);
		if (!glfwInit()) {
			PushLogger(LogError, LABLogSystemResource, "core opengl_glfw init failed.");
			return false;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // glfw.version 3.2+ [profile].
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);         // glfw.version 3.0+

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // fixed window size flag.
		glfwWindowHint(GLFW_SAMPLES, (int)JsonDoc["WindowMSAA"].GetUint());
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

		glfwWindowHint(GLFW_DECORATED, (int)JsonDoc["WindowBorder"].GetBool());

		// create window.
		GLFWmonitor* CreateMonitor = {};

		WindowSize.x = (float)JsonDoc["WindowSize"][0].GetInt();
		WindowSize.y = (float)JsonDoc["WindowSize"][1].GetInt();

		WindowSize.x = WindowSize.x < 256.0f ? 256.0f : WindowSize.x;
		WindowSize.y = WindowSize.y < 256.0f ? 256.0f : WindowSize.y;

		WindowObject = glfwCreateWindow(
			(int)WindowSize.x, (int)WindowSize.y, JsonDoc["WindowTitle"].GetString(),
			CreateMonitor, nullptr
		);
		// create context.
		glfwMakeContextCurrent(WindowObject);
		// enable async (窗口渲染垂直同步).
		glfwSwapInterval((int)JsonDoc["WindowAsync"].GetBool());

		// windows: dwmapi call. system_window titlebar.
		SetSystemWindowTitleBarColor(WindowObject, ImVec4(0.15f, 0.0f, 0.35f, 1.0f));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glewInit() != GL_NO_ERROR) {
			PushLogger(LogError, LABLogSystemResource, "core opengl_glew init failed.");
			return false;
		}
		InitSystemWindowClear(WindowObject);
		PushLogger(LogTrace, LABLogSystemResource, "system graphics context init.");

		WindowFontFile = JsonDoc["ImGuiFont"].GetString();
		WindowFontSize = JsonDoc["ImGuiFontSize"].GetFloat();
		return true;
	}

	bool SystemRenderStarter::CreateImGuiContext(const char* version) {
		if (!filesystem::exists(WindowFontFile) || WindowFontSize < 1.0f) {
			PushLogger(LogError, LABLogSystemResource, "system config font params err.");
			return false;
		}
		IMGUI_CHECKVERSION();

		// setup imgui context.
		ImGui::CreateContext();
		ImGuiIO& GuiIO = ImGui::GetIO(); (void)GuiIO;

		// // enable keyboard & gamepad controls.
		GuiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		GuiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		// init setting font(s).
		auto ConfigFonts = ImGui::GetIO().Fonts;
		ConfigFonts->AddFontFromFileTTF
		(
			WindowFontFile.c_str(), WindowFontSize, NULL,
			ConfigFonts->GetGlyphRangesChineseFull()
		);
		ImGui::GetIO().FontGlobalScale = 0.85f;

		// setup platform & renderer backends.
		ImGui_ImplGlfw_InitForOpenGL(WindowObject, true);
		ImGui_ImplOpenGL3_Init(version);
		PushLogger(LogTrace, LABLogSystemResource, "system imgui context init.");
		return true;
	}

	SystemRenderStarter::SystemRenderStarter(const string& folder) : SystemConfigFolder(folder) {
		if (!filesystem::is_directory(SystemConfigFolder)) {
			PushLogger(LogError, LABLogSystemResource, "system folder invalid.");
			return;
		}
		bool ContextErrFlag = false;

		ContextErrFlag |= !CreateGraphicsContext();
		ContextErrFlag |= !CreateImGuiContext(PSAG_SHADER_VERSION);

		if (ContextErrFlag) {
			PushLogger(LogError, LABLogSystemResource, "system context init failed.");
			return;
		}
		filesystem::path FilepathAttrib = SystemConfigFolder / PSAG_NODES_PATHNAME_ATTRIB;
		filesystem::path FilepathTypes  = SystemConfigFolder / PSAG_NODES_PATHNAME_TYPES;

		Filepaths[0] = FilepathAttrib.string();
		Filepaths[1] = FilepathTypes.string();
	}

	SystemRenderStarter::~SystemRenderStarter() {
		if (RUN_OBJECT == nullptr) {
			PushLogger(LogError, LABLogSystemResource, "system free object failed.");
			return;
		}
		delete RUN_OBJECT;

		// free(graph) imgui context.
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// free(graph) window context.
		glfwDestroyWindow(WindowObject);
		glfwTerminate();

		PushLogger(LogTrace, LABLogSystemResource, "system imgui context free.");
		PushLogger(LogTrace, LABLogSystemResource, "system graphics context free.");
	}

	bool SystemRenderStarter::SettingRunEventEntity(EditorRunInterface* object) {
		// 空指针检查, 堆内存分配检测(win).
		bool CheckFlags[2] = { object == nullptr, (!IsOnHeap(object)) };
		if (CheckFlags[0] || CheckFlags[1]) {
			PushLogger(
				LogError, LABLogSystemResource, 
				"system set object nullptr | not heap_memory, flags: %d%d",
				(int)CheckFlags[0], (int)CheckFlags[1]
			);
			return false;
		}
		RUN_OBJECT = object;
		PushLogger(LogInfo, LABLogSystemResource, "system set object: %x", RUN_OBJECT);
		return true;
	}
	/*
	* imgui bloom 接管渲染上下文, 替换:
	* 
	* ImGui_ImplOpenGL3_NewFrame();
	* ImGui_ImplGlfw_NewFrame();
	* ImGui::NewFrame();
	* ...
	* ImGui::Render();
	* ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	*/
	void SystemRenderStarter::RunRenderSystem() {
		RUN_OBJECT->EditorCoreEventInit(Filepaths[0], Filepaths[1]);

		BloomImGui::FxBloomSystem FrameBloomFX(
			(GLuint)WindowSize.x, (GLuint)WindowSize.y
		);
		// opengl glfw api, newframe impl_function.
		FrameBloomFX.GetImGuiNewFrameFUNC(ImGui_ImplGlfw_NewFrame);

		FrameBloomFX.SettingBloomRadius(12);
		FrameBloomFX.SettingFilterParams()->ColorRGBchannelsAvgFilter = 0.24f;

		auto SetParams = FrameBloomFX.SettingBlendParams();

		SetParams->BlurFragmentBlend   = 1.52f;
		SetParams->SourceFragmentBlend = 0.94f;

		PushLogger(LogInfo, LABLogSystemResource, "system start render loop.");
		int BufferSize[2] = {};
		// opengl & imgui render_loop.
		while (!(bool)glfwWindowShouldClose(WindowObject)) {
			// rendering event_loop.
			glfwPollEvents();
			glfwGetFramebufferSize(WindowObject, &BufferSize[0], &BufferSize[1]);

			glViewport(0, 0, BufferSize[0], BufferSize[1]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render capture context.
			FrameBloomFX.RenderContextCaptureBegin();
			RUN_OBJECT->EditorCoreEventFrame();
			FrameBloomFX.RenderContextCaptureEnd();

			glfwMakeContextCurrent(WindowObject);
			glfwSwapBuffers(WindowObject);
		}
		RUN_OBJECT->EditorCoreEventDestroy();
		PushLogger(LogInfo, LABLogSystemResource, "system exit render loop.");
	}
}