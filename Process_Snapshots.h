//
// Created by MoCuishle on 2019/11/29.
//

#ifndef IG_NOOP_5_PROCESS_SNAPSHOTS_H
#define IG_NOOP_5_PROCESS_SNAPSHOTS_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <bitset>
#include <iomanip>
#include <time.h>

#include "Lifespan.h"
#include "Graph.h"
#include "SCCTable.h"

//typedef pair<SCCEdge, set<NodeEdge>> SCCEdgeInfoItem;
typedef struct {
    SCCEdge sccEdge;
    mutable set<NodeEdge> nodeEdges;
} SCCEdgeInfoItem;
bool operator<(const SCCEdgeInfoItem& a, const SCCEdgeInfoItem& b) {
    return a.sccEdge < b.sccEdge;
}
typedef set<SCCEdgeInfoItem> SCCEdgeInfo;
typedef vector<SCCEdgeInfo> SCCEdgeInfoSequence;
SccTable GetSCCTable(int timeIntervalLength, Graphs& originGraph, vector<vector<int>>& evolvingGraphSequence, SCCEdgeInfoSequence& sccEdgesSequence, double& buildSccTableTime);
vector<int> GetFileData1(string fileAddressHead, int timeStamp);
vector<int> GetFileData2(string fileAddressHead, int timeStamp);
int getSccId(int vertexId, int timestamp, SccTable sccTable);
void UpdateSccTable(int numOfSCC, map<int, int> &v2s, SccTable &st, int &timeStamp, int &sccId, Graphs& originGraph);
SCCEdgeInfo BuildCurDAGEdgesDataSequenceFromIndex(Graph graph, SccTable &st, map<int, int> &index_ST, int &timeStamp);
map<int, int> BuildIndexOfCurSccTable(SccTable &st, int &timeStamp);

//----------------------------
//问题：不同时间的 SCCID 是如何分配的？
SccTable GetSCCTable(int timeIntervalLength, Graphs& originGraph, vector<vector<int>> &evolvingGraphSequence,
                     SCCEdgeInfoSequence& sccEdgesSequence, double &buildSccTableTime) {
    //st 变量是全局时间有效的，对于每一个时间戳都有效
    SccTable st;
    int sccId = 0;

    for (int timeStamp = 0; timeStamp < timeIntervalLength; ++timeStamp) { 
        //计算图中SCC
        originGraph[timeStamp].CalculateConnection();
        originGraph[timeStamp].SumScc();
        int numOfSCC = originGraph[timeStamp].GetConnectedCount();
        map<int, int> v2s = originGraph[timeStamp].GetMapV2S();

        //更新SCC-Table
        clock_t buildSCCT_startTime, buildSCCT_endTime;
        buildSCCT_startTime = clock();
        UpdateSccTable(numOfSCC, v2s, st, timeStamp, sccId, originGraph);
        buildSCCT_endTime = clock();
        double updateTime = (double) (buildSCCT_endTime - buildSCCT_startTime);
        buildSccTableTime += updateTime;

        //构建SCC-Table的索引
        int numOfV = originGraph[timeStamp].GetVexNum();
        map<int, int> index_curST = BuildIndexOfCurSccTable(st, timeStamp);
        //index_curST: key:节点的编号 value:该节点所在的SCC的编号
        SCCEdgeInfo sccEdges = BuildCurDAGEdgesDataSequenceFromIndex(originGraph[timeStamp], st, index_curST, timeStamp);
        vector<int> tmp;
        for (auto it = sccEdges.begin(); it != sccEdges.end(); it++) {
            tmp.push_back(it->sccEdge.sScc);
            tmp.push_back(it->sccEdge.tScc);
        }
        evolvingGraphSequence.push_back(tmp);
        sccEdgesSequence.push_back(sccEdges);
    }
    return st;
}

