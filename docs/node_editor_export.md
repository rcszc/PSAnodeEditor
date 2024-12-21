# Node Data Export

__Update:__ `2024.12.21` `V101` `RCSZ`

---

### 调用编辑器数据导出
- __V101:__ 目前只有 V101 一种默认的字符串流编码器.

```cpp
// 例: 使用 "PSAnodesEncode::PSAN_ENCODE_V101A" 编码器进行导出.
std::string Temp = NodesEditor->ENC_ExportCurrentData(PSAnodesEncode::PSAN_ENCODE_V101A);
```

---

### 导出数据格式
- __V101:__ 是的我们可以从格式看出与 .obj 模型文件的流编码方式有相似之处, 前半段描述了所有节点的必要属性以及连接的唯一标识(UID), 后半段描述了哪些点它们是相连的, 目前并没有循环依赖拓扑检查. 

```cpp
// 示例数据:

name Test.A       // 节点名称
tid 1             // 节点类型 UID
uid 1             // 节点实体 UID
point_i 1 Float32 // 输入连接点 UID 类型名称
point_i 2 Int32
point_i 3 Int32
point_o 4 Int32   // 输出连接点 UID 类型名称
point_o 5 Double
node_params_end   // 节点参数组描述 结束标识
name Test.Component.IN
tid 7
uid 2
point_o 6 Float32
point_o 7 Float32
point_o 8 String
node_params_end
name Test.E
tid 5
uid 3
point_i 9 Float32
point_i 10 Int32
point_o 11 Int32
point_o 12 Float32
node_params_end
node_link 6 1 // 连接点描述 连接对 UID-UID
node_link 7 9
```