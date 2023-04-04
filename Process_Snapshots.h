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
#include "SCC_Table.h"

SccTable GetSCCTable(int timeIntervalLength, string fileAddressHead, vector<vector<int>> &evolvingGraphSequence,
                     double &buildSccTableTime);

vector<int> GetFileData(string fileAddressHead, int timeStamp);

int getSccId(int vertexId, int timestamp, SccTable sccTable);

void UpdateSccTable(int numOfSCC, map<int, int> &v2s, SccTable &st, int &timeStamp, int &sccId);

vector<int> BuildCurDAGEdgesDataSequence(Graph graph, SccTable &st, int &timeStamp);

vector<int> BuildCurDAGEdgesDataSequenceFromIndex(Graph graph, SccTable &st, map<int, int> &index_ST, int &timeStamp);

map<int, int> BuildIndexOfCurSccTable(SccTable &st, int &timeStamp);


//----------------------------
//问题：不同时间的 SCCID 是如何分配的？
SccTable GetSCCTable(int timeIntervalLength, Graph* originGraph, vector<vector<int>> &evolvingGraphSequence,
                     double &buildSccTableTime) {
    //st 变量是全局时间有效的，对于每一个时间戳都有效
    SccTable st;
    int sccId = 0;

    for (int timeStamp = 1; timeStamp < timeIntervalLength + 1; ++timeStamp) { 
        //计算图中SCC
        originGraph[timeStamp - 1].CalculateConnection();
        originGraph[timeStamp - 1].SumScc();
        int numOfSCC = originGraph[timeStamp - 1].GetConnectedCount();
        map<int, int> v2s = originGraph[timeStamp - 1].GetMapV2S();

        //更新SCC-Table
        clock_t buildSCCT_startTime, buildSCCT_endTime;
        buildSCCT_startTime = clock();
        UpdateSccTable(numOfSCC, v2s, st, timeStamp, sccId);
        buildSCCT_endTime = clock();
        double updateTime = (double) (buildSCCT_endTime - buildSCCT_startTime);
        buildSccTableTime += updateTime;

        //构建SCC-Table的索引

        int numOfV = originGraph[timeStamp - 1].GetVexNum();

        printf("Start building the index of SCC-Table------the %dth snapshot\n", timeStamp);
        map<int, int> index_curST = BuildIndexOfCurSccTable(st, timeStamp);
        //index_curST: key:节点的编号 value:该节点所在的SCC的编号
        int complete = 0;
        // 每一个节点都有对应的SCC编号
        if (index_curST.size() == numOfV) {
            complete = 1;
        }
        vector<int> cur_DAG_Edges = BuildCurDAGEdgesDataSequenceFromIndex(originGraph[timeStamp - 1], st, index_curST, timeStamp);
        //cur_DAG_Edges: SCC图中所有边的集合，两行表示一个边，第一行为边的起点，第二行为边的终点
        evolvingGraphSequence.push_back(cur_DAG_Edges);
    }

    return st;

}

