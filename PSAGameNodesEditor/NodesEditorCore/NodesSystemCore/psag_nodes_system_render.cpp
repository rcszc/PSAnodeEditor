// psag_nodes_system_render.
#include "psag_nodes_system.hpp"

using namespace std;
using namespace PSAG_LOGGER;

inline void EditorRectLED(const ImVec4& color, float rect_size) {
	ImGuiColorEditFlags ButtonFlags =
		ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip;
	ImGui::ColorButton("##LED", color, ButtonFlags, ImVec2(rect_size, rect_size));
}

inline void IMNODES_POP_STYLE_COL(size_t number) {
	for (size_t i = 0; i < number; ++i)
		ImNodes::PopColorStyle();
}

#define SMOOTH_YSTEP 72.0f
void ImGuiSmoothSlide(ImVec2* ypos, float speed = 1.0f, bool wheel = true, bool focus = true) {
	// flags: focus & hover | exten_focus.
	if (ImGui::IsWindowHovered() || !focus) {
		if (wheel) {
			if (ImGui::GetIO().MouseWheel < 0.0f) ypos->x += SMOOTH_YSTEP;
			if (ImGui::GetIO().MouseWheel > 0.0f) ypos->x -= SMOOTH_YSTEP;
		}
		// slide y limit.
		float YmaxposTemp = ImGui::GetScrollMaxY() + 4.0f;

		ypos->x = ypos->x < 0.0f ? 0.0f : ypos->x;
		ypos->x = ypos->x > YmaxposTemp ? YmaxposTemp : ypos->x;

		ypos->y += (ypos->x - ypos->y) * 0.075f * speed;
		ypos->y = ypos->y > YmaxposTemp ? YmaxposTemp : ypos->y;

		ImGui::SetScrollY(ypos->y);
	}
}

#define IM_ITEM_SPAC ImGui::GetStyle().ItemSpacing.x
namespace PSAnodesRender {

	void NodesEditorProject::RefashPathsData() {
		ProjectPaths.clear();
		auto LOAD_CALLBACK_FUNC = [&](const string& path, const string& name) {
			ProjectPaths.push_back(pair(path, name));
		};
		if (!PSAnodesFiletool::ForEachDirectoryFile(
			ProjectSaveFolder,
			PSAG_SYSTEM_FILEEXT,
			LOAD_CALLBACK_FUNC
		))
			PushLogger(LogWarning, LABLogNodesEditor, "project paths refash failed.");
	}

#define PSAG_NODES_EDITOR_BASE_FPS 120.0f
	float NodesEditorStatus::GetFramerateTimeStep() {
		return PSAG_NODES_EDITOR_BASE_FPS / ImGui::GetIO().Framerate;
	}

	ImVec4 NodesEditorStatus::RUN_STATUS_CYCLES() {
		RUN_CYCLES_COUNT += 0.0125f * GetFramerateTimeStep();
		ImVec4 ColorResult = {};
		// status colors switch. [0.0,1.0,2.0]
		RUN_CYCLES_COUNT > 1.0f ?
			ColorResult = ImVec4(0.0f, 0.2f, 0.18f, 1.0f) :
			ColorResult = ImVec4(0.0f, 1.0f, 0.92f, 1.0f);
		// clmap: cycles_count value. <= 2.0f
		RUN_CYCLES_COUNT = RUN_CYCLES_COUNT > 2.0f ? 0.0f : RUN_CYCLES_COUNT;
		return ColorResult;
	}

	ImVec4 NodesEditorStatus::STATUS_FLAG_COLOR(bool flag) {
		return ImVec4(0.0f, 1.0f * (float)flag + 0.2f, 0.92f * (float)flag + 0.18f, 1.0f);
	}

	inline void DrawPointIsInput(PSAnodesData::LinkPointInputSlot data) {
		ImNodes::BeginInputAttribute(data.PointUnique);
		// in.point draw type_title.
		ImGui::TextColored(
			data.PointSlotType.PointColor,
			data.PointText.c_str()
		);
		ImNodes::EndInputAttribute();
	}

