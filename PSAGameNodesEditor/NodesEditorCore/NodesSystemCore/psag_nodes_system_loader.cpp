// psag_nodes_system_loader.
#include <fstream>
#include <sstream>
#include "psag_nodes_system.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAnodesSystem {
	string SystemJsonConfigFileLoader::SourceStringFileRead(const string& filepath, bool* status) {
        ifstream READ_FILE_OBJ(filepath);
        // read_file status.
        if (!READ_FILE_OBJ.is_open()) {
            PushLogger(
                LogError, LABLogNodesSystem, "string file failed read, path: %s", 
                filepath.c_str()
            );
            *status = PSAG_SYSTEM_FAILED;
            return string();
        }
        // read source string data.
        string Content((istreambuf_iterator<char>(READ_FILE_OBJ)), istreambuf_iterator<char>());
        READ_FILE_OBJ.close();
        PushLogger(
            LogInfo, LABLogNodesSystem, "string file read, size: %u, path: %s",
            Content.size(), filepath.c_str()
        );
        *status = PSAG_SYSTEM_SUCCESS;
        return Content;
	}

    JsonConfig SystemJsonConfigFileLoader::SourceStringDecode(const string& config, bool* status) {
        rapidjson::Document DecoderOutputTemp = {};
        string DecoderStrTemp = config;

        DecoderOutputTemp.Parse(DecoderStrTemp.c_str());
        // check rapidjson error flag | check object. 
        if (DecoderOutputTemp.HasParseError() || !DecoderOutputTemp.IsObject()) {
            PushLogger(LogError, LABLogNodesSystem, "decode failed src_string to json_doc.");
            *status = PSAG_SYSTEM_FAILED;
        }
        *status = PSAG_SYSTEM_SUCCESS;
        return DecoderOutputTemp;
    }

	bool SystemJsonConfigFileLoader::SourceStringFileWrite(const string& filepath, const string& str) {
		ofstream FileWrite(filepath);
		if (FileWrite.is_open()) {
			// str content =write=> file.
			FileWrite << str;
			FileWrite.close();

			PushLogger(
				LogInfo, LABLogNodesSystem, "string file write, size: %u, path: %s",
				str.size(), filepath.c_str()
			);
			return PSAG_SYSTEM_SUCCESS;
		}
		PushLogger(
			LogError, LABLogNodesSystem, "string file failed write, path: %s",
			filepath.c_str()
		);
		return PSAG_SYSTEM_FAILED;
	}