int getSccId(int vertexId, int timestamp, SccTable sccTable) {
    //在SccTable中找到源节点对应的SCC_id
    for (auto record = sccTable.begin(); record != sccTable.end(); record++) {
        if ((*record).second.life_time.test(timestamp)) {
            //在该行记录的SCC部分查找是否包含该节点
            auto findVertex = (*record).first.find(vertexId);
            if (findVertex != (*record).first.end()) {
                //该行包含该节点
                int SccID = (*record).second.scc_id;
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

vector<int> GetFileData(string fileAddressHead, int timeStamp) {
    string fileAddressTail = to_string(timeStamp) + ".txt";
    string fileAddress = fileAddressHead + fileAddressTail;

    //声明变量
    vector<int> dataVector;
    int data;

    //打开文件
    ifstream fin;
    fin.open(fileAddress);
    if (fin) {
        printf("Reading the %dth file...\n", timeStamp);
        //读取数据,存储到向量dataVector中
        while (fin >> data) {
            dataVector.push_back(data);
        }
    }
    fin.close();

    return dataVector;
}

void UpdateSccTable(int numOfSCC, map<int, int> &v2s, SccTable &st, int &timeStamp, int &sccId) {

    set<int> sccSet[numOfSCC];

    printf("Analysing SCCs of the %dth snapshot ...\n", timeStamp);
    //sccset: key:SCC的编号 value:该SCC中所有节点的集合
    for (auto it = v2s.begin(); it != v2s.end(); it++) {
        int curV = (*it).first;
        int curS = (*it).second;

        sccSet[curS].insert(curV);
    }

    printf("Updating the SCC-Table...\n");
    for (int m = 0; m < numOfSCC; ++m) {
        auto findScc = st.find(sccSet[m]);
        if (findScc != st.end()) {
            //sccTable中存在该scc记录,更新该记录的生存期

            //获取当前sccId
            //int curSccId = (*findScc).second.scc_id;

            //更新该SCC生存期
            //bitset<MNS> sccLifeSpan = (*findScc).second.life_time.set(timeStamp);
            (*findScc).second.life_time.set(timeStamp);

        } else {
            //sccTable中不存在该scc记录，增加该记录，并更新id
            SccID_Life lifeId;
            lifeId.life_time.set(timeStamp);
            lifeId.scc_id = sccId;

            st.insert(pair<set<int>, SccID_Life>(sccSet[m], lifeId));
            sccId++;
        }
    }
}

vector<int>
BuildCurDAGEdgesDataSequence(Graph graph, SccTable &st, int &timeStamp) {

    //获取当前图快照的领接表以及节点数量
    AdjList_Graph graph_vertices = graph.GetVertices();
    int numOfV = graph.GetVexNum();

    //初始化存储容器
    set<SE> sse;
    vector<int> cur_DAG_Edges;
    map<int, int> curN2S_Map;
    curN2S_Map.clear();

    for (int i = 0; i < numOfV; ++i) {

        //获取当前节点SCC_ID
        printf("\t Processing the %dth (/ %d ) vertex in the %dth snapshot...\n", i, numOfV, timeStamp);

        int curSouNodeID = graph_vertices[i].souID;
        int sourceSccId = -1;

        auto souPos = curN2S_Map.find(curSouNodeID);
        if (souPos != curN2S_Map.end()) {
            sourceSccId = (*souPos).second;
        } else {
            sourceSccId = getSccId(curSouNodeID, timeStamp, st);
            curN2S_Map.insert(pair<int, int>(curSouNodeID, sourceSccId));
        }

        printf("\t The %dth (/ %d ) vertex in the %dth snapshot has been located\n", i, numOfV, timeStamp);

        //以此获取当前节点的出边邻居的SCC_ID
        ArcNode *p = graph_vertices[i].firstArc;
        int c = 0;
        while (p) {
            SE se;
            c++;

            printf("\t\t Processing the %dth (/ %d ) vertex's %dth Out-neighbor vertex in the %dth snapshot...\n", i,
                   numOfV, c, timeStamp);

            int curTarNodeID = p->tarID;
            int targetSccId = -1;

            auto tarPos = curN2S_Map.find(curTarNodeID);
            if (tarPos != curN2S_Map.end()) {
                targetSccId = (*tarPos).second;
            } else {
                targetSccId = getSccId(curTarNodeID, timeStamp, st);
                curN2S_Map.insert(pair<int, int>(curTarNodeID, targetSccId));
            }

            printf("\t\t The %dth (/ %d ) vertex's %dth Out-neighbor in the %dth snapshot has been located\n", i,
                   numOfV, c, timeStamp);

            if (sourceSccId != -1 && targetSccId != -1 && sourceSccId != targetSccId) {
                se.sScc = sourceSccId;
                se.tScc = targetSccId;

                sse.insert(se);
            }

            printf("\n");

            p = p->nextarc;
        }
        printf("----------------------------------\n");

    }

    printf("======================All vertices in the %dth snapshot has been processed======================\n",
           timeStamp);
    printf("Building edges data sequence of the %dth snapshot...\n", timeStamp);

    for (auto it = sse.begin(); it != sse.end(); it++) {
        int curS = (*it).sScc;
        int curT = (*it).tScc;

        cur_DAG_Edges.push_back(curS);
        cur_DAG_Edges.push_back(curT);
    }

    return cur_DAG_Edges;
}

map<int, int> BuildIndexOfCurSccTable(SccTable &st, int &timeStamp) {
    map<int, int> index_ST;
    //first : key second : value 
    for (auto it_st = st.begin(); it_st != st.end(); it_st++) {
        if ((*it_st).second.life_time.test(timeStamp)) {
            int curSccId = (*it_st).second.scc_id;
            for (auto it_vset = (*it_st).first.begin(); it_vset != (*it_st).first.end(); it_vset++) {
                int curVerId = (*it_vset);
                index_ST.insert(pair<int, int>(curVerId, curSccId));
            }
        }
    }
    return index_ST;
}

vector<int> BuildCurDAGEdgesDataSequenceFromIndex(Graph graph, SccTable &st, map<int, int> &index_ST, int &timeStamp) {
    vector<int> cur_DAG_Edges;
    set<SE> sse;

    int numOfV = graph.GetVexNum();
    AdjList_Graph graph_vertices = graph.GetVertices();

    for (int i = 0; i < numOfV; ++i) {
        //获取当前节点SCC_ID
        printf("\t Processing the %dth (/ %d ) vertex in the %dth snapshot...\n", i, numOfV, timeStamp);

        int curSouNodeID = graph_vertices[i].souID;
        int sourceSccId = -1;
        //map<int,int>.find : return the iterator of the key
        auto souPos = index_ST.find(curSouNodeID);

        if (souPos != index_ST.end()) {
            sourceSccId = (*souPos).second;
        } else {
            sourceSccId = getSccId(curSouNodeID, timeStamp, st);
            index_ST.insert(pair<int, int>(curSouNodeID, sourceSccId));
        }

        //printf("\t The %dth (/ %d ) vertex in the %dth snapshot has been located\n", i, numOfV, timeStamp);

        //以此获取当前节点的出边邻居的SCC_ID
        ArcNode *p = graph_vertices[i].firstArc;
        int c = 0;
        while (p) {
            SE se; //SE: SCC Edge
            c++;

            //printf("\t\t Processing the %dth (/ %d ) vertex's %dth Out-neighbor vertex in the %dth snapshot...\n", i, numOfV, c, timeStamp);

            int curTarNodeID = p->tarID;
            int targetSccId = -1;

            auto tarPos = index_ST.find(curTarNodeID);
            if (tarPos != index_ST.end()) {
                targetSccId = (*tarPos).second;
            } else {
                targetSccId = getSccId(curTarNodeID, timeStamp, st);
                index_ST.insert(pair<int, int>(curTarNodeID, targetSccId));
            }

            //printf("\t\t The %dth (/ %d ) vertex's %dth Out-neighbor in the %dth snapshot has been located\n", i, numOfV, c, timeStamp);
            // 不在同一个SCC中
            if (sourceSccId != -1 && targetSccId != -1 && sourceSccId != targetSccId) {
                se.sScc = sourceSccId;
                se.tScc = targetSccId;

                sse.insert(se);
            }

            //printf("\n");

            p = p->nextarc;
        }
        //printf("----------------------------------\n");
    }

    printf("======================All vertices in the %dth snapshot has been processed======================\n",
           timeStamp);
    printf("Building edges data sequence of the %dth snapshot...\n", timeStamp);
    //sse:存储SSC图的边
    for (auto it = sse.begin(); it != sse.end(); it++) {
        int curS = (*it).sScc;
        int curT = (*it).tScc;

        cur_DAG_Edges.push_back(curS);
        cur_DAG_Edges.push_back(curT);
    }

    return cur_DAG_Edges;
}

#endif //IG_NOOP_5_PROCESS_SNAPSHOTS_H
