// psag_nodes_system_operation.
#include "psag_nodes_system.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAnodesSystem {
	bool NodesDataOperation::NodeTemplateADD(const PSAnodesData::NodeAttributeTemplate& temp) {
		if (NodeTempMapper.find(temp.NodeType.NodeUniqueID) == NodeTempMapper.end()) {
			NodeTemplates.push_back(temp);
			// vector back index = size - 1.
			NodeTempMapper[temp.NodeType.NodeUniqueID] = NodeTemplates.size() - 1;
			PushLogger(
				LogInfo, LABLogNodesSystem, "node template(add) tid: %u", 
				temp.NodeType.NodeUniqueID
			);
			return PSAG_SYSTEM_SUCCESS;
		}
		PushLogger(
			LogWarning, LABLogNodesSystem, "node template(add) repeat_key tid: ", 
			temp.NodeType.NodeUniqueID
		);
		return PSAG_SYSTEM_FAILED;
	}

	bool NodesDataOperation::NodeTemplateDEL(const NodeType& unique) {
		if (NodeTempMapper.find(unique) != NodeTempMapper.end()) {
			// swap current(delete) => back.
			swap(NodeTemplates[NodeTempMapper[unique]], NodeTemplates.back());

			// copy current(delete) => back index_addr.
			NodeTempMapper[NodeTemplates.back().NodeType.NodeUniqueID]
				= NodeTempMapper[unique];

			// delete mapper(hash_map) index & temp.
			NodeTempMapper.erase(unique);
			// print type_id => delete template.
			PushLogger(
				LogInfo, LABLogNodesSystem, "node template(delete) tid: %u",
				NodeTemplates.back().NodeType.NodeUniqueID
			);
			NodeTemplates.pop_back();
			return PSAG_SYSTEM_SUCCESS;
		}
		PushLogger(
			LogWarning, LABLogNodesSystem, "node template(delete) not_key."
		);
		return PSAG_SYSTEM_FAILED;
	}

	PSAnodesData::NodeAttributeTemplate NodesDataOperation::NodeTemplateFIND(const NodeType& unique) {
		// unique => find mapper index => template.
		return NodeTempMapper.find(unique) != NodeTempMapper.end() ?
			NodeTemplates[NodeTempMapper[unique]] : PSAnodesData::NodeAttributeTemplate();
	}

	void NodesDataOperation::TemplatesForEach(TEMP_CALLBACK function) {
		for (auto& Template : NodeTemplates)
			function(Template);
	}

	bool NodesDataOperation::NodeEntityGEN(
		const PSAnodesData::NodeAttributeTemplate& temp, const ImVec2& position, 
		NodeUid uid
	) {
		PSAnodesData::NodeAttributeEntity CreateEntity = {};

		CreateEntity.InputPoints  = temp.InputPoints;
		CreateEntity.OutputPoints = temp.OutputPoints;

		// alloc entity node => alloc entity points unique_id.
		for (auto& InPoint  : CreateEntity.InputPoints)  InPoint.PointUnique  = PointsGenUid.AllocUniqueID();
		for (auto& OutPoint : CreateEntity.OutputPoints) OutPoint.PointUnique = PointsGenUid.AllocUniqueID();

		CreateEntity.NodePosition = position;
		CreateEntity.NodeUnique   = uid;

		if (CreateEntity.NodeUnique == PSAG_SYSTEM_ALLOC_UID)
			CreateEntity.NodeUnique = NodesGenUid.AllocUniqueID();

		ImNodes::SetNodeGridSpacePos(CreateEntity.NodeUnique, position);
		// template params =copy=> entity(temp).
		bool StatusFlag = CreateEntity.TemplateLoader(temp);
		NodeEntities.push_back(CreateEntity);
		return StatusFlag;
	}

	bool NodesDataOperation::NodeEntityDEL(const NodeUid& unique) {
		bool NodeFind = PSAG_SYSTEM_FAILED;

		// find delete_item => swap to back => pop delete back.
		for (size_t i = 0; i < NodeEntities.size(); ++i) {
			if (NodeEntities[i].NodeUnique == unique) {
				swap(NodeEntities[i], NodeEntities.back());
				NodeFind = PSAG_SYSTEM_SUCCESS;
				break;
			}
		}
		if (NodeFind == PSAG_SYSTEM_SUCCESS) {
			// delete point.in line.
			for (auto& InPoint : NodeEntities.back().InputPoints)
				LinkLineEntities.erase(InPoint.LinkUnique);

			// delete point.out.array line.
			for (auto& OutPoint : NodeEntities.back().OutputPoints)
				for (auto& LinkLines : OutPoint.LinksUnique)
					LinkLineEntities.erase(LinkLines);
		}
		NodeFind == PSAG_SYSTEM_FAILED ?
			PushLogger(LogWarning, LABLogNodesSystem, "node entity(delete) not_key.") :
			PushLogger(LogInfo, LABLogNodesSystem, "node entity(delete) key: %d", unique);
		NodeEntities.pop_back();
		return NodeFind;
	}

	PSAnodesData::NodeAttributeEntity* NodesDataOperation::NodeEntityFIND(const NodeUid& unique) {
		for (size_t i = 0; i < NodeEntities.size(); ++i)
			if (NodeEntities[i].NodeUnique == unique)
				return &NodeEntities[i];
		return nullptr;
	}

	void NodesDataOperation::PointsEntityLink(PointUid p_begin, PointUid p_end) {
		PSAnodesData::LineLink CreateLinkLine = {};
		PointType PointTypeTemp[2] = {};
		// 0: point.in.i, 1: point.in.j, 2: point.out.i, 3: point.out.j
		size_t IndexesTemp[4] = {};

		CreateLinkLine.LinePointBegin = p_begin;
		CreateLinkLine.LinePointEnd   = p_end;

		for (size_t i = 0; i < NodeEntities.size(); ++i) {
			// node.input.points
			for (size_t j = 0; j < NodeEntities[i].InputPoints.size(); ++j) {
				// input 连接点所在 node => 互连标识 link_id.
				if ((NodeEntities[i].InputPoints[j].PointUnique == CreateLinkLine.LinePointBegin) ||
					(NodeEntities[i].InputPoints[j].PointUnique == CreateLinkLine.LinePointEnd))
				{
					IndexesTemp[0] = i;
					IndexesTemp[1] = j;
					PointTypeTemp[0] = NodeEntities[i].
						InputPoints[j].PointSlotType.PointUniqueID;
				}
			}
			// node.output.points
			for (size_t j = 0; j < NodeEntities[i].OutputPoints.size(); ++j) {
				// output 连接点所在 node => 互连标识 link_id.
				if ((NodeEntities[i].OutputPoints[j].PointUnique == CreateLinkLine.LinePointBegin) ||
					(NodeEntities[i].OutputPoints[j].PointUnique == CreateLinkLine.LinePointEnd))
				{
					IndexesTemp[2] = i;
					IndexesTemp[3] = j;
					PointTypeTemp[1] = NodeEntities[i].
						OutputPoints[j].PointSlotType.PointUniqueID;
				}
			}
		}
		// 参数校验连接节点参数槽.
		// link check: out.param type = in.param type.
		if (PointTypeTemp[0] == PointTypeTemp[1]) {
			// first: delete old_link_line.
			LinkLineEntities.erase(
				NodeEntities[IndexesTemp[0]].InputPoints[IndexesTemp[1]].LinkUnique
			);
			auto UniqueIDAlloc = LinksUid.AllocUniqueID();

			NodeEntities[IndexesTemp[0]].InputPoints[IndexesTemp[1]].LinkUnique = UniqueIDAlloc;
			NodeEntities[IndexesTemp[2]].OutputPoints[IndexesTemp[3]].LinksUnique.push_back(UniqueIDAlloc);

			CreateLinkLine.LineDrawColor =
				NodeEntities[IndexesTemp[0]].InputPoints[IndexesTemp[1]].PointSlotType.PointColor;
			// params setting => create link.
			LinkLineEntities.insert(make_pair(UniqueIDAlloc, CreateLinkLine));
			PushLogger(LogInfo, LABLogNodesSystem, "create entity link line, uid: %u", UniqueIDAlloc);
			return;
		}
		PushLogger(LogWarning, LABLogNodesSystem, "create entity link failed.");
	}

	void NodesDataOperation::PointsEntityLinkMonitor() {
		PointUid PointBegin = 0, PointEnd = 0;
		// imnodes create link event => link.
		if (ImNodes::IsLinkCreated(&PointBegin, &PointEnd))
			PointsEntityLink(PointBegin, PointEnd);
	}

	bool NodesDataOperation::NodesPointsEntityDelete() {
		size_t ClearSize[2] = { NodeEntities.size(),LinkLineEntities.size() };

		NodeEntities.clear();
		LinkLineEntities.clear();

		PushLogger(
			LogInfo, LABLogNodesSystem, "clear entites nodes: %u, lines: %u", 
			ClearSize[0], ClearSize[1]
		);
		return NodeEntities.empty() || LinkLineEntities.empty();
	}
}