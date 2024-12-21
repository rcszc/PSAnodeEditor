// psag_nodes_system_data. core data_format. RCSZ 2024_12_08.
// update: 2024_12_08 [0146]

#ifndef __PSAG_NODES_SYSTEM_DATA_HPP
#define __PSAG_NODES_SYSTEM_DATA_HPP
#include <vector>
#include <string>
#include <unordered_map>

#define IMGUI_USE_STB_TRUETYPE
#include "imgui.h"

#define PSAG_GRAPHICS_TEX_INVALID (uint32_t)0
using IconTextureHD = uint32_t;

using LinkUid  = int32_t;
using PointUid = int32_t;
using NodeUid  = int32_t;

using PointType = uint32_t;
using NodeType  = uint32_t;

// nodes & lines =enc=> data stream string.
using EncodeExport = std::string;

#define PSAG_INIT_COL ImVec4(1.0f, 1.0f, 1.0f, 1.0f)

// link point_type (unique_id,style).
struct PointTypeUid {
	PointType PointUniqueID;
	ImVec4    PointColor;

	constexpr PointTypeUid() : PointUniqueID(NULL), PointColor(PSAG_INIT_COL) {};
	constexpr PointTypeUid(uint32_t uid, const ImVec4& style) : 
		PointUniqueID(uid), PointColor(style)
	{};
};
// link node_type (unique_id,style).
struct NodeTypeUid {
	NodeType NodeUniqueID;
	uint64_t NodeVersion;

	constexpr NodeTypeUid() : NodeUniqueID(NULL), NodeVersion(NULL) {};
	constexpr NodeTypeUid(uint32_t uid, uint32_t version) : NodeUniqueID(uid), NodeVersion(version) {};
};

#define PSAG_SYSTEM_FILEEXT ".psanodes"
#define PSAG_SYSTEM_ALLOC_UID 0

#define PSAG_SYSTEM_SUCCESS true
#define PSAG_SYSTEM_FAILED  false

#define PSAG_NODES_DATA_FORMAT_VERSION 0b0001'0010'0001
namespace PSAnodesData {

	struct LinkPointInputSlot {
		PointTypeUid PointSlotType; // param unique id.

		std::string  PointText;   // params title text.
		PointUid     PointUnique; // global unique id.
		
		LinkUid LinkUnique; // 输入(参数)只能有单连接.
	};

	struct LinkPointOutputSlot {
		PointTypeUid PointSlotType; // param unique id.

		std::string  PointText;   // params title text.
		PointUid     PointUnique; // global unique id.

		std::vector<LinkUid> LinksUnique; // 输出(参数)可以有多连接.
	};

	struct LineLink {
		ImVec4 LineDrawColor;

		PointUid LinePointBegin; // begin: out.point
		PointUid LinePointEnd;   // end: in.point
	};

	struct NodeAttributeTemplate {
		// draw icon image file, graphics api load.
		std::string NodeRenderICON;

		NodeTypeUid NodeType;   // param unique id.

		ImVec4      NodeDrawColor; // titlebar & border color rgba.
		std::string NodeTitle;     // title text.
		ImVec2      NodePosition;  // draw position.
		float       NodeWidth;     // draw width.

		std::vector<LinkPointInputSlot>  InputPoints;
		std::vector<LinkPointOutputSlot> OutputPoints;
	};

	struct NodeAttributeEntity {
		// draw icon image, graphics api load.
		IconTextureHD NodeRenderICON;

		ImVec4      NodeDrawColor; // titlebar & border color rgba.
		std::string NodeTitle;     // title text.
		ImVec2      NodePosition;  // draw position.
		float       NodeWidth;     // draw width.

		std::vector<LinkPointInputSlot>  InputPoints;
		std::vector<LinkPointOutputSlot> OutputPoints;

		NodeTypeUid NodeType;   // param unique id.
		NodeUid     NodeUnique; // global unique id.

		// non link points copy dataset.
		inline bool TemplateLoader(const NodeAttributeTemplate& temp) {
			// template params copy.
			this->NodeDrawColor = temp.NodeDrawColor;
			this->NodeTitle     = temp.NodeTitle;
			this->NodePosition  = temp.NodePosition;
			this->NodeWidth     = temp.NodeWidth;

			this->NodeType = temp.NodeType;
			return temp.NodeType.NodeUniqueID != NULL;
		}
	};

	struct ExportNodesSource {
		// editor nodes & lines source dataset.
		std::vector<NodeAttributeEntity>      NodesData = {};
		std::unordered_map<LinkUid, LineLink> LinesData = {};
	};

	struct SaveProjectLineLinkData {
		// link begin(output) -> end(input).
		PointUid LinePointBegin;
		PointUid LinePointEnd;
	};

	struct SaveProjectNodeData {
		ImVec2 NodePosition;

		NodeTypeUid NodeType;
		NodeUid     NodeUnique;

		std::vector<PointUid> I_PointsUniqueID;
		std::vector<PointUid> O_PointsUniqueID;
	};
}

#endif