#define COLOR_FORMAT_MUL8255 0.0039216f
    // 255.0 * value = 1.000008, +- 0.000008
    ImVec4 SystemJsonConfigDEC::SelectArrayToColor(JsonConfigValue vaule) {
        if (vaule.IsArray()) {
            ImVec4 ColorFormatResult(
                (float)vaule[0].GetInt() * COLOR_FORMAT_MUL8255,
                (float)vaule[1].GetInt() * COLOR_FORMAT_MUL8255,
                (float)vaule[2].GetInt() * COLOR_FORMAT_MUL8255,
                (float)vaule[3].GetInt() * COLOR_FORMAT_MUL8255
            );
            return ColorFormatResult;
        }
        PushLogger(LogError, LABLogNodesSystem, "json config_value not array.");
        return ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

	bool NodesDataOperation::SystemInitPointTypesLoad(JsonConfig config) {
		// load all point types (iterator).
		for (auto it = config.MemberBegin(); it != config.MemberEnd(); ++it) {
			string PointTypeStr = it->name.GetString();
			// check repeat point_type.
			if (PointTypes.find(PointTypeStr) != PointTypes.end()) {
				PushLogger(LogWarning, LABLogNodesSystem, "point type load repeat_key: %s", PointTypeStr.c_str());
				continue;
			}
			PointTypeUid UniqueTypeTemp = {};
			// unique id: draw link_point color & unique number.
			UniqueTypeTemp.PointColor    = SelectArrayToColor(it->value);
			UniqueTypeTemp.PointUniqueID = PointTypesUid.AllocUniqueID();

			PointTypes[PointTypeStr] = UniqueTypeTemp;
			PushLogger(
				LogInfo, LABLogNodesSystem, "point type load tid: %u, name: %s",
				UniqueTypeTemp.PointUniqueID, PointTypeStr.c_str()
			);
		}
		return !config.ObjectEmpty();
	}

	bool NodesDataOperation::SystemInitNodeTypesLoad(JsonConfig config) {
		NodeTempMapper.clear();
		NodeTemplates.clear();
		// load all node types(iterator).
		for (auto it = config.MemberBegin(); it != config.MemberEnd(); ++it) {
			string CheckName = it->name.GetString();
			// 加载项目(全局)名称标识符.
			if (CheckName == "ProjectUniqueName" && it->value.IsString()) {
				ProjectUniqueNAME = it->value.GetString();
				PushLogger(
					LogInfo, LABLogNodesSystem, "node project u_name: %s",
					ProjectUniqueNAME.c_str()
				);
				continue;
			}
			if (!it->value.IsObject()) {
				PushLogger(LogWarning, LABLogNodesSystem, "node type load value not obj.");
				continue;
			}
			PSAnodesData::NodeAttributeTemplate NodeTemplate = {};

			NodeTemplate.NodeType.NodeVersion  = it->value["Version"].GetUint64();
			NodeTemplate.NodeType.NodeUniqueID = NodeTypesUid.AllocUniqueID();

			NodeTemplate.NodeDrawColor = SelectArrayToColor(it->value["Color"]);
			NodeTemplate.NodeTitle     = it->name.GetString();
			NodeTemplate.NodeWidth     = it->value["Width"].GetFloat();
			NodeTemplate.NodePosition  = ImVec2(0.0f, 0.0f);

			// node template points: type, text
			// node entity   points: point_unique, link_unique

			// input link points params.
			for (auto& InPointType : it->value["InputPoints"].GetArray()) {
				PSAnodesData::LinkPointInputSlot SlotDataTemp = {};

				SlotDataTemp.PointSlotType = PointTypes[InPointType.GetString()];
				SlotDataTemp.PointText     = InPointType.GetString();

				NodeTemplate.InputPoints.push_back(SlotDataTemp);
			}
			// output link points params.
			for (auto& OutPointType : it->value["OutputPoints"].GetArray()) {
				PSAnodesData::LinkPointOutputSlot SlotDataTemp = {};

				SlotDataTemp.PointSlotType = PointTypes[OutPointType.GetString()];
				SlotDataTemp.PointText     = OutPointType.GetString();

				NodeTemplate.OutputPoints.push_back(SlotDataTemp);
			}
			PushLogger(
				LogInfo, LABLogNodesSystem, "node type load tid: %u, name: %s",
				NodeTemplate.NodeType.NodeUniqueID, it->name.GetString()
			);
			NodeTemplates.push_back(NodeTemplate);
			NodeTempMapper[NodeTemplate.NodeType.NodeUniqueID] = NodeTemplates.size() - 1;
		}
		return !config.ObjectEmpty();
	}

	void NodesDataOperation::SaveDatasetToEntites(
		const vector<PSAnodesData::SaveProjectNodeData>&     data_nodes,
		const vector<PSAnodesData::SaveProjectLineLinkData>& data_lines
	) {
		NodeEntities.clear();
		LinkLineEntities.clear();

		// node entities.
		for (auto& Item : data_nodes) {
			PSAnodesData::NodeAttributeEntity Temp = {};

			auto Template = NodeTemplateFIND(Item.NodeType.NodeUniqueID);
			Temp.TemplateLoader(Template);

			Temp.InputPoints  = Template.InputPoints;
			Temp.OutputPoints = Template.OutputPoints;

			Temp.NodeUnique   = Item.NodeUnique;
			Temp.NodePosition = Item.NodePosition;
			
			for (size_t i = 0; i < Temp.InputPoints.size(); ++i)
				Temp.InputPoints[i].PointUnique = Item.I_PointsUniqueID[i];

			for (size_t i = 0; i < Temp.OutputPoints.size(); ++i)
				Temp.OutputPoints[i].PointUnique = Item.O_PointsUniqueID[i];

			ImNodes::SetNodeGridSpacePos(Temp.NodeUnique, Temp.NodePosition);
			NodeEntities.push_back(Temp);
		}
		// line entities.
		for (auto& Item : data_lines)
			PointsEntityLink(Item.LinePointBegin, Item.LinePointEnd);
	}
}

#define STREAM_LABLE_GLOBALUNAME "proj_u_name"
#define STREAM_LABLE_NODEENDFLAG "node_params_end"

#define STREAM_LABLE_TYPE "tid"
#define STREAM_LABLE_EUID "uid"
#define STREAM_LABLE_EPOS "pos"

#define STREAM_LABLE_PIN  "point_i"
#define STREAM_LABLE_POUT "point_o"

#define STREAM_LABLE_LINES "line"
namespace PSAnodesConvert {

	string ExportProjectData(const PSAnodesData::ExportNodesSource& data, const string& u_name) {
		if (data.NodesData.empty() && data.LinesData.empty())
			PushLogger(LogWarning, LABLogNodesPROJ, "export nodes data empty.");

		stringstream ISSdata = {};

		ISSdata << STREAM_LABLE_GLOBALUNAME << " " << u_name << endl;
		// node entites data => stream.
		for (size_t i = 0; i < data.NodesData.size(); ++i) {

			ISSdata << STREAM_LABLE_TYPE << " " << data.NodesData[i].NodeType.NodeUniqueID << endl;
			ISSdata << STREAM_LABLE_EUID << " " << data.NodesData[i].NodeUnique << endl;
			ISSdata << STREAM_LABLE_EPOS << " " << data.NodesData[i].NodePosition.x << " " << data.NodesData[i].NodePosition.y << endl;

			for (size_t j = 0; j < data.NodesData[i].InputPoints.size(); ++j)
				ISSdata << STREAM_LABLE_PIN << " " << data.NodesData[i].InputPoints[j].PointUnique << endl;

			for (size_t j = 0; j < data.NodesData[i].OutputPoints.size(); ++j)
				ISSdata << STREAM_LABLE_POUT << " " << data.NodesData[i].OutputPoints[j].PointUnique << endl;

			ISSdata << STREAM_LABLE_NODEENDFLAG << endl;
		}
		// line entities data => stream.
		for (const auto& Line : data.LinesData)
			ISSdata << STREAM_LABLE_LINES << " " << Line.second.LinePointBegin << " " << Line.second.LinePointEnd << endl;

		PushLogger(
			LogInfo, LABLogNodesPROJ, "export nodes data stream_string: %u bytes", 
			ISSdata.str().size()
		);
		return ISSdata.str();
	}