	inline void DrawPointIsOutput(
		PSAnodesData::LinkPointOutputSlot data, float text_length, const char* in_text
	) {
		ImNodes::BeginOutputAttribute(data.PointUnique);
		// clac: node_width - out_point_text_w - in_point_text_w.
		ImGui::Indent(text_length - ImGui::CalcTextSize(data.PointText.c_str()).x - ImGui::CalcTextSize(in_text).x);
		// out.point draw type_title.
		ImGui::TextColored(
			data.PointSlotType.PointColor,
			data.PointText.c_str()
		);
		ImNodes::EndOutputAttribute();
	}

	void NodesEditorRender::DrawEditorComponentMouseMenu(const ImVec2& size) {
		// window state operation.
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			MouseMenuOpenFlag = !MouseMenuOpenFlag;
			// setting create menu position.  
			if (MouseMenuOpenFlag)
				ImGui::SetNextWindowPos(ImGui::GetMousePos());
		}
		// draw menu window, indep style.
		if (MouseMenuOpenFlag) {
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.2f);

			ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(0.18f, 0.0f, 0.38f, 0.8f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg,       ImVec4(0.04f, 0.0f, 0.12f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_TitleBg,       ImVec4(0.28f, 0.0f, 0.86f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.28f, 0.0f, 0.86f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0.0f, 1.0f, 0.92f, 0.78f));

			ImGui::SetNextWindowSize(size);
			ImGuiWindowFlags Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin(" NodesMenu", &MouseMenuOpenFlag, Flags);
			// window border = 0.0f, button border = 2.0f
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC * 0.5f);

			// create node entity.
			ImVec2 M_POS = ImGui::GetMousePos();
			ImVec2 E_POS = ImNodes::EditorContextGetPanning();
			// mouse =mapping=> editor grid position [x,y].
			ImVec2 Position = ImVec2(M_POS.x - E_POS.x, M_POS.y - E_POS.y);

			bool ButtonsHovered = false;

