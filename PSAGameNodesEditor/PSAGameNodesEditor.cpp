/*
* PSAGameNodesEditor.RCSZ 2024_12_07.
* @pomelo_star studio game(framework), nodes editor.
* Ext.Library: OpenGL3, GLFW, GLEW, ImGui, STB_IMAGE, ImNodes, RapidJSON.
* Editor: For Windows10-11, version: 0.1.2 alpha 20241207.
* Dev: VisualStudio2022, MSVC Debug x64 C11/C++17.
* 外部依赖自写模块:
* Editor Bloom FX : https://github.com/rcszc/OGL-ImGui-BloomEXT.git
* Editor Logger : https://github.com/rcszc/PSAGame2D/tree/master/PSAGameFramework/PSAGameFrameworkCore/LLLogger
*/

#include <iostream>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib"   )
#pragma comment(lib, "glew32.lib"  )

#include "NodesEditorCore/psag_nodes_editor.h"

using namespace std;

int main() {
	PSAG_LOGGER_PROCESS::StartLogProcessing("NodesEditorResource/SystemLogs/");
	PSAG_LOGGER::PushLogger(LogTrace, "MAIN", "logger system start.");
	{
		SystemResource::SystemRenderStarter EditorFramework("NodesEditorResource/SystemConfig/");
		EditorRunDrawImpl* EditorRunDraw = new EditorRunDrawImpl();

		EditorFramework.SettingRunEventEntity(EditorRunDraw);
		EditorFramework.RunRenderSystem();
	}
	PSAG_LOGGER_PROCESS::FreeLogProcessing();
	return NULL;
}