	bool ProjectDataLoader(
		const string& sstr_data,
		const string& u_name,
		vector<PSAnodesData::SaveProjectNodeData>*     ptr_nodes,
		vector<PSAnodesData::SaveProjectLineLinkData>* ptr_lines,
		// reset alloter counter value.
		PSAnodesSystem::UniqueGenerateCounter<NodeUid>*  n_alloter,
		PSAnodesSystem::UniqueGenerateCounter<PointUid>* p_alloter
	) {
		if (ptr_nodes == nullptr || ptr_lines == nullptr) {
			PushLogger(LogError, LABLogNodesPROJ, "project load failed, nullptr.");
			return false;
		}
		string LineRead = {}, HeaderRead = {};

		NodeUid  NmaxUniqueID = NULL;
		PointUid PmaxUniqueID = NULL;

		bool IS_PROJ_VALID = PSAG_SYSTEM_FAILED;

		PSAnodesData::SaveProjectNodeData     NodeDataTemp = {};
		PSAnodesData::SaveProjectLineLinkData LineDataTemp = {};

		auto PointMaxFunc = [&](const PointUid& uid) {
			PmaxUniqueID = uid > PmaxUniqueID ? uid : PmaxUniqueID;
		};
		istringstream ISSdata(sstr_data);
		while (getline(ISSdata, LineRead)) {
			istringstream DataStrLine(LineRead);

			DataStrLine >> HeaderRead;

			if (HeaderRead == STREAM_LABLE_GLOBALUNAME) {
				string CheckUname = {};
				DataStrLine >> CheckUname;
				// check project unique_name.
				if (u_name != CheckUname) {
					PushLogger(LogError, LABLogNodesPROJ, "project load failed, u_name invalid.");
					// clear invalid data.
					ptr_nodes->clear(); ptr_lines->clear();
					return PSAG_SYSTEM_FAILED;
				}
				IS_PROJ_VALID = PSAG_SYSTEM_SUCCESS;
				hash<string> StringHash;
				size_t NameHashCode = StringHash(CheckUname);
			}

			if (HeaderRead == STREAM_LABLE_TYPE)
				DataStrLine >> NodeDataTemp.NodeType.NodeUniqueID;

			if (HeaderRead == STREAM_LABLE_EUID) {
				DataStrLine >> NodeDataTemp.NodeUnique;
				// node unique id max_value.
				NmaxUniqueID = NodeDataTemp.NodeUnique > NmaxUniqueID ? 
					NodeDataTemp.NodeUnique : NmaxUniqueID;
			}

			if (HeaderRead == STREAM_LABLE_EPOS)
				DataStrLine >> NodeDataTemp.NodePosition.x >> NodeDataTemp.NodePosition.y;

			if (HeaderRead == STREAM_LABLE_PIN) {
				int32_t PointSlotUID = NULL;
				DataStrLine >> PointSlotUID;

				PointMaxFunc(PointSlotUID);
				NodeDataTemp.I_PointsUniqueID.push_back(PointSlotUID);
			}

			if (HeaderRead == STREAM_LABLE_POUT) {
				int32_t PointSlotUID = NULL;
				DataStrLine >> PointSlotUID;

				PointMaxFunc(PointSlotUID);
				NodeDataTemp.O_PointsUniqueID.push_back(PointSlotUID);
			}

			if (HeaderRead == STREAM_LABLE_NODEENDFLAG) {
				ptr_nodes->push_back(NodeDataTemp);
				// clear data temp.
				NodeDataTemp.I_PointsUniqueID.clear();
				NodeDataTemp.O_PointsUniqueID.clear();
			}

			if (HeaderRead == STREAM_LABLE_LINES) {
				DataStrLine >> LineDataTemp.LinePointBegin >> LineDataTemp.LinePointEnd;
				ptr_lines->push_back(LineDataTemp);
			}
		}
		if (!IS_PROJ_VALID) {
			PushLogger(LogError, LABLogNodesPROJ, "project load failed, project invalid.");
			// clear invalid data.
			ptr_nodes->clear(); ptr_lines->clear();
			return PSAG_SYSTEM_FAILED;
		}
		// reset counter.
		n_alloter->Counter = NmaxUniqueID;
		p_alloter->Counter = PmaxUniqueID;
		return PSAG_SYSTEM_SUCCESS;
	}
}