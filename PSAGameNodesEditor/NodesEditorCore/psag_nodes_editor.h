// psag_nodes_editor. include: windows 'DWM' api.
// warning: windows platform.

#ifndef __PSAG_NODES_EDITOR_H
#define __PSAG_NODES_EDITOR_H
#include <filesystem>
#include "NodesSystemLogger/framework_logger.hpp"

#include <GL/glew.h>
#include <GL/GL.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(_MSV_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib,"legacy_stdio_definitons.lib")
#endif

#include <document.h>

#define PSAG_EDITOR_PATHNAME "system_editor_config.json"

#define PSAG_NODES_PATHNAME_ATTRIB "nodes_config_attributes.json"
#define PSAG_NODES_PATHNAME_TYPES  "nodes_config_types.json"

class EditorRunInterface {
public:
	virtual bool EditorCoreEventInit(const std::string& attrib_cfg, const std::string& types_cfg) = 0;
	virtual void EditorCoreEventFrame() = 0;
	virtual void EditorCoreEventDestroy() = 0;
};

namespace SystemResource {
	StaticStrLABEL LABLogSystemResource = "SYSTEM_RESOURCE";

#define PSAG_SHADER_VERSION "#version 330 core"
	class SystemRenderStarter {
	private:
		EditorRunInterface* RUN_OBJECT = nullptr;
	protected:
		std::filesystem::path SystemConfigFolder = {};

		GLFWwindow* WindowObject   = nullptr;
		ImVec2      WindowSize     = {};
		std::string WindowFontFile = {};
		float       WindowFontSize = 1.0f;

		rapidjson::Document JsonLoader(const std::string& filename);

		bool CreateGraphicsContext();
		bool CreateImGuiContext(const char* version);

		std::string Filepaths[2] = {};
	public:
		SystemRenderStarter(const std::string& folder);
		~SystemRenderStarter();

		// warning: alloc memory => heap.
		bool SettingRunEventEntity(EditorRunInterface* object);
		void RunRenderSystem();
	};
}

#include "NodesSystemCore/psag_nodes_system.hpp"

class EditorRunDrawImpl :public EditorRunInterface {
protected:
	PSAnodesRender::NodesEditorRender* NodesEditor = nullptr;
public:
	bool EditorCoreEventInit(const std::string& attrib_cfg, const std::string& types_cfg);
	void EditorCoreEventFrame();
	void EditorCoreEventDestroy();
};

#ifdef ENABLE_WINDOWS_BLUR_API
bool EnableWindowsBlur(HWND hwnd);
void EnableWindowsTransparency(HWND hwnd);
#endif

#endif