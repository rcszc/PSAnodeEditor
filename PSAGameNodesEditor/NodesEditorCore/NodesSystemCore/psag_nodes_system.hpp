// psag_nodes_system. core define. RCSZ 2024_12_08.
// update: 2024_12_08 [0042]

#ifndef __PSAG_NODES_SYSTEM_HPP
#define __PSAG_NODES_SYSTEM_HPP
#include <functional>
#include <document.h>
#include "psag_nodes_system_data.hpp"

#include "imnodes.h"
#include "imnodes_internal.h"

// framework logger system.
#include "../NodesSystemLogger/framework_logger.hpp"

namespace PSAnodesSystem {
	StaticStrLABEL LABLogNodesSystem = "NODES_SYSTEM";

	// nodes core template & entity dataset. 
	class NodesDataComponentManager {
	protected:
		std::vector<PSAnodesData::NodeAttributeTemplate> NodeTemplates = {};
		std::vector<PSAnodesData::NodeAttributeEntity>   NodeEntities  = {};

		std::unordered_map<std::string, PointTypeUid> PointTypes = {};
		std::unordered_map<LinkUid, PSAnodesData::LineLink> LinkLineEntities = {};
		// node template vector index mapper, select temp.
		std::unordered_map<NodeType, size_t> NodeTempMapper = {};

		std::string ProjectUniqueNAME = {};
	};

	using JsonConfig = rapidjson::Document;
	class SystemJsonConfigFileLoader {
	public:
		std::string SourceStringFileRead(const std::string& filepath, bool* status);
		JsonConfig SourceStringDecode(const std::string& config, bool* status);

		bool SourceStringFileWrite(const std::string& filepath, const std::string& str);
	};

	using JsonConfigValue = const rapidjson::Value&;
	class SystemJsonConfigDEC {
	protected:
		// color channels rgba(array:4), rgba_integer. std: rgba8, hdr10.
		ImVec4 SelectArrayToColor(JsonConfigValue vaule);
	};

	template<typename T>
	struct UniqueGenerateCounter {
		T Counter = {};
		T AllocUniqueID() { return ++Counter; }
	};
	class NodesUniqueIDgenerate {
	protected:
		// points entity unique_id, points types unique_id.
		UniqueGenerateCounter<PointUid>  PointsGenUid  = {};
		UniqueGenerateCounter<PointType> PointTypesUid = {};

		// nodes entity unique_id, nodes types unique_id.
		UniqueGenerateCounter<NodeUid>  NodesGenUid  = {};
		UniqueGenerateCounter<NodeType> NodeTypesUid = {};

		UniqueGenerateCounter<LinkUid> LinksUid = {};
	};

	using TEMP_CALLBACK = std::function<void(const PSAnodesData::NodeAttributeTemplate&)>;
	// nodes data operation: add, delete, find. 
	class NodesDataOperation :public SystemJsonConfigDEC, 
		public NodesDataComponentManager, public NodesUniqueIDgenerate {
	protected:
		// nodes templates oper: add, delete, find.
		bool NodeTemplateADD(const PSAnodesData::NodeAttributeTemplate& temp);
		bool NodeTemplateDEL(const NodeType& unique);
		PSAnodesData::NodeAttributeTemplate NodeTemplateFIND(const NodeType& unique);

		void TemplatesForEach(TEMP_CALLBACK function);

		// nodes entites oper: generate, delete, find.
		bool NodeEntityGEN(
			const PSAnodesData::NodeAttributeTemplate& temp, const ImVec2& position, 
			NodeUid uid = PSAG_SYSTEM_ALLOC_UID
		);
		bool NodeEntityDEL(const NodeUid& unique);
		PSAnodesData::NodeAttributeEntity* NodeEntityFIND(const NodeUid& unique);

		// 2-calls: add load point types.
		bool SystemInitPointTypesLoad(JsonConfig config);
		// 2-calls: refresh load node types. clear => reload.
		bool SystemInitNodeTypesLoad(JsonConfig config);

		void SaveDatasetToEntites(
			const std::vector<PSAnodesData::SaveProjectNodeData>&     data_nodes,
			const std::vector<PSAnodesData::SaveProjectLineLinkData>& data_lines
		);

		void PointsEntityLink(PointUid p_begin, PointUid p_end);
		void PointsEntityLinkMonitor();
		// return flag: false: map empty.
		bool NodesPointsEntityDelete();
	};
}

namespace PSAnodesFiletool {
	StaticStrLABEL LABLogNodesFILETOOL = "NODES_FILESYS";

	// directory(lv1) iterator(filter) files.
	bool ForEachDirectoryFile(
		const std::string& directory_path, const std::string& file_filter,
		const std::function<void(const std::string&, const std::string&)>& callback
	);
	// safe filepath loader(assemble).
	std::string FilepathAssemble(
		const std::string& directory, 
		const std::string& name, 
		const std::string& extension
	);
	bool DeleteFile(const std::string& filepath);
}

