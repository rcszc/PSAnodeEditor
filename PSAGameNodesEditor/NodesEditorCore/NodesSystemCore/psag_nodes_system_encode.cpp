// psag_nodes_system_encode.
#include <sstream>
#include "psag_nodes_system.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAnodesEncode {
#define V1PENC_LABLE_NODEENDFLAG "node_params_end"
#define V1PENC_LABLE_NODELINK    "node_link"

#define V1PENC_LABLE_TYPE "tid"
#define V1PENC_LABLE_EUID "uid"
#define V1PENC_LABLE_NAME "name"

#define V1PENC_LABLE_PIN  "point_i"
#define V1PENC_LABLE_POUT "point_o"

	string PSAN_ENCODE_V101A(const PSAnodesData::ExportNodesSource& data) {
		if (data.NodesData.empty() && data.LinesData.empty())
			PushLogger(LogWarning, LABLogNodesEncode, "encode nodes data empty.");

		stringstream ISSdata = {};

		// node entites data => stream.
		for (size_t i = 0; i < data.NodesData.size(); ++i) {

			ISSdata << V1PENC_LABLE_NAME << " " << data.NodesData[i].NodeTitle << endl;
			ISSdata << V1PENC_LABLE_TYPE << " " << data.NodesData[i].NodeType.NodeUniqueID << endl;
			ISSdata << V1PENC_LABLE_EUID << " " << data.NodesData[i].NodeUnique << endl;

			for (size_t j = 0; j < data.NodesData[i].InputPoints.size(); ++j) {
				ISSdata << V1PENC_LABLE_PIN << " "
					<< data.NodesData[i].InputPoints[j].PointUnique << " "
					<< data.NodesData[i].InputPoints[j].PointText
					<< endl;
			}

			for (size_t j = 0; j < data.NodesData[i].OutputPoints.size(); ++j) {
				ISSdata << V1PENC_LABLE_POUT << " "
					<< data.NodesData[i].OutputPoints[j].PointUnique << " "
					<< data.NodesData[i].OutputPoints[j].PointText
					<< endl;
			}
			ISSdata << V1PENC_LABLE_NODEENDFLAG << endl;
		}
		// line entities data => stream.
		for (const auto& Line : data.LinesData)
			ISSdata << V1PENC_LABLE_NODELINK << " " << Line.second.LinePointBegin << " " << Line.second.LinePointEnd << endl;

		PushLogger(
			LogInfo, LABLogNodesEncode, "encode nodes data stream_string: %u bytes",
			ISSdata.str().size()
		);
		PushLogger(LogInfo, LABLogNodesEncode, "encoder version 1.01 ahpla.");
		return ISSdata.str();
	}
}

namespace PSAnodesRender {
	string NodesEditorRender::ENC_ExportCurrentData(PSAnodesEncode::PSAencoderPFN funcptr) {
		PSAnodesData::ExportNodesSource DataTemp = {};
		DataTemp.NodesData = NodeEntities;
		DataTemp.LinesData = LinkLineEntities;
		return funcptr(DataTemp);
	}
}