// psag_nodes_editor.
#include "psag_nodes_editor.h"

using namespace std;

bool EditorRunDrawImpl::EditorCoreEventInit(const string& attrib_cfg, const string& types_cfg) {

	NodesEditor = new PSAnodesRender::NodesEditorRender(attrib_cfg, types_cfg);

	NodesEditor->NEP_SettingDefaultFolder("NodesEditorResource/SystemProject/");

	return true;
}

void EditorRunDrawImpl::EditorCoreEventFrame() {

	NodesEditor->DrawEditorWindowFrame(ImGui::GetIO().DisplaySize, true);

	if (ImGui::IsKeyDown(ImGuiKey_E))
		cout << NodesEditor->ENC_ExportCurrentData(PSAnodesEncode::PSAN_ENCODE_V101A) << endl;
}

void EditorRunDrawImpl::EditorCoreEventDestroy() {
	delete NodesEditor;
}