namespace PSAnodesConvert {
	StaticStrLABEL LABLogNodesPROJ = "NODES_PROJECT";

	// return string data empty ? warning.
	std::string ExportProjectData(
		const PSAnodesData::ExportNodesSource& data, const std::string& u_name
	);
	bool ProjectDataLoader(
		const std::string& sstr_data,
		const std::string& u_name,
		std::vector<PSAnodesData::SaveProjectNodeData>*     ptr_nodes, 
		std::vector<PSAnodesData::SaveProjectLineLinkData>* ptr_lines,
		// reset alloter counter value.
		PSAnodesSystem::UniqueGenerateCounter<NodeUid>*  n_alloter,
		PSAnodesSystem::UniqueGenerateCounter<PointUid>* p_alloter
	);
}

namespace PSAnodesEncode {
	StaticStrLABEL LABLogNodesEncode = "NODES_ENCODE";
	// pomelo_star_studio node_editor encoder pointer to function.
	typedef std::string(*PSAencoderPFN)(const PSAnodesData::ExportNodesSource&);

	std::string PSAN_ENCODE_V101A(const PSAnodesData::ExportNodesSource& data);
}

using NodeTempsPtr = std::vector<PSAnodesData::NodeAttributeTemplate>*;
using NodeEntisPtr = std::vector<PSAnodesData::NodeAttributeEntity>*;

namespace PSAnodesRender {
	StaticStrLABEL LABLogNodesEditor = "NODES_EDITOR";

#define PROJECT_SAVE_FOLDER_LENGTH 1024
	class NodesEditorProject {
	protected:
		// params pair: f: file_path, s: project_name. 
		std::vector<std::pair<std::string, std::string>> ProjectPaths = {};

		std::string ProjectSaveFolder = {};
		std::string ProjectSaveName   = {};

		char ProjectSaveFolderTemp[PROJECT_SAVE_FOLDER_LENGTH] = {};
		char ProjectSaveNameTemp  [PROJECT_SAVE_FOLDER_LENGTH] = {};

		std::pair<std::string, std::string> ProjectCurrentPath = {};

		void RefashPathsData();

		ImVec2 StatusScroMouseMenu = {};
		ImVec2 StatusScroToolbar   = {};
	public:
		NodesEditorProject() {
			ProjectSaveName = "default";
			memcpy(ProjectSaveNameTemp, ProjectSaveName.data(), ProjectSaveName.size());
		}
	};

	class NodesEditorStatus {
	protected:
		float GetFramerateTimeStep();

		bool EditorStatusFlagMutexButton = false;
		bool EditorStatusFlagOperation   = false;

		ImVec4 STATUS_FLAG_COLOR(bool flag);

		float RUN_CYCLES_COUNT = 0.0f;
		ImVec4 RUN_STATUS_CYCLES();
	};

	class NodesEditorRender :
		public NodesEditorProject, public NodesEditorStatus, 
		public PSAnodesSystem::NodesDataOperation
	{
	protected:
		float WindowPropTitlebar = 0.04f;
		float WindowPropToolbar  = 0.18f;

		std::string TitlebarLogMsg = {};
		void SetTitlebarText(const std::string& text) {
			TitlebarLogMsg = text;
		}
		NodeUid ImNodesHoverUniqueNode = {};
		LinkUid ImNnodeHoverUniqueLine = {};

		bool MouseMenuOpenFlag = false;
		// mouse_menu: im_window, toolbar: non_window.
		void DrawEditorComponentMouseMenu(const ImVec2& size);
		void DrawEditorComponentToolbar();

		void DrawCoreNodesLines();
	public:
		NodesEditorRender(const std::string& f_aribs, const std::string& f_types);
		~NodesEditorRender();

		// set default search folder path.
		void NEP_SettingDefaultFolder(const std::string& path) {
			ProjectSaveFolder = path;
			memcpy(ProjectSaveFolderTemp, path.data(), path.size());
			RefashPathsData();
		}
		// set default save project name.
		void NEP_SettingDefaultName(const std::string& name) {
			ProjectSaveName = name;
			memcpy(ProjectSaveNameTemp, name.data(), name.size());
			RefashPathsData();
		}

		std::string  NDC_GetProjectName()   { return ProjectUniqueNAME; }
		NodeTempsPtr NDC_GetNodeTemplates() { return &NodeTemplates;    }
		NodeEntisPtr NDC_GetNodeEntities()  { return &NodeEntities;     }

		std::string ENC_ExportCurrentData(PSAnodesEncode::PSAencoderPFN funcptr);

		// rendering loop: frame event.
		void DrawEditorWindowFrame(
			const ImVec2&    size,
			bool             fixed = true,
			ImGuiWindowFlags flags = NULL
		);
	};
}

#endif