SccTable GetSCCTableFromOneGraph(int timeStamp, Graph* originGraph, vector<int>& evolvingGraph, SCCEdgeInfo& sccEdgeInfo) {
    SccTable st;
    int sccId = 0;
    originGraph->CalculateConnection();
    originGraph->SumScc();
    int numOfSCC = originGraph->GetConnectedCount();
    map<int, int> v2s = originGraph->GetMapV2S();

    //更新SCC-Table
    map<int, set<int>> sccSet;
    for (auto it = v2s.begin(); it != v2s.end(); it++) {
        int curV = (*it).first;
        int curS = (*it).second;
        if (sccSet.find(curS) == sccSet.end()) {
            set<int> tmp;
            tmp.insert(curV);
            sccSet[curS] = tmp;
        }
        else {
            sccSet[curS].insert(curV);
        }
    }
    for (int m = 0; m < numOfSCC; ++m) {
        set <int> thisSet = sccSet[m];
        auto findScc = find_if(st.begin(), st.end(), [&thisSet](const SCCTableItem& p) {return thisSet == p.nodeGroup;});
        if (findScc != st.end()) {
            (*findScc).sccID_Life.life_time.set(timeStamp);
            cout << "SCCIDremap timeStamp" << timeStamp << endl;
            cout << findScc->sccID_Life.scc_id << endl;
            originGraph->SCCIDremap(thisSet, findScc->sccID_Life.scc_id);
        }
        else {
            //sccTable中不存在该scc记录，增加该记录，并更新id
            SccID_Life lifeId;
            lifeId.life_time.set(timeStamp);
            lifeId.scc_id = sccId;
            SCCTableItem item;
            item.sccID_Life = lifeId;
            item.nodeGroup = sccSet[m];
            auto res = st.insert(item);
            if (!res.second) assert(false);
            originGraph->SCCIDremap(thisSet, sccId);
            sccId++;
        }
    }
    //构建SCC-Table的索引
    int numOfV = originGraph->GetVexNum();
    map<int, int> index_curST = BuildIndexOfCurSccTable(st, timeStamp);
    sccEdgeInfo = BuildCurDAGEdgesDataSequenceFromIndex(*originGraph, st, index_curST, timeStamp);
    for (auto it = sccEdgeInfo.begin(); it != sccEdgeInfo.end(); it++) {
        evolvingGraph.push_back(it->sccEdge.sScc);
        evolvingGraph.push_back(it->sccEdge.tScc);
    }
    return st;
}

int getSccId(int vertexId, int timestamp, SccTable sccTable) {
    //在SccTable中找到源节点对应的SCC_id
    for (auto record = sccTable.begin(); record != sccTable.end(); record++) {
        if ((*record).sccID_Life.life_time.test(timestamp)) {
            //在该行记录的SCC部分查找是否包含该节点
            auto findVertex = (*record).nodeGroup.find(vertexId);
            if (findVertex != (*record).nodeGroup.end()) {
                //该行包含该节点
                int SccID = (*record).sccID_Life.scc_id;
                return SccID;
            } else {
                //该行不含该节点,继续查询后续记录
                continue;
            }
        } else {
            continue;
        }
    }

    return -1;
}

vector<int> GetFileData1(string fileAddressHead, int timeStamp) {
    string fileAddressTail = to_string(timeStamp) + ".txt";
    string fileAddress = fileAddressHead + fileAddressTail;
    vector<int> dataVector;
    int data;
    ifstream fin;
    fin.open(fileAddress);
    if (fin) {
        while (fin >> data) {
            dataVector.push_back(data);
        }
    }
    fin.close();
    return dataVector;
}

vector<int> GetFileData2(string fileAddressHead, int timeStamp) {
    string fileAddressTail = to_string(timeStamp) + ".txt";
    string fileAddress = fileAddressHead + fileAddressTail;
    vector<int> dataVector;
    ifstream fin;
    fin.open(fileAddress);
    int src, dst, ts;
    if (fin) {
        while (fin >> src >> dst >> ts) {
            dataVector.push_back(src);
            dataVector.push_back(dst);
        }
    }
    fin.close();
    return dataVector;
}