			ImVec2 OperButtonSize((size.x - IM_ITEM_SPAC * 3.0f) * 0.5f, 28.0f);
			if (ImGui::Button("DELETE", OperButtonSize)) {
				// 删除节点实体.
				if (ImNodesHoverUniqueNode != NULL) {
					if (!NodeEntityDEL(ImNodesHoverUniqueNode))
						PushLogger(LogWarning, LABLogNodesEditor, "delete node failed.");
				}
				// 删除连线实体.
				if (ImNnodeHoverUniqueLine != NULL)
					LinkLineEntities.erase(ImNnodeHoverUniqueLine);

				// delete oper => close menu.
				MouseMenuOpenFlag = false;
			}
			ButtonsHovered |= ImGui::IsItemHovered();
			ImGui::SameLine();
			if (ImGui::Button("COPY", OperButtonSize)) {

				auto EntityPtr = NodeEntityFIND(ImNodesHoverUniqueNode);
				if (EntityPtr != nullptr)
					NodeEntityGEN(NodeTemplateFIND(EntityPtr->NodeType.NodeUniqueID), Position);
				else
					PushLogger(LogWarning, LABLogNodesEditor, "copy node failed.");
				// copy oper => close menu.
				MouseMenuOpenFlag = false;
			}
			ButtonsHovered |= ImGui::IsItemHovered();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC);

			ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

			ImVec2 SelectWindowSize(size.x - IM_ITEM_SPAC * 2.0f, size.y - ImGui::GetCursorPosY() - IM_ITEM_SPAC);
			ImGui::BeginChild("##SELECT_NODES", SelectWindowSize, false, WindowFlags);
			{
				ImVec2 SelectButtonSize(ImGui::GetWindowWidth() - IM_ITEM_SPAC * 2.0f, 30.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC);
				ImGui::Indent(IM_ITEM_SPAC);
				// draw templates buttons list.
				TemplatesForEach([&](const PSAnodesData::NodeAttributeTemplate& template_item) {

					ImGui::PushStyleColor(ImGuiCol_Border, template_item.NodeDrawColor);
					ImGui::PushStyleColor(ImGuiCol_Text,   template_item.NodeDrawColor);

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC * 0.5f);
					if (ImGui::Button(template_item.NodeTitle.c_str(), SelectButtonSize)) {
						// check 'GEN'(create) status.
						if (!NodeEntityGEN(template_item, Position))
							PushLogger(LogWarning, LABLogNodesEditor, "create node failed.");
						// create oper => close menu.
						MouseMenuOpenFlag = false;
					}
					ButtonsHovered |= ImGui::IsItemHovered();
					ImGui::PopStyleColor(2);
				});
				ImGui::Unindent(IM_ITEM_SPAC);
				// 菜单平滑滚动条.
				ImGuiSmoothSlide(&StatusScroMouseMenu, GetFramerateTimeStep());
			}
			bool IsChildWindowHovered = ImGui::IsWindowHovered();
			ImGui::EndChild();
			// open window & mouse_left & not_hover_window & not_hover_button 
			// => close menu window.
			MouseMenuOpenFlag = MouseMenuOpenFlag == false ? false :
				!(ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
				!(ImGui::IsWindowHovered() || IsChildWindowHovered) &&
				!ButtonsHovered);
			// close context free, pop_status.
			ImGui::PopStyleVar(1);
			ImGui::End();
			ImGui::PopStyleColor(5);
			ImGui::PopStyleVar(1);
		}
	}

	// ImGui 静态文本(窗口,子窗口)居中.
	inline void StaticTextCenter(
		const char* text, const ImVec4& color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f)
	) {
		float Center = (ImGui::GetWindowWidth() - ImGui::CalcTextSize(text).x) * 0.5f;
		ImGui::SetCursorPosX(Center);
		ImGui::TextColored(color, text);
	}

	void NodesEditorRender::DrawEditorComponentToolbar() {
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC);
		// toolbar title text.
		StaticTextCenter("Node Projects Toolbar", ImVec4(0.0f, 1.0f, 0.92f, 1.0f));
		
		ImVec2 ProjectWinSize(ImGui::GetWindowSize().x - IM_ITEM_SPAC * 2.0f, ImGui::GetWindowSize().y * 0.5f);
		ImGui::Indent(IM_ITEM_SPAC);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.0f, 0.85f, 1.0f));

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC);
		ImGui::SetNextItemWidth(ProjectWinSize.x - 80.0f - IM_ITEM_SPAC);
		// input_text => copy => refash.
		if (ImGui::InputText("##FOLDER", ProjectSaveFolderTemp, PROJECT_SAVE_FOLDER_LENGTH))
			ProjectSaveFolder = string(ProjectSaveFolderTemp);

		ImGui::SameLine();
		if (ImGui::Button("REFASH", ImVec2(80.0f, 0.0f)))
			RefashPathsData();

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.22f, 0.0f, 0.48f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.0f, 0.88f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.22f, 0.0f, 0.48f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg,       ImVec4(0.04f, 0.0f, 0.12f, 0.7f));

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC);
		ImGui::BeginChild("##PROJECT", ProjectWinSize);
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC);
			ImGui::Indent(IM_ITEM_SPAC);

			float ButtonWidth = ProjectWinSize.x - IM_ITEM_SPAC * 2.0f;
			for (const auto& Item : ProjectPaths) {
				if (ImGui::Button(Item.second.c_str(), ImVec2(ButtonWidth, 42.0f))) {
					// c-single 选择当前项目, c-double 取消当前项目.
					if (ProjectCurrentPath.second != Item.second) {
						ProjectCurrentPath = Item;
						// update title_msg.
						SetTitlebarText("current project name: '" + Item.second + "'");
					}
					else {
						ProjectCurrentPath = pair(string(), string());
						SetTitlebarText("current no-project name.");
					}
				}
			}
			ImGui::Unindent(IM_ITEM_SPAC);
			// 菜单平滑滚动条.
			ImGuiSmoothSlide(&StatusScroToolbar, GetFramerateTimeStep());
		}
		ImGui::EndChild();

		ImGui::SetNextItemWidth(ProjectWinSize.x);
		if (ImGui::InputText("##SAVENAME", ProjectSaveNameTemp, PROJECT_SAVE_FOLDER_LENGTH))
			ProjectSaveName = string(ProjectSaveNameTemp);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC * 2.0f);
		StaticTextCenter("Node Project", ImVec4(0.0f, 1.0f, 0.92f, 1.0f));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.0f, 0.12f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_Text,   ImVec4(0.02f, 1.0f, 0.92f, 0.7f));

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC * 2.0f);

		bool ClearFlag = ImGui::Button("Clear Mesh Nodes", ImVec2(ProjectWinSize.x, 42.0f));
		if (!EditorStatusFlagMutexButton && ClearFlag)
			NodesPointsEntityDelete();

		// 节点项目保存. save data & u_name.
		if (ImGui::Button("Save Project", ImVec2(ProjectWinSize.x, 42.0f))) {
			if (!ProjectSaveName.empty()) {

				PSAnodesData::ExportNodesSource DataTemp = {};
				DataTemp.NodesData = NodeEntities;
				DataTemp.LinesData = LinkLineEntities;
				
				string SaveFilepath = PSAnodesFiletool::FilepathAssemble(
					ProjectSaveFolder, 
					ProjectSaveName, 
					PSAG_SYSTEM_FILEEXT
				);
				PSAnodesSystem::SystemJsonConfigFileLoader FileLoader = {};
				// write project file.
				if (!FileLoader.SourceStringFileWrite(
					SaveFilepath,
					PSAnodesConvert::ExportProjectData(DataTemp, ProjectUniqueNAME)
				))
					PushLogger(LogWarning, LABLogNodesEditor, "project save failed.");
				// save file => refash.
				RefashPathsData();
			}
		}

		// 节点项目加载. load data & check u_name.
		if (ImGui::Button("Read Project", ImVec2(ProjectWinSize.x, 42.0f))) {
			// read file => decode => to_entites.
			PSAnodesSystem::SystemJsonConfigFileLoader FileLoader = {};

			bool StatusFlag = false;
			string Content = FileLoader.SourceStringFileRead(ProjectCurrentPath.first, &StatusFlag);
			if (StatusFlag) {
				vector<PSAnodesData::SaveProjectNodeData>     NodesTemp = {};
				vector<PSAnodesData::SaveProjectLineLinkData> LinesTemp = {};
				
				if (!PSAnodesConvert::ProjectDataLoader(
					Content, ProjectUniqueNAME, &NodesTemp, &LinesTemp,
					&NodesGenUid, &PointsGenUid
				))
					PushLogger(LogError, LABLogNodesEditor, "project data decode failed.");

				SaveDatasetToEntites(NodesTemp, LinesTemp);
				PushLogger(
					LogInfo, LABLogNodesEditor, "reset uid alloter counter: n: %d, p: %d",
					NodesGenUid.Counter, PointsGenUid.Counter
				);
			}
		}

		// 节点项目删除.
		if (ImGui::Button("Delete Project", ImVec2(ProjectWinSize.x, 42.0f)) && 
			!EditorStatusFlagMutexButton
		) {
			// first: file path => delete.
			if (!PSAnodesFiletool::DeleteFile(ProjectCurrentPath.first))
				PushLogger(LogWarning, LABLogNodesEditor, "project delete failed.");
			// delete file => refash.
			RefashPathsData();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + IM_ITEM_SPAC * 2.0f);
		ImGui::Checkbox(" mutex clear button", &EditorStatusFlagMutexButton);

		ImGui::Unindent(IM_ITEM_SPAC);
		ImGui::PopStyleColor(7);
	}

	void NodesEditorRender::DrawCoreNodesLines() {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
		// nodes entites render.
		for (auto& EntityItem : NodeEntities) {
			uint32_t NodeColorStyle = ImGui::ColorConvertFloat4ToU32(ImVec4(EntityItem.NodeDrawColor));

			ImNodes::PushColorStyle(ImNodesCol_TitleBar,         NodeColorStyle);
			ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered,  NodeColorStyle);
			ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, NodeColorStyle);

			if (ImNodes::IsNodeSelected(EntityItem.NodeUnique))
				NodeColorStyle = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
			// node entity border color.
			ImNodes::PushColorStyle(ImNodesCol_NodeOutline, NodeColorStyle);

			ImNodes::BeginNode(EntityItem.NodeUnique);
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(EntityItem.NodeTitle.c_str());
			ImNodes::EndNodeTitleBar();

			// get update node draw position.
			EntityItem.NodePosition = ImNodes::GetNodeGridSpacePos(EntityItem.NodeUnique);

			size_t Difference = abs((int64_t)EntityItem.InputPoints.size() - (int64_t)EntityItem.OutputPoints.size());
			size_t Count = min(EntityItem.InputPoints.size(), EntityItem.OutputPoints.size());

			// draw editor input slot.points
			for (size_t i = 0; i < Count; ++i) {
				DrawPointIsInput(EntityItem.InputPoints[i]);
				// input & output same_line.
				ImGui::SameLine();

				DrawPointIsOutput(
					EntityItem.OutputPoints[i],
					EntityItem.NodeWidth,
					EntityItem.InputPoints[i].PointText.c_str()
				);
			}
			// draw editor output slot.points
			if (Count == EntityItem.InputPoints.size()) {
				// output > input, residue out.
				for (size_t i = Count; i < Count + Difference; ++i)
					DrawPointIsOutput(
						EntityItem.OutputPoints[i],
						EntityItem.NodeWidth + 8.0f, ""
					);
			}
			else // output < input, residue input.
				for (size_t i = Count; i < Count + Difference; ++i)
					DrawPointIsInput(EntityItem.InputPoints[i]);
			ImNodes::EndNode();

			IMNODES_POP_STYLE_COL(4);
		}

		ImU32 SelectColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.32f, 0.32f, 0.32f, 0.58f));
		ImNodes::PushColorStyle(ImNodesCol_LinkSelected, SelectColor);
		// link_lines entites render.
		for (const auto& Line : LinkLineEntities) {
			uint32_t LineColorStyle = ImGui::ColorConvertFloat4ToU32(Line.second.LineDrawColor);

			ImNodes::PushColorStyle(ImNodesCol_Link,        LineColorStyle);
			ImNodes::PushColorStyle(ImNodesCol_LinkHovered, LineColorStyle);

			ImNodes::Link(Line.first, Line.second.LinePointBegin, Line.second.LinePointEnd);

			IMNODES_POP_STYLE_COL(2);
		}
		ImNodes::PopColorStyle();
		ImGui::PopStyleColor(1);
	}

	NodesEditorRender::NodesEditorRender(const string& f_aribs, const string& f_types) {
		// create imnodes rebder context.
		ImNodes::CreateContext();
		ImNodes::StyleColorsDark();
		PushLogger(LogInfo, LABLogNodesEditor, "system init create imnodes context.");

		PSAnodesSystem::SystemJsonConfigFileLoader JsonConifg = {};
		bool FileOperateFlag = false;

		// config filepath => load => json decode => config dataset.
		SystemInitPointTypesLoad(JsonConifg.SourceStringDecode(
			JsonConifg.SourceStringFileRead(f_types, &FileOperateFlag), &FileOperateFlag
		));
		SystemInitNodeTypesLoad(JsonConifg.SourceStringDecode(
			JsonConifg.SourceStringFileRead(f_aribs, &FileOperateFlag), &FileOperateFlag
		));
		// check files load status flag.
		if (!FileOperateFlag) {
			PushLogger(
				LogError, LABLogNodesEditor, "system init failed load files: %s, %s",
				f_aribs.c_str(), f_types.c_str()
			);
			return;
		}
		// imgui, imnodes style color, value setting.
		ImGui::PushStyleColor(ImGuiCol_Text,     ImVec4(0.02f, 1.0f, 0.92f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.0f, 0.16f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg,  ImVec4(0.16f, 0.0f, 0.42f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg,  ImVec4(0.04f, 0.0f, 0.12f, 0.7f));

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.08f, 0.0f, 0.22f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.0f, 0.85f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.25f, 0.0f, 0.85f, 0.2f));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    5.8f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,    5.8f);

		ImNodesStyle& ImNodesStyle = ImNodes::GetStyle();

		ImNodesStyle.Colors[ImNodesCol_GridBackground]  = IM_COL32(10, 00, 24, 255);
		ImNodesStyle.Colors[ImNodesCol_GridLine]        = IM_COL32(36, 16, 58, 255);
		ImNodesStyle.Colors[ImNodesCol_GridLinePrimary] = IM_COL32(36, 16, 58, 255);

		ImNodesStyle.Colors[ImNodesCol_Pin]        = IM_COL32(0, 255, 255, 255);
		ImNodesStyle.Colors[ImNodesCol_PinHovered] = IM_COL32(0, 255, 255, 255);

		ImNodesStyle.Colors[ImNodesCol_MiniMapBackground]        = IM_COL32(42, 0, 92, 178);
		ImNodesStyle.Colors[ImNodesCol_MiniMapBackgroundHovered] = IM_COL32(42, 0, 92, 178);
		ImNodesStyle.Colors[ImNodesCol_MiniMapOutline]           = IM_COL32(42, 0, 92, 178);
		ImNodesStyle.Colors[ImNodesCol_MiniMapOutlineHovered]    = IM_COL32(42, 0, 92, 178);
		ImNodesStyle.Colors[ImNodesCol_MiniMapCanvas]            = IM_COL32(22, 0, 36, 128);
		ImNodesStyle.Colors[ImNodesCol_MiniMapCanvasOutline]     = IM_COL32(0, 255, 255, 255);
		ImNodesStyle.Colors[ImNodesCol_MiniMapLink]              = IM_COL32(0, 255, 255, 225);
		ImNodesStyle.Colors[ImNodesCol_MiniMapLinkSelected]      = IM_COL32(0, 255, 255, 225);

		ImNodesStyle.Colors[ImNodesCol_NodeBackground]         = IM_COL32(16, 0, 42, 192);
		ImNodesStyle.Colors[ImNodesCol_NodeBackgroundHovered]  = IM_COL32(16, 0, 42, 192);
		ImNodesStyle.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(16, 0, 42, 192);

		ImNodesStyle.NodeBorderThickness = 4.85f;
		ImNodesStyle.NodeCornerRounding  = 10.0f;
		
		PushLogger(
			LogTrace, LABLogNodesEditor, "nodes editor version code: %x", 
			PSAG_NODES_DATA_FORMAT_VERSION
		);
		PushLogger(LogInfo, LABLogNodesEditor, "nodes editor system init success.");
		// set start title_msg.
		SetTitlebarText("welcome @pomelo_star nodes art.editor. [alpha]");
	}

	NodesEditorRender::~NodesEditorRender() {
		// pop restore style context.
		ImGui::PopStyleColor(7);
		ImGui::PopStyleVar(6);
		// free imnodes render context.
		ImNodes::DestroyContext();
	}

	void NodesEditorRender::DrawEditorWindowFrame(const ImVec2& size, bool fixed, ImGuiWindowFlags flags) {
		ImGui::SetNextWindowSize(size);
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | flags;
		// fixed window => system window(0,0) => setting flags.
		if (fixed) {
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			WindowFlags |= ImGuiWindowFlags_NoMove;
		}
		// nodes editor main window context.
		ImGui::Begin("##NODES_EDITOR", (bool*)NULL, WindowFlags);
		{
			// setting editor style: values.
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.8f);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   9.8f);
			// setting editor style: colors.
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.0f, 0.52f, 1.0f));

			ImVec2 NodesWindowSize(
				(1.0f - WindowPropToolbar ) * size.x - IM_ITEM_SPAC * 3.0f,
				(1.0f - WindowPropTitlebar) * size.y - IM_ITEM_SPAC * 3.0f
			);
			float LevLineYaxis = size.y * WindowPropTitlebar + IM_ITEM_SPAC * 2.0f;

			ImGui::SetCursorPos(ImVec2(IM_ITEM_SPAC, LevLineYaxis));
			ImGui::BeginChild("##W_NODES", NodesWindowSize);
			// menu open => status not_refresh.
			if (!MouseMenuOpenFlag) {
				ImNodesHoverUniqueNode = NULL;
				ImNnodeHoverUniqueLine = NULL;
			}
			ImNodes::IsNodeHovered(&ImNodesHoverUniqueNode);
			ImNodes::IsLinkHovered(&ImNnodeHoverUniqueLine);

			ImNodes::BeginNodeEditor();
			{
				DrawCoreNodesLines();
				// nodes editor mini_map 28% size.
				ImNodes::MiniMap(0.25f, ImNodesMiniMapLocation_TopRight);
			}
			ImNodes::EndNodeEditor();
			ImGui::EndChild();

			// 节点实体连接监听不能再绘制上下文中. 
			// RCSZ 2024_12_12.
			PointsEntityLinkMonitor();
			
			ImGui::PopStyleColor(1);
			ImGui::PopStyleVar(2);

			ImVec2 TitlebarWindowSize(size.x - IM_ITEM_SPAC * 2.0f, WindowPropTitlebar * size.y);

			ImGui::SetCursorPos(ImVec2(IM_ITEM_SPAC, IM_ITEM_SPAC));
			ImGui::BeginChild("##W_TITLEBAR", TitlebarWindowSize);
			{
				float TextHeight = ImGui::CalcTextSize("I").y;
				// draw begin offset x,y.
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + IM_ITEM_SPAC);
				ImGui::SetCursorPosY(TitlebarWindowSize.y * 0.5f - TextHeight * 0.425f);
				// titlebar editor LED[3](total), title text.
				EditorRectLED(RUN_STATUS_CYCLES(), TextHeight * 0.85f);
				// warning status LED[2].
				ImGui::SameLine(); EditorRectLED(STATUS_FLAG_COLOR(EditorStatusFlagMutexButton), TextHeight * 0.85f);
				ImGui::SameLine(); EditorRectLED(STATUS_FLAG_COLOR(EditorStatusFlagOperation),   TextHeight * 0.85f);

				ImGui::SetCursorPosY(TitlebarWindowSize.y * 0.5f - TextHeight * 0.5f);
				// titlebar: editor params, log message print.
				ImGui::SameLine();
				ImGui::Text(
					" Nodes Templates: %u Entities: %u Lines: %u Message: ",
					NodeTemplates.size(), NodeEntities.size(), LinkLineEntities.size()
				);
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.45f, 0.0f, 0.85f, 1.0f), TitlebarLogMsg.c_str());
			}
			ImGui::EndChild();

			ImVec2 ToolbarWindowSize(size.x * WindowPropToolbar, NodesWindowSize.y);
			ImVec2 ToolbarWindowPos(NodesWindowSize.x + IM_ITEM_SPAC * 2.0f, LevLineYaxis);

			ImGui::SetCursorPos(ToolbarWindowPos);
			ImGui::BeginChild("##W_TOOLBAR", ToolbarWindowSize);
			{
				DrawEditorComponentToolbar();
			}
			ImGui::EndChild();
		}
		ImGui::End();
		// draw menu,tool window components. 
		DrawEditorComponentMouseMenu(ImVec2(240.0f, 360.0f));
		EditorStatusFlagOperation = MouseMenuOpenFlag;
	}
}