void UpdateSccTable(int numOfSCC, map<int, int>& v2s, SccTable& st, int& timeStamp, int& sccId, Graphs& originGraph) {
    map<int, set<int>> sccSet;
    //sccset: key:SCC的编号 value:该SCC中所有节点的集合
    for (auto it = v2s.begin(); it != v2s.end(); it++) {
        int curV = (*it).first;
        int curS = (*it).second;
        if (sccSet.find(curS) == sccSet.end()) {
            set<int> tmp;
            tmp.insert(curV);
            sccSet[curS] = tmp;
        }
        else sccSet[curS].insert(curV);
    }
    
    for (int m = 0; m < numOfSCC; ++m) {
        set <int> thisSet = sccSet[m];
        auto findScc = find_if(st.begin(), st.end(), [&thisSet](const SCCTableItem &p) {return thisSet == p.nodeGroup;});
        if (findScc != st.end()) {
            (*findScc).sccID_Life.life_time.set(timeStamp);
            originGraph[timeStamp].SCCIDremap(thisSet, findScc->sccID_Life.scc_id);
        } else {
            //sccTable中不存在该scc记录，增加该记录，并更新id
            SccID_Life lifeId;
            lifeId.life_time.set(timeStamp);
            lifeId.scc_id = sccId;
            SCCTableItem item;
            item.sccID_Life = lifeId;
            item.nodeGroup = sccSet[m];
            auto res = st.insert(item);
            if (!res.second) assert(false);
            originGraph[timeStamp].SCCIDremap(thisSet, sccId);
            sccId++;
        }
    }
}

map<int, int> BuildIndexOfCurSccTable(SccTable &st, int &timeStamp) {
    map<int, int> index_ST;
    //first : key second : value 
    for (auto it_st = st.begin(); it_st != st.end(); it_st++) {
        if ((*it_st).sccID_Life.life_time.test(timeStamp)) {
            int curSccId = (*it_st).sccID_Life.scc_id;
            for (auto it_vset = (*it_st).nodeGroup.begin(); it_vset != (*it_st).nodeGroup.end(); it_vset++) {
                int curVerId = (*it_vset);
                index_ST.insert(pair<int, int>(curVerId, curSccId));
            }
        }
    }
    return index_ST;
}

SCCEdgeInfo BuildCurDAGEdgesDataSequenceFromIndex(Graph graph, SccTable& st, map<int, int>& index_ST, int& timeStamp) {
    SCCEdgeInfo sccEdges;
    int numOfV = graph.GetVexNum();
    AdjList_Graph graph_vertices = graph.GetVertices();
    for (int i = 0; i < numOfV; ++i) {
        int curSouNodeID = graph_vertices[i].souID;
        int sourceSccId = -1;
        auto souPos = index_ST.find(curSouNodeID);
        if (souPos != index_ST.end()) {
            sourceSccId = (*souPos).second;
        }
        else {
            //这里不会执行
            assert(false);
            sourceSccId = getSccId(curSouNodeID, timeStamp, st);
            index_ST.insert(pair<int, int>(curSouNodeID, sourceSccId));
        }
        //以此获取当前节点的出边邻居的SCC_ID
        ArcNode* p = graph_vertices[i].firstArc;
        while (p) {
            int curTarNodeID = p->tarID;
            int targetSccId = -1;

            auto tarPos = index_ST.find(curTarNodeID);
            if (tarPos != index_ST.end()) {
                targetSccId = (*tarPos).second;
            }
            else {
                assert(false);
                targetSccId = getSccId(curTarNodeID, timeStamp, st);
                index_ST.insert(pair<int, int>(curTarNodeID, targetSccId));
            }
            // 不在同一个SCC中
            if (sourceSccId != -1 && targetSccId != -1 && sourceSccId != targetSccId) {
                auto res = find_if(sccEdges.begin(), sccEdges.end(),
                                    [&](const SCCEdgeInfoItem& item){return item.sccEdge.sScc == sourceSccId && item.sccEdge.tScc == targetSccId;} 
                );
                NodeEdge e(curSouNodeID, curTarNodeID);
                if (res != sccEdges.end()) {
                    res->nodeEdges.insert(e);
                }
                else {
                    SCCEdgeInfoItem item;
                    item.sccEdge.sScc = sourceSccId;
                    item.sccEdge.tScc = targetSccId;
                    item.nodeEdges.insert(e);
                    sccEdges.insert(item);
                }
            }
            p = p->nextarc;
        }
    }
    return sccEdges;
}

#endif //IG_NOOP_5_PROCESS_SNAPSHOTS_H
