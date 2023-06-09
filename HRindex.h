#ifndef IG_NOOP_5_HRINDEX_H
#define IG_NOOP_5_HRINDEX_H
#define UPDATE_TYPE_ADD_NODE 1
#define UPDATE_TYPE_ADD_EDGE 2
#define UPDATE_TYPE_DELETE_NODE 3
#define UPDATE_TYPE_DELETE_EDGE 4

#define UPDATE_ADD_TYPE_SAMESCC 1
#define UPDATE_ADD_TYPE_NOCYCLE 2
#define UPDATE_ADD_TYPE_CYCLE 3
#define UPDATE_ADD_TYPE_EDGEEXIST 4

#define UPDATE_DEL_TYPE_NOTSAMESCC 4
#define UPDATE_DEL_TYPE_KEEPEDGE 1
#define UPDATE_DEL_TYPE_NOSPLIT 2
#define UPDATE_DEL_TYPE_SPLIT 3

#include "common.h"
#include "Lifespan.h"
#include "Graph.h"
#include "SCCTable.h"
#include "Process_Snapshots.h"
#include "IndexGraph.h"
#include "Process_Query.h"
#include "SCCGraph.h"
#include "NIT.h"
#include "update.h"
using namespace std;

class HRindex
{
public:
    int timeIntervalLength;
    string graphDatafileAddHead;
    string queryFileAddress;
    string resultFileAddress;

    Graphs originGraph;
    SCCGraphs sccGraphs;
    vector<vector<int>> evolvingGraphSequence;
    SCCEdgeInfoSequence sccEdgeInfoSequence;
    SccTable sccTable;

    NodeInfoTable nodeInfoTable;
    RefineNITable refineNITable;
    IGraph IG;

    clock_t buildOriginGraphStartTime, buildOriginGraphEndTime;
    clock_t buildSccTableStartTime, buildSccTableEndTime;
    clock_t buildSCCGraphStartTime, buildSCCGraphEndTime;
    clock_t buildNITStartTime, buildNITEndTime;
    clock_t buildRefineNITStartTime, buildRefineNITEndTime;
    clock_t buildIGStartTime, buildIGEndTime;

    double buildOriginGraphTime;
    double buildSccTableTime;
    double buildSCCGraphTime;
    double buildNITTime;
    double buildRefineNITTime;
    double buildIGTime;

    HRindex(int timeIntervalLength,
        string graphDatafileAddHead,

        string queryFileAddress,
        string resultFileAddress);
    ~HRindex();
    bool buildOriginGraph();
    bool getSCCTable();
    bool buildSCCGraph();
    bool getNITable();
    bool getRefineNITable();
    bool buildIndexGraph();

    bool updateFromFile(string updateFileAddress, int method);
    bool updateFromRecords(vector<updateRecord>& updateRecordVector, int method);

    int getaNewSCCID();
    bool addSCCnode(int nodeID, int newSCCID, Lifespan lifespan);
    bool addSCCedge(int srcNodeID, int dstNodeID, Lifespan lifespan);
    bool reconstructEvolvingGraphSequence(int timestamp);
    void printStatistics();

    void storeOriginGraph(string dir);
    void storeSCCGraph(string dir);
    void storeIG(string dir);

    void sortAll();

private:
    bool singleStepUpdate(updateRecord& ur, int method);
    bool singleStepUpdateAddNode(int u, int timestamp);
    bool singleStepUpdateAddEdge1(int u, int v, int timestamp);
    bool singleStepUpdateAddEdge2(int u, int v, int timestamp);
    bool singleStepUpdateAddEdge3(int u, int v, int timestamp);
    bool singleStepUpdateDeleteNode(int u, int timestamp);
    bool singleStepUpdateDeleteEdge(int u, int v, int timestamp);
};

HRindex::HRindex(
    int timeIntervalLength,
    string graphDatafileAddHead,
    string queryFileAddress,
    string resultFileAddress) {
    this->timeIntervalLength = timeIntervalLength;
    this->graphDatafileAddHead = graphDatafileAddHead;
    this->queryFileAddress = queryFileAddress;
    this->resultFileAddress = resultFileAddress;
}

HRindex::~HRindex() {}

bool HRindex::buildOriginGraph() {
    buildOriginGraphStartTime = clock();
    for (int timeStamp = 0; timeStamp < timeIntervalLength; ++timeStamp) {
        Graph g;
        g.timestamp = timeStamp;
        string fileAddressTail = to_string(timeStamp) + ".txt";
        string fileAddress = graphDatafileAddHead + fileAddressTail;
        ifstream fin;
        fin.open(fileAddress);
        int src, dst, ts;
        if (fin) {
            while (fin >> src >> dst >> ts) {
                g.InsertEdge(src, dst);
            }
        }
        else {
            throw "buildOriginGraph: file opne error";
        }
        fin.close();
        originGraph.push_back(g);
    }
    /*
    vector<int> vexnums;
    vector<int> edgenums;
    for (int i = 0; i < timeIntervalLength; i++) {
        LOG << "G" << i << ": vertex " << originGraph[i].GetVexNum() << " edge = " << originGraph[i].GetEdgeNum() << endl;
        vexnums.push_back(originGraph[i].GetVexNum());
        edgenums.push_back(originGraph[i].GetEdgeNum());
    }
    int vexmean = std::accumulate(vexnums.begin(), vexnums.end(), 0) / vexnums.size();
    int edgemean = std::accumulate(edgenums.begin(), edgenums.end(), 0) / edgenums.size();
    LOG << "G: vertex mean " << vexmean << " edge mean = " << edgemean << endl;
    */
    buildOriginGraphEndTime = clock();
    buildOriginGraphTime = (double)(buildOriginGraphEndTime - buildOriginGraphStartTime) / CLOCKS_PER_SEC;
    LOG << "step1:buildOriginGraph(" << std::fixed << std::setprecision(2) << buildOriginGraphTime << "s)" << endl;

    return true;
};

bool HRindex::getSCCTable()
{
    buildSccTableStartTime = clock();
    this->sccTable = GetSCCTable(timeIntervalLength, originGraph, evolvingGraphSequence, sccEdgeInfoSequence, buildSccTableTime);
    buildSccTableEndTime = clock();
    buildSccTableTime = (double)(buildSccTableEndTime - buildSccTableStartTime) / CLOCKS_PER_SEC;
    LOG << "step2:buildSccTable(" << std::fixed << std::setprecision(2) << buildSccTableTime << "s)" << endl;
    return true;
}

bool HRindex::buildSCCGraph()
{
    buildSCCGraphStartTime = clock();
    sccGraphs = BuildSCCGraphs(evolvingGraphSequence, sccTable, sccEdgeInfoSequence, timeIntervalLength);
    buildSCCGraphEndTime = clock();
    buildSCCGraphTime = (double)(buildSCCGraphEndTime - buildSCCGraphStartTime) / CLOCKS_PER_SEC;
    LOG << "step3:buildSCCGraph(" << std::fixed << std::setprecision(2) << buildSCCGraphTime << "s)" << endl;
    return true;
}


bool HRindex::getNITable() {
    buildNITStartTime = clock();
    nodeInfoTable.clear();
    int timestamp;
    for (int i = 0; i <= timeIntervalLength; ++i) {
        timestamp = i;
        nodeInfoTable = GetNITable(nodeInfoTable, evolvingGraphSequence, timestamp);
    }
    buildNITEndTime = clock();
    buildNITTime = (double)(buildNITEndTime - buildNITStartTime) / CLOCKS_PER_SEC;
    LOG << "step4:buildNITable(" << std::fixed << std::setprecision(2) << buildNITTime << "s)" << endl;
    return true;
}

bool HRindex::getRefineNITable() {
    buildRefineNITStartTime = clock();
    refineNITable.clear();
    refineNITable = GetRefineNITable(nodeInfoTable);
    buildRefineNITEndTime = clock();
    buildRefineNITTime = (double)(buildRefineNITEndTime - buildRefineNITStartTime) / CLOCKS_PER_SEC;
    LOG << "step5:buildRefineNITable(" << std::fixed << std::setprecision(2) << buildRefineNITTime << "s)" << endl;
    return true;
}

bool HRindex::buildIndexGraph() {
    buildIGStartTime = clock();
    IG = BuildIndexGraph(refineNITable);
    buildIGEndTime = clock();
    buildIGTime = (double)(buildIGEndTime - buildIGStartTime) / CLOCKS_PER_SEC;
    LOG << "step6:buildIndexGraph(" << std::fixed << std::setprecision(2) << buildIGTime << "s)" << endl;
    return true;
}


bool HRindex::updateFromFile(string updateFileAddress, int method) {
    vector<updateRecord> updateRecordVector;
    readUpdateRecords(updateRecordVector, updateFileAddress);
    int totalRecordNum = updateRecordVector.size();
    int finishRecordNum = 0;

    for (int i = 0; i < updateRecordVector.size(); ++i) {
        LOG << "update " << i << "/" << totalRecordNum << endl;
        if (singleStepUpdate(updateRecordVector[i], method)) {
            finishRecordNum++;
        }
    }
    LOG << finishRecordNum << "/" << totalRecordNum << endl;
    LOG << endl;
    return true;
}

bool HRindex::updateFromRecords(vector<updateRecord>& updateRecordVector, int method) {
    int totalRecordNum = updateRecordVector.size();
    int finishRecordNum = 0;
    for (int i = 0; i < updateRecordVector.size(); ++i) {
        LOG << "update " << i << "/" << totalRecordNum << endl;
        if (singleStepUpdate(updateRecordVector[i], method)) {
            finishRecordNum++;
        }
    }
    LOG << finishRecordNum << "/" << totalRecordNum << endl;
    LOG << endl;
    return true;
}

int HRindex::getaNewSCCID() {
    int maxSccid = sccTable.begin()->sccID_Life.scc_id;
    // trasverse the sccTable to find the new sccid
    for (auto it = sccTable.begin(); it != sccTable.end(); ++it) {
        if (it->sccID_Life.scc_id > maxSccid) {
            maxSccid = it->sccID_Life.scc_id;
        }
    }
    return maxSccid + 1;
}

bool HRindex::addSCCnode(int nodeID, int newSCCID, Lifespan lifespan) {
    SccID_Life newSccID_Life;
    newSccID_Life.scc_id = newSCCID;
    newSccID_Life.life_time = lifespan;
    SCCTableItem item;
    item.nodeGroup = set<int>{ nodeID };
    item.sccID_Life = newSccID_Life;
    sccTable.insert(item);
    return true;
}

bool HRindex::reconstructEvolvingGraphSequence(int timestamp) {
    vector<int> tmp;
    for (auto it = sccGraphs.begin(); it != sccGraphs.end(); ++it)
    {
        if (it->timestamp == timestamp) {
            for (auto it2 = it->vertices.begin(); it2 != it->vertices.end(); ++it2)
            {
                int id = it2->SCCID;
                SCCarc* a = it2->firstArc;
                while (a != NULL)
                {
                    tmp.push_back(id);
                    tmp.push_back(a->dstID);
                    a = a->next;
                }
            }
            break;
        }
    }
    evolvingGraphSequence[timestamp] = tmp;
    return true;
}

bool HRindex::singleStepUpdate(updateRecord& ur, int method) {
    switch (ur.type)
    {
    case(UPDATE_TYPE_ADD_NODE): {
        return singleStepUpdateAddNode(ur.u, ur.timestamp);
        break;
    }
    case(UPDATE_TYPE_ADD_EDGE): {
        if (method == 1)
            return singleStepUpdateAddEdge1(ur.u, ur.v, ur.timestamp);
        else
            return singleStepUpdateAddEdge3(ur.u, ur.v, ur.timestamp);
        break;
    }
    case(UPDATE_TYPE_DELETE_EDGE): {
        return singleStepUpdateDeleteEdge(ur.u, ur.v, ur.timestamp);
        break;
    }
    case(UPDATE_TYPE_DELETE_NODE): {
        return singleStepUpdateDeleteNode(ur.u, ur.timestamp);
        break;
    }
    default: {
        printf("Error: update type error!\n");
        return false;
    }
    }
    return false;
}

bool HRindex::singleStepUpdateAddNode(int u, int timestamp) {
    //添加一个节点u，这个节点没有与任何其他边相连，因此是孤立的SCC
    /*需要更改的内容
        1. originGraph:新增一个节点 id = u, sccid = newSCCID, firstArc = NULL
        2. SCCTable: 增加一个项目：nodeGroup = {u}, sccID_Life = {newSCCID, 只有一个时间段timeStamp}
        3. SCCGraph：增加一个node：nodeGroup = {u}, SCCID = newSCCID, firstArc = NULL
        4. evolovingGraphSequence: 不改变
        5. sccEdgeInfoSequence：不改变
        6. NIT：增加一个表项：node = newSCCID， in = NULL, out = NULL
        7. RefineNIT：增加一个表项：node = newSCCID， out = NULL
        8. IG：不改变
    */
    Lifespan newLifespan;
    int newSCCID = getaNewSCCID();
    originGraph[timestamp].AddNode(u, newSCCID);
    addSCCnode(u, newSCCID, LifespanBuild(newLifespan, timestamp, timestamp));
    sccGraphs[timestamp].addNode(newSCCID, { u });
    //加入一个新的ONTable的表项
    RecordItem newNITableRecordItem;
    newNITableRecordItem.node = newSCCID;
    nodeInfoTable.push_back(newNITableRecordItem);
    //加入一个新的RefineNITable的表项
    RefineRecordItem newRefineRecordItem;
    newRefineRecordItem.node = newSCCID;
    refineNITable.push_back(newRefineRecordItem);
    return true;
}

/*
    更新描述: 添加一条边(u,v)

    需要更改的内容:
    1. originGraph:新增一条边 u->v
    2. SCCTable: 不改变
    3. SCCGraph：如果SCCID不同，需要改变
    4. evolovingGraphSequence: 根据SCCGraph重建
    5. sccEdgeInfoSequence：TODO
    6. NIT：重建TimeStamp时刻
    7. RefineNIT：重建
    8. IG：重建

    失败情况：无
*/
bool HRindex::singleStepUpdateAddEdge1(int u, int v, int timestamp) {
    clock_t start, end;
    double duration;
    clock_t addNodeStart, addNodeEnd;
    double addNodeDuration;
    start = clock();
    LOG << "singleStepUpdateAddEdge: " << u << "->" << v << " " << timestamp << endl;
    //添加一条边(u,v),先检测是否存在
    if (!originGraph[timestamp].NodeIsExists(u)) {
        addNodeStart = clock();
        singleStepUpdateAddNode(u, timestamp);
        addNodeEnd = clock();
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC) * 1000;
        LOG << "node : " << u << " not exists(" << addNodeDuration << "ms)" << endl;
    }
    else {
        LOG << "node : " << u << " exists" << endl;
    }
    if (!originGraph[timestamp].NodeIsExists(v)) {
        addNodeStart = clock();
        singleStepUpdateAddNode(v, timestamp);
        addNodeEnd = clock();
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC) * 1000;
        LOG << "node : " << v << " not exists(" << addNodeDuration << "ms)" << endl;
    }
    else {
        LOG << "node : " << v << " exists" << endl;
    }

    originGraph[timestamp].InsertEdgeWithCheck(u, v);
    LOG << "step1 : originGraph insert edge" << endl;
    auto [d1, uSCCID] = measureTime<int>(std::bind(&Graph::findSCCIDFromNodeId, &originGraph[timestamp], u));
    LOG << "step2.1 : findSCCIDFromNodeId(" << u <<  "at" << uSCCID << ")(" << d1 << "ms)" << endl;
    auto [d2, vSCCID] = measureTime<int>(std::bind(&Graph::findSCCIDFromNodeId, &originGraph[timestamp], v));
    LOG << "step2.2 : findSCCIDFromNodeId(" << v <<  "at" << vSCCID << ")(" << d2 << "ms)" << endl;
    //int uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    //int vSCCID = originGraph[timestamp].findSCCIDFromNodeId(v);
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        //循环检测环，直到没有环为止
        sccGraphs[timestamp].insertEdgeNotExist(uSCCID, vSCCID, { NodeEdge(u,v) });
        clock_t findCycleStart, findCycleEnd;
        double findCycleDuration;
        findCycleStart = clock();
        vector<SCCnode> cycle = sccGraphs[timestamp].findCycle(uSCCID);
        while (cycle.size() != 0) {
            //考虑合并后的SCC与其他SCC相同
            int reusedID = sccGraphs[timestamp].merge(cycle, sccTable);
            cycle = sccGraphs[timestamp].findCycle(reusedID);
        }
        findCycleEnd = clock();
        findCycleDuration = ((double)(findCycleEnd - findCycleStart) / CLOCKS_PER_SEC) * 1000;
        LOG << "step2 : findCycle(" << findCycleDuration << "ms)" << endl;

        auto [d3, notuse] = measureTime<bool>(std::bind(&HRindex::reconstructEvolvingGraphSequence, this, timestamp));
        //reconstructEvolvingGraphSequence(sccGraph, timestamp);
        LOG << "step3 : reconstructEvolvingGraphSequence(" << d3 << "ms)" << endl;

        clock_t reconstructNITStart, reconstructNITEnd;
        double reconstructNITDuration;
        reconstructNITStart = clock();
        int id;
        auto exist = [&id](RecordItem& ri) {return ri.node == id;};
        //整个删除在timeStamp时刻的NIT中的内容，然后重建
        deleteNIT(nodeInfoTable, timestamp);
        for (auto sccGraphit = sccGraphs[timestamp].vertices.begin();
            sccGraphit != sccGraphs[timestamp].vertices.end(); ++sccGraphit) {
            id = sccGraphit->SCCID;
            vector<int> INlist;
            vector<int> OUTlist;
            //遍历整个图来找到这个SCCID的入边
            for (auto INlistit : sccGraphs[timestamp].vertices) {
                if (INlistit.SCCID == id) continue;
                else {
                    for (auto arcit = INlistit.firstArc; arcit != NULL; arcit = arcit->next)
                    {
                        if (arcit->dstID == id) {
                            INlist.push_back(INlistit.SCCID);
                            break;
                        }
                    }
                }
            }
            //找到这个SCCID的出边
            for (auto outArcit = sccGraphit->firstArc; outArcit != NULL; outArcit = outArcit->next) {
                OUTlist.push_back(outArcit->dstID);
            }
            auto record = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), exist);
            if (record != nodeInfoTable.end()) {
                for (auto inID = INlist.begin(); inID != INlist.end(); ++inID) {
                    insertNITin(*record, *inID, timestamp);
                }
                for (auto outID = OUTlist.begin(); outID != OUTlist.end(); ++outID) {
                    insertNITout(*record, *outID, timestamp);
                }
            }
            else {
                RecordItem newNITableRecordItem;
                newNITableRecordItem.node = id;
                for (auto inID = INlist.begin(); inID != INlist.end(); ++inID) {
                    insertNITin(newNITableRecordItem, *inID, timestamp);
                }
                for (auto outID = OUTlist.begin(); outID != OUTlist.end(); ++outID) {
                    insertNITout(newNITableRecordItem, *outID, timestamp);
                }
                nodeInfoTable.push_back(newNITableRecordItem);
            }
        }
        reconstructNITEnd = clock();
        reconstructNITDuration = ((double)(reconstructNITEnd - reconstructNITStart) / CLOCKS_PER_SEC) * 1000;
        LOG << "step4 : reconstructNIT(" << reconstructNITDuration << "ms)" << endl;

        //更新RefineNITable
        auto [d5, newRefineNITable] = measureTime<RefineNITable>(GetRefineNITable, this->nodeInfoTable);
        this->refineNITable = newRefineNITable;
        LOG << "step5 : Reconstruct RefineNITable(" << d5 << "ms)" << endl;

        //更新IG
        auto [d6, newIG] = measureTime<IGraph>(BuildIndexGraph, refineNITable);
        this->IG = newIG;
        LOG << "step6 : Rebuild IndexGraph(" << d6 << "ms)" << endl;
    }
    end = clock();
    duration = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;
    LOG << "finish(" << duration << "ms)" << endl;
    LOG << endl;
    return true;
}


Lifespan _getLpSi(const RefineRecordItem& rri) {
    Lifespan res;
    for (auto it : rri.Out) {
        if (it.partLab == 2)
            res = LifespanUnion(res, it.lifespan);
    }
    return res;
}
Lifespan _getUnionIN(const RecordItem& ri) {
    Lifespan res;
    for (auto it : ri.In) {
        res = LifespanUnion(res, it.lifespan);
    }
    return res;
}
/*
    重要定理：
    假设新加入了一个节点使得 [S1, S2, ... , Sn] -> Snew
    对于不在SCC的中的节点，Sk
    有3种情况
    1. Sk的出入边都不在cycle中，不需要做任何改变
    2. Sk的In交cycle不空，那么，只需要把NIT中的in更新，也不需要做任何改变
    3. Sk的Out交cycle不空，需要先把原来的删掉，在加入新的
    证明：Sk的In交cycle不空 与 Sk的Out交cycle不空 最多存在一种
    反证法
        假设Sk的In交cycle为S1， Sk的Out交cycle为S2
        那么在更新后的SCC中就存在Snew -> Sk, Sk -> Snew
        不是DAG
*/
bool HRindex::singleStepUpdateAddEdge3(int u, int v, int timestamp) {
    clock_t start, end;
    double duration;
    clock_t addNodeStart, addNodeEnd;
    double addNodeDuration;
    start = clock();
    int type = -1;
    LOG << "singleStepUpdateAddEdge: " << u << "->" << v << " " << timestamp << endl;
    //添加一条边(u,v),先检测是否存在
    if (!originGraph[timestamp].NodeIsExists(u)) {
        addNodeStart = clock();
        singleStepUpdateAddNode(u, timestamp);
        addNodeEnd = clock();
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC) * 1000;
        LOG << "node : " << u << " not exists(" << addNodeDuration << "ms)" << endl;
    }
    else {
        LOG << "node : " << u << " exists" << endl;
    }
    if (!originGraph[timestamp].NodeIsExists(v)) {
        addNodeStart = clock();
        singleStepUpdateAddNode(v, timestamp);
        addNodeEnd = clock();
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC) * 1000;
        LOG << "node : " << v << " not exists(" << addNodeDuration << "ms)" << endl;
    }
    else {
        LOG << "node : " << v << " exists" << endl;
    }
    //这里不该这样子找，因为originGraph的SCC好像没有更新
    originGraph[timestamp].InsertEdgeWithCheck(u, v);
    LOG << "step1 : originGraph insert edge" << endl;
    auto [d1, uSCCID] = measureTime<int>(std::bind(&SCCGraph::findSCCIDNodeFromOriginNodeID, &sccGraphs[timestamp], u));
    LOG << "step2.1 : findSCCIDFromNodeId(" << u <<  "at" << uSCCID << ")(" << d1 << "ms)" << endl;
    auto [d2, vSCCID] = measureTime<int>(std::bind(&SCCGraph::findSCCIDNodeFromOriginNodeID, &sccGraphs[timestamp], v));
    LOG << "step2.2 : findSCCIDFromNodeId(" << v << "at" << vSCCID << ")(" << d2 << "ms)" << endl;
    //所有节点都必须找到
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        SCCGraph& thisSCCGraph = sccGraphs[timestamp];
        //考虑(S_u,S_v)已经存在的情况
        //sccGraphs[timestamp].insertEdgeNotExist(uSCCID, vSCCID, { NodeEdge(u,v) });
        //已经存在
        if (sccGraphs[timestamp].insertEdgeNodeMustExist(uSCCID, vSCCID, { NodeEdge(u,v) }) == 2) {
            type = UPDATE_ADD_TYPE_EDGEEXIST;
            end = clock();
            goto FINISH;
        }
        //循环检测环，直到没有环为止
        clock_t findCycleStart, findCycleEnd;
        double findCycleDuration;
        findCycleStart = clock();
        Lifespan tx = LifespanBuild(timestamp, timestamp);

        //vector<SCCnode> cycle = thisSCCGraph.findCycle(uSCCID);
        //int newSCCID = thisSCCGraph.merge(cycle, sccTable);
        pair<int, vector<SCCnode>> res = thisSCCGraph.findCycles(uSCCID, sccTable);
        int newSCCID = res.first;
        vector<SCCnode> cycle = res.second;

        //无环时
        if (newSCCID == -1) {
            //处理Su的出边，Sv的入边
            int su = uSCCID;
            int sv = vSCCID;
            auto ru = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
                [&](const RecordItem& it) {return it.node == su;});
            assert(ru != nodeInfoTable.end());
            auto rru = findRefineRecordItem(refineNITable, su);
            auto rv = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
                [&](const RecordItem& it) {return it.node == sv;});
            assert(rv != nodeInfoTable.end());
            auto rrv = findRefineRecordItem(refineNITable, sv);
            //先处理Su的出边
            Lifespan LsubSu;
            for (const auto it : ru->In) {
                LsubSu = LsubSu | it.lifespan;
            }
            if (LsubSu.test(timestamp)) {
                //送往N1
                rru.Out.push_back(Item(sv, tx, 1));
                IG.InsertEdgeOrCreate(su, tx, sv, tx);
            }
            else {
                //送往N2,先判断LPSI
                Lifespan Lpsu = _getLpSi(rru);
                auto exist = find_if(rru.Out.begin(), rru.Out.end(),
                    [&](const Item& tmp) {return tmp.vertexID == sv && tmp.partLab == 2;});
                if (!Lpsu.test(timestamp)) {
                    Lifespan newLpsu = Lpsu | tx;
                    //考虑之前不存在此节点
                    if (Lpsu.none()) {
                        IG.CreateVertex(su, newLpsu);
                    }
                    else {
                        IG.ModifyNodeOrThrow(su, Lpsu, newLpsu);
                    }
                    if (exist != rru.Out.end()) {
                        IG.DeleteEdgeKeepEmptyNode(su, newLpsu, sv, exist->lifespan);
                        assert(exist->lifespan.test(timestamp) == false);
                        exist->lifespan.set(timestamp, true);
                        IG.InsertEdgeSrcMustExistOrThrow(su, newLpsu, sv, exist->lifespan);
                    }
                    else {
                        //直接加入
                        rru.Out.push_back(Item(sv, tx, 2));
                        IG.InsertEdgeSrcMustExistOrThrow(su, newLpsu, sv, tx);
                    }
                }
                else {
                    //已经存在t时刻的出边,此时Lpsu必定存在
                    if (exist != rru.Out.end()) {
                        IG.DeleteEdgeKeepEmptyNode(su, Lpsu, sv, exist->lifespan);
                        assert(exist->lifespan.test(timestamp) == false);
                        exist->lifespan.set(timestamp, true);
                        IG.InsertEdgeSrcMustExistOrThrow(su, Lpsu, sv, exist->lifespan);
                    }
                    else {
                        //直接加入
                        rru.Out.push_back(Item(sv, tx, 2));
                        IG.InsertEdgeSrcMustExistOrThrow(su, Lpsu, sv, tx);
                    }

                }
            }
            //处理Sv的入边
            Lifespan LsubSv;
            for (const auto it : rv->In) {
                LsubSv = LsubSv | it.lifespan;
            }
            //sv本来就有in边,直接加入即可
            if (LsubSv.test(timestamp)) {
                auto exist = find_if(rv->In.begin(), rv->In.end(),
                    [&](const Item& tmp) {return tmp.vertexID == su;});
                if (exist != rv->In.end()) {
                    exist->lifespan.set(timestamp, true);
                }
                else {
                    rv->In.push_back(Item(su, tx, 1));
                }
            }
            else {
                //本来没有In边，需要将N1向N2转移
                rv->In.push_back(Item(su, tx));
                auto n1 = rrv.Out.begin();
                while (n1 != rrv.Out.end()) {
                    if (n1->lifespan.test(timestamp) && n1->partLab == 1) {
                        IG.DeleteEdgeKeepEmptyNode(su, tx, n1->vertexID, tx);
                        n1 = rrv.Out.erase(n1);
                        //向N2转移(n1.vertexId, tx)，先判断是否存在
                        Lifespan Lpsv = _getLpSi(rrv);
                        auto findInN2 = find_if(rrv.Out.begin(), rrv.Out.end(),
                            [&](const auto& tmp) {return tmp.vertexID == n1->vertexID && tmp.partLab == 2;});
                        if (!Lpsv.test(timestamp)) {
                            //如果lpsv不包含tx，需要先将lpsv更新
                            Lifespan newLpsv = Lpsv | tx;
                            if (Lpsv.none()) {
                                IG.CreateVertex(su, newLpsv);
                            }
                            else {
                                IG.ModifyNodeOrThrow(su, Lpsv, newLpsv);
                            }
                            if (findInN2 != rrv.Out.end()) {
                                IG.DeleteEdgeKeepEmptyNode(sv, newLpsv , findInN2->vertexID, findInN2->lifespan);
                                assert(findInN2->lifespan.test(timestamp) == false);
                                findInN2->lifespan.set(timestamp, true);
                                IG.InsertEdgeSrcMustExistOrThrow(sv, newLpsv, sv, findInN2->lifespan);
                            }
                            else {
                                //直接加入
                                rru.Out.push_back(Item(n1->vertexID, tx, 2));
                                IG.InsertEdgeSrcMustExistOrThrow(su, newLpsv, sv, tx);
                            }
                        }
                        else {
                            if (findInN2 != rrv.Out.end()) {
                                IG.DeleteEdgeKeepEmptyNode(sv, Lpsv , findInN2->vertexID, findInN2->lifespan);
                                assert(findInN2->lifespan.test(timestamp) == false);
                                findInN2->lifespan.set(timestamp, true);
                                IG.InsertEdgeSrcMustExistOrThrow(sv, Lpsv, sv, findInN2->lifespan);
                            }
                            else {
                                //直接加入
                                rru.Out.push_back(Item(n1->vertexID, tx, 2));
                                IG.InsertEdgeSrcMustExistOrThrow(su, Lpsv, sv, tx);
                            }
                        }
                    }
                    else {
                        n1++;
                    }
                }
            }
            type = UPDATE_ADD_TYPE_NOCYCLE;
            end = clock();
            goto FINISH;
        }
        reconstructEvolvingGraphSequence(timestamp);
        //加入新的SCC节点, 使用SCCGraph来获取出入边信息
        auto newSCCFindRes = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
            [&](RecordItem& item) {return item.node == newSCCID;});
        vector<int> SCCNewNodein;
        vector<int> SCCNewNodeout;
        auto inoutPair = sccGraphs[timestamp].getInAndOutNodes(newSCCID);
        SCCNewNodein = inoutPair.first;
        SCCNewNodeout = inoutPair.second;
        LOG << "new SCC node In size =  " << SCCNewNodein.size() << endl;
        LOG << "new SCC node Out size =  " << SCCNewNodeout.size() << endl;
        //sort(nodeInfoTable.begin(), nodeInfoTable.end(), compareRecordItem);
        //sort(refineNITable.begin(), refineNITable.end(), compareRefineRecordItem);
        //先处理新增的SCCNode
        //合并后的SCC在其他的时刻有出现
        clock_t addNewSCCNodeStart = clock();
        if (newSCCFindRes != nodeInfoTable.end()) {
            for (auto init : SCCNewNodein) {
                //增加In边
                auto findres = find_if(newSCCFindRes->In.begin(), newSCCFindRes->In.end(),
                    [&](Item& item) {return item.vertexID == init;});
                if (findres != newSCCFindRes->In.end()) {
                    assert(findres->lifespan.test(timestamp) == false);
                    findres->lifespan.set(timestamp, true);
                }
                else {
                    Item newItem;
                    newItem.vertexID = init;
                    newItem.lifespan = tx;
                    newSCCFindRes->In.push_back(newItem);
                }
            }
            assert(SCCNewNodeout.size() == 0);
            for (auto outit : SCCNewNodeout) {
                //增加Out边
                auto findres = find_if(newSCCFindRes->Out.begin(), newSCCFindRes->Out.end(),
                    [&](Item& item) {return item.vertexID == outit;});
                if (findres != newSCCFindRes->Out.end()) {
                    assert(findres->lifespan.test(timestamp) == false);
                    findres->lifespan.set(timestamp, true);
                }
                else {
                    Item newItem;
                    newItem.vertexID = outit;
                    newItem.lifespan = tx;
                    newSCCFindRes->In.push_back(newItem);
                }
            }
        }
        else {
            RecordItem recordItem;
            recordItem.node = newSCCID;
            for (auto init : SCCNewNodein) {
                Item newItem;
                newItem.vertexID = init;
                newItem.lifespan = tx;
                recordItem.In.push_back(newItem);
            }
            for (auto outit : SCCNewNodeout) {
                Item newItem;
                newItem.vertexID = outit;
                newItem.lifespan = tx;
                recordItem.Out.push_back(newItem);
            }
            nodeInfoTable.push_back(recordItem);
            RefineRecordItem ritem = getRefineRecordItem(recordItem);
            refineNITable.push_back(ritem);
            IG.updateAddRefineRecord(ritem);
        }
        clock_t addNewSCCNodeEnd = clock();
        LOG << "process new Node (" << ((double)(addNewSCCNodeEnd - addNewSCCNodeStart) / CLOCKS_PER_SEC) * 1000 << "ms)" << endl;
        
        clock_t traverseStart = clock();
        for (auto& item : nodeInfoTable) {
            RefineRecordItem& ritem = findRefineRecordItem(refineNITable, item.node);
            int Si = item.node;
            
            if (!sccIncycle(Si, cycle)) {
                /*
                    需要考虑新增的NewSCCID如何加入，一共有3种情况，最多只能是其中的一种
                    1. 新增的在IN， 不会导有入边的时间的变化，Out中只需要删除cycle中的节点
                    2. 新增的在OUT，导致LpSi的变化
                    3. 都不在
                */
                //添加新的节点
                auto findin = find(SCCNewNodein.begin(), SCCNewNodein.end(), Si);
                auto findout = find(SCCNewNodeout.begin(), SCCNewNodeout.end(), Si);
                assert(!(findin != SCCNewNodein.end() && findout != SCCNewNodeout.end()));
                if (findin == SCCNewNodein.end() && findout != SCCNewNodeout.end()) {
                    //新增的在in，先删除cycle中的节点，再加入新节点
                    //LOG << "porcessing node " << item.node << " In table "<< endl;
                    auto it = item.In.begin();
                    while (it != item.In.end()) {
                        if (sccIncycle(it->vertexID, cycle) && it->lifespan.test(timestamp)) {
                            it->lifespan.set(timestamp, false);
                            if (it->lifespan.none()) it = item.In.erase(it);
                            else it++;
                        }
                        else it++;
                    }
                    //考虑此节点已经存在
                    insertNITin(item, newSCCID, timestamp);
                }
                if (findin != SCCNewNodein.end() && findout == SCCNewNodeout.end()) {
                    //LOG << "porcessing node " << item.node << " Out table"<< endl;
                    //新增的在out, Lpsi是不会发生变化的
                    Lifespan Lpsi = _getLpSi(ritem);
                    Lifespan unionIn = _getUnionIN(item);
                    //加入新节点，考虑节点已经存在的情况
                    bool exist = insertNITout(item, newSCCID, timestamp);
                    if (unionIn.test(timestamp)) {
                        //加入到N1，即使N1中已经存在此节点，仍然直接加入
                        ritem.Out.push_back(Item(newSCCID, tx, 1));
                        IG.InsertEdgeSrcMustExistOrThrow(Si, tx, newSCCID, tx);
                    }
                    else {
                        //加入到N2
                        auto findInN2 = find_if(ritem.Out.begin(), ritem.Out.end(),
                            [&](const Item& elm) {return elm.vertexID == newSCCID && elm.partLab == 2;});
                        //在N2中已经存在
                        if (findInN2 != ritem.Out.end()) {
                            IG.DeleteEdgeKeepEmptyNode(Si, Lpsi, findInN2->vertexID, findInN2->lifespan);
                            findInN2->lifespan.set(timestamp, true);
                            IG.InsertEdgeSrcMustExistOrThrow(Si, Lpsi, findInN2->vertexID, findInN2->lifespan);
                        }
                        //在N2中不存在，直接加入
                        else {
                            ritem.Out.push_back(Item(newSCCID, tx, 2));
                            //这里可能出现Lpsi为空的情况
                            IG.InsertEdgeOrCreate(Si, Lpsi, newSCCID, tx);
                            //IG.InsertEdgeSrcMustExistOrThrow(Si, Lpsi, newSCCID, tx);
                        }
                    }

                    //删除out中cycle的节点
                    auto it = item.Out.begin();
                    while (it != item.Out.end()) {
                        if (sccIncycle(it->vertexID, cycle) && it->lifespan.test(timestamp)) {
                            it->lifespan.set(timestamp, false);
                            if (it->lifespan.none()) it = item.Out.erase(it);
                            else it++;
                        }
                        else it++;
                    }
                    //删除ritem的节点，同时处理IG
                    auto rit = ritem.Out.begin();
                    while (rit != ritem.Out.end()) {
                        if (sccIncycle(rit->vertexID, cycle) && rit->lifespan.test(timestamp)) {
                            if (rit->partLab == 1) {
                                IG.DeleteEdgeKeepEmptyNode(Si, tx, rit->vertexID, tx);
                                rit = ritem.Out.erase(rit);
                            }
                            else {
                                if (rit->lifespan.count() == 1) {
                                    assert(rit->lifespan == tx);
                                    IG.DeleteEdgeKeepEmptyNode(Si, Lpsi, rit->vertexID, rit->lifespan);
                                    rit = ritem.Out.erase(rit);
                                }
                                else {
                                    IG.DeleteEdgeKeepEmptyNode(Si, Lpsi, rit->vertexID, rit->lifespan);
                                    rit->lifespan.set(timestamp, false);
                                    IG.InsertEdgeSrcMustExistOrThrow(Si, Lpsi, rit->vertexID, rit->lifespan);
                                    rit++;
                                }
                            }
                        }
                        else rit++;
                    }
                }
            }
            else {
                //LOG << "processing node " << item.node << " in cycle " << endl;
                //node in cycleList
                deleteNITItemIN(item, timestamp);
                if (item.Out.size() == 0) continue;
                deleteNITItemOut(item, timestamp);
                //in和out在tx时刻都不存在
                auto it = ritem.Out.begin();
                Lifespan Lpsi = _getLpSi(ritem);
                while (it != ritem.Out.end())
                {
                    if (!it->lifespan.test(timestamp)) {
                        it++;
                    }
                    else {
                        if (it->partLab == 1) {
                            //<Si, tx> -> <Sj, tx>
                            IG.DeleteEdgeKeepEmptyNode(Si, tx, it->vertexID, tx);
                            it = ritem.Out.erase(it);
                        }
                        else {
                            IG.DeleteEdgeKeepEmptyNode(Si, Lpsi, it->vertexID, it->lifespan);
                            //<Si, Lpsi> -> <Sj, tx>
                            if (it->lifespan.count() == 1) {
                                it = ritem.Out.erase(it);
                            }
                            //<Si, Lpsi> -> <Sj, Lsj>
                            else {
                                Lifespan newlife = LifespanTestAndUnset(it->lifespan, timestamp);
                                IG.InsertEdgeSrcMustExistOrThrow(Si, Lpsi, it->vertexID, newlife);
                                it++;
                            }
                        }
                    }

                }
            }
        }
        clock_t traverseEnd = clock();
        LOG << "traverse NIT(" << ((double)(traverseEnd - traverseStart) / CLOCKS_PER_SEC) *1000 << ")"<< endl;
        auto nit = nodeInfoTable.begin();
        while (nit != nodeInfoTable.end())
        {
            if (nit->In.size() == 0 && nit->Out.size() == 0) {
                auto rnit = refineNITable.begin();
                while (rnit != refineNITable.end()) {
                    if (rnit->node == nit->node) {
                        assert(rnit->Out.size() == 0);
                        refineNITable.erase(rnit);
                        break;
                    }
                    else {
                        rnit++;
                    }
                }
                nit = nodeInfoTable.erase(nit);
            }
            else nit++;
        }
        //删除所有在cycle中且含有tx的节点
        //IG.deleteNodeWhichInCycleAndIncludeTimestamp(cycle, timestamp);
        double timeDel1 = measureTime([&](){ IG.deleteNodeWhichInCycleAndIncludeTimestamp(cycle, timestamp); });
        LOG << "deleteNodeWhichInCycleAndIncludeTimestamp(" << timeDel1 << ")ms" << endl;
        //IG.StoreFullIndexGraphJSON("./");
        //IG.deleteEmptyNode();
        end = clock();
        double timeDelEmptyNode = measureTime([&]() { IG.deleteEmptyNode(); });
        LOG << "delEmptyNode(" << timeDelEmptyNode << "ms)" << endl;
        //IG.rebuildCase3();
        double timeRebuildCase3 = measureTime([&]() { IG.rebuildCase3(); });
        LOG << "rebuildCase3(" << timeRebuildCase3 << "ms)" << endl;
        type = UPDATE_ADD_TYPE_CYCLE;;
        goto FINISH;
    }
    else {
        type = UPDATE_ADD_TYPE_SAMESCC;
        end = clock();
        goto FINISH;
    }

FINISH:
    duration = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;
    LOG << "finish(" << duration << "ms)" << endl;
    LOG << "####################" << endl;
    LOG << "type = " << type;
    switch (type)
    {
        case UPDATE_ADD_TYPE_SAMESCC:
            LOG << "UPDATE_ADD_TYPE_SAMESCC" << endl;
            break;
        case UPDATE_ADD_TYPE_NOCYCLE:
            LOG << "UPDATE_ADD_TYPE_NOCYCLE" << endl;
            break;
        case UPDATE_ADD_TYPE_CYCLE:
            LOG << "UPDATE_ADD_TYPE_CYCLE" << endl;
            break;
        case UPDATE_ADD_TYPE_EDGEEXIST:
            LOG << "UPDATE_ADD_TYPE_EDGEEXIST" << endl;
            break;
        default:
            LOG << "error type" << endl;
            break;
    }
    LOG << "####################" << endl;
    LOG << endl;
    return true;
}


static void _SCCEdgeInfoRemap(SCCEdgeInfo& s, map<int, int>& remap) {
    for (auto it = s.begin(); it != s.end(); ++it) {
        it->sSCC = remap[it->sSCC];
        it->dSCC = remap[it->dSCC];
    }
}
/*
    更新描述：删除(u,v)
    需要更改的内容：
    不在同一个SCC
    1. originGraph:删除 u->v
    2. SCCTable: 不变
    3. SCCGraph：删除u->v
    4. evolovingGraphSequence: 根据SCCGraph重建
    5. sccEdgeInfoSequence：删除对应项目
    6. NIT：在timeStamp时刻u->v
    7. RefineNIT：重建
    8. IG：重建

    在同一个SCC
    1. originGraph:删除 u->v
    2. SCCTable: 有可能分裂，删除原有项目，添加新的项目
    3. SCCGraph：先收集原SCC的边，删除原SCC的节点，先重建内部边，然后重建外部边
    4. evolovingGraphSequence: 根据SCCGraph重建
    5. sccEdgeInfoSequence：需要把新的分裂后的SCC的内部的边加入
    6. NIT：重建
    7. RefineNIT：重建
    8. IG：重建

    失败情况：
    1. u，v 不存在
    2. u->v不存在
*/

bool HRindex::singleStepUpdateDeleteEdge(int u, int v, int timestamp) {
    LOG << "singleStepUpdateDeleteEdge(" << u << "->" << v << "," << timestamp << ")" << endl;
    clock_t start, end;
    double duration;
    start = clock();

    int uSCCID = -1;
    int vSCCID = -1;

    auto d1 = measureTime(std::bind(&Graph::DeleteEdge, &originGraph[timestamp], u, v));
    LOG << "step1 : DeleteEdge(" << d1 << "ms)" << endl;
    uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    vSCCID = originGraph[timestamp].findSCCIDFromNodeId(v);
    LOG << " u = " << u << " uSCCID = " << uSCCID << endl;
    LOG << " v = " << v << " vSCCID = " << vSCCID << endl;
    assert(uSCCID != -1 && vSCCID != -1);
    SCCGraph& thisSCCGraph = sccGraphs[timestamp];
    Lifespan tx = LifespanBuild(timestamp, timestamp);
    int type = -1;
    
    if (uSCCID != vSCCID) {
        LOG << "uSCCID != vSCCID" << endl;
        
        //不在一个SCC内，所以对SCC内部没有影响,并且SCC的节点是没有变化的
        //需要注意，一个SCC的边可能对应多个原始图中的边，因此先检查是由还有其他边的存在
        int returnValue = thisSCCGraph.deleteEdge(uSCCID, vSCCID, u, v);
        if (returnValue == -1) {
            //边不存在
            end = clock();
            LOG << "SCC DELETE EDGE : EDGE NOT EXIST !" << endl;
            goto ABORT;
        }
        if (returnValue == 0) {
            //还有其他边存在，没有影响
            LOG << "SCC DELETE EDGE : EDGE STILL EXIST !" << endl;
            type = UPDATE_DEL_TYPE_KEEPEDGE;
            end = clock();
            goto FINISH;
        }
        LOG << "SCC DELETE EDGE : EDGE DELETED !" << endl;
        //删除SCCGraph中的(uSCCID, vSCCID)边
        reconstructEvolvingGraphSequence(timestamp);

        //删除u->v,要更改u的出边，v的入边
        auto uexist = [&uSCCID](RecordItem& ri) {return ri.node == uSCCID;};
        auto vexist = [&vSCCID](RecordItem& ri) {return ri.node == vSCCID;};
        auto uitem = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), uexist);
        auto vitem = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), vexist);
        assert(uitem != nodeInfoTable.end() && vitem != nodeInfoTable.end());
        RefineRecordItem& uritem = findRefineRecordItem(refineNITable, uitem->node);
        RefineRecordItem& vritem = findRefineRecordItem(refineNITable, vitem->node);
        //先更新uSCCID的出边, Out项
        bool find1 = false, find2 = false;
        for (auto uOutit = uitem->Out.begin(); uOutit != uitem->Out.end();++uOutit) {
            if (uOutit->vertexID == vSCCID) {
                find1 = true;
                uOutit->lifespan.set(timestamp, false);
                if (uOutit->lifespan.none()) {
                    uitem->Out.erase(uOutit);
                }
                break;
            }
        }
        assert(find1);
        //遍历N表, 要么在N1表，要么在N2表
        auto uNit = uritem.Out.begin();
        while (uNit != uritem.Out.end())
        {
            if (uNit->vertexID == vSCCID && uNit->lifespan.test(timestamp)) {
                find2 = true;
                if (uNit->partLab == 1) {
                    //N1表中，删除(<Si,tx>, <Sj,tx>)
                    IG.DeleteEdgeKeepEmptyNode(uSCCID, tx, vSCCID, tx);
                    uNit = uritem.Out.erase(uNit);
                }
                else {
                    //N2表中,删除(<Si, LpSi>, <Sj, L2(Si,Sj)>)
                    //删除一个N2中的项，需要有可能需要更改LPsi
                    Lifespan Lpsi_1 = _getLpSi(uritem);
                    if (uNit->lifespan.count() == 1) {
                        assert(uNit->lifespan == tx);
                        IG.DeleteEdgeKeepEmptyNode(uSCCID, Lpsi_1, vSCCID, uNit->lifespan);
                        uNit = uritem.Out.erase(uNit);
                    }
                    else {
                        IG.DeleteEdgeKeepEmptyNode(uSCCID, Lpsi_1, vSCCID, uNit->lifespan);
                        uNit->lifespan.set(timestamp, false);
                        IG.InsertEdgeSrcMustExistOrThrow(uSCCID, Lpsi_1, vSCCID, uNit->lifespan);
                        uNit++;
                    }
                    Lifespan Lpsi_2 = _getLpSi(uritem);
                    if (!Lpsi_2.test(timestamp)) {
                        //u的出边中已经没有了tx时刻的边，需要更新Lpsi
                        IG.ModifyNodeOrThrow(uSCCID, Lpsi_1, Lpsi_2);
                    }
                }
                break;
            }
            uNit++;
        }
        assert(find2);

        //处理vSCCID的入边, In项
        auto find3 = false, find4 = false;
        for (auto vInit = vitem->In.begin(); vInit != vitem->In.end(); ++vInit) {
            if (vInit->vertexID == uSCCID) {
                find3 = true;
                vInit->lifespan.set(timestamp, false);
                if (vInit->lifespan.none()) {
                    vitem->In.erase(vInit);
                }
                break;
            }
        }
        assert(find3);
        //这里可能会导致在tx时刻的入边消失, 发生移动
        Lifespan unionIn;
        for (auto _ : vitem->In) unionIn |= _.lifespan;

        if (unionIn.test(timestamp)) {
            //在tx时刻仍然有入边，不需要处理
            end = clock();
            type = UPDATE_DEL_TYPE_NOTSAMESCC;
            LOG << "process v(u->v), not move" << endl;
            goto FINISH;
        }
        //在tx时刻没有入边，需要把tx时刻的N1表中的边移动到N2表中, 在删除前，必定是N1中含有tx的项，N2中没有
        //移动后，N1中必定不含tx，N2中含有tx，删除前LpSi必定不含有tx
        Lifespan unionOut;
        for (auto _ : vitem->Out) unionOut |= _.lifespan;
        if (!unionOut.test(timestamp)) {
            end = clock();
            type = UPDATE_DEL_TYPE_NOTSAMESCC;
            LOG << "process v(u->v), v.out is empty" << endl;
            goto FINISH;
        }
        //此时的出边必在N1中，移动后，LpSi = LpSi + tx
        Lifespan Lpsi = _getLpSi(vritem);
        IG.ModifyNodeOrThrow(vSCCID, Lpsi, Lpsi | tx);
        Lpsi = Lpsi | tx;
        auto vNit = vritem.Out.begin();
        while (vNit != vritem.Out.end()) {
            if (vNit->partLab == 1 && vNit->lifespan == tx) {
                //移动到N2表中
                auto findRes = find_if(vritem.Out.begin(), vritem.Out.end(),
                    [&](Item& tmp) {return tmp.vertexID == vNit->vertexID && tmp.partLab == 2;});
                if (findRes != vritem.Out.end()) {
                    assert(findRes->lifespan.test(timestamp) == false);
                    //删除原先此节点在N1时构建的边
                    IG.DeleteEdgeKeepEmptyNode(vSCCID, tx, vNit->vertexID, tx);
                    findRes->lifespan.set(timestamp, true);
                    IG.InsertEdgeSrcMustExistOrThrow(vSCCID, Lpsi, findRes->vertexID, findRes->lifespan);
                    vNit = vritem.Out.erase(vNit);
                    int a = 1;
                    a++;
                }
                else {
                    vNit->partLab = 2;
                    IG.DeleteEdgeKeepEmptyNode(vSCCID, tx, vNit->vertexID, tx);
                    IG.InsertEdgeSrcMustExistOrThrow(vSCCID, Lpsi, vNit->vertexID, vNit->lifespan);
                    vNit++;
                }
            }
            else {
                vNit++;
            }
        }
        type = UPDATE_DEL_TYPE_NOTSAMESCC;
        end = clock();
        IG.deleteEmptyNode();
        IG.rebuildCase3();
    }
    else {
        //%%%%%%%%%%%%%%%%%%% PART1: 处理节点分裂 %%%%%%%%%%%%%%%%%%%%%%%
        clock_t part1Timer = clock();
        //在同一个SCC内部，先把SCC内部重新分割
        auto findResult = find_if(sccGraphs[timestamp].vertices.begin(), sccGraphs[timestamp].vertices.end(),
            [&uSCCID](SCCnode& sccnode) { return sccnode.SCCID == uSCCID; });
        if (findResult == sccGraphs[timestamp].vertices.end()) {
            LOG << "SCC not in SCCGraph" << endl;
            goto ABORT;
        }
        set<int> nodeSet = findResult->originNodeSet;
        //收集此SCC的所有边
        vector<SCCEdgeInfoItem> in;
        vector<SCCEdgeInfoItem> out;
        for (auto it = sccGraphs[timestamp].vertices.begin(); it != sccGraphs[timestamp].vertices.end(); ++it) {
            if (it->SCCID == uSCCID) {
                for (auto arcit = it->firstArc; arcit != NULL; arcit = arcit->next) {
                    SCCEdgeInfoItem tmp;
                    tmp.sSCC = uSCCID;
                    tmp.dSCC = arcit->dstID;
                    tmp.nodeEdges = arcit->originEdgeSet;
                    out.push_back(tmp);
                }
            }
            else {
                for (auto arcit = it->firstArc; arcit != NULL; arcit = arcit->next) {
                    if (arcit->dstID == uSCCID) {
                        SCCEdgeInfoItem tmp;
                        tmp.sSCC = it->SCCID;
                        tmp.dSCC = uSCCID;
                        tmp.nodeEdges = arcit->originEdgeSet;
                        in.push_back(tmp);
                    }
                }
            }
        }


        //从SCC中的节点重新构建出一个图
        Graph newGraph;
        for (auto nodeit = nodeSet.begin(); nodeit != nodeSet.end(); ++nodeit) {
            VerNode& findRes = originGraph[timestamp].findNodeRefByID(*nodeit);
            for (auto arcit = findRes.firstArc; arcit != NULL; arcit = arcit->nextarc) {
                if (nodeSet.find(arcit->tarID) != nodeSet.end()) {
                    newGraph.InsertEdge(*nodeit, arcit->tarID);
                }
            }
        }
        //重新分割SCC
        SccTable newSCCTable;
        vector<int> newEvolvingGraph;
        SCCEdgeInfo newSCCEdgeInfo;
        newSCCTable = GetSCCTableFromOneGraph(timestamp, &newGraph, newEvolvingGraph, newSCCEdgeInfo);
        SCCGraph newSCCGraph = SCCGraph(newEvolvingGraph, newSCCTable, newSCCEdgeInfo, timestamp);
        //节点未发生分裂
        if (newSCCGraph.vertices.size() == 1) {
            type = UPDATE_DEL_TYPE_NOSPLIT;
            end = clock();
            goto FINISH;
        }
        map<int, int> SCCRemap;
        //把新图中的SCC与原图之中的SCC合并，本来删除的SCC在SCCtable中是唯一的，但是删除以后分裂的SCC有可能已经存在于SCCtable中
        //加入
        for (auto it = newSCCTable.begin(); it != newSCCTable.end(); ++it) {
            auto res = sccTable.insert(*it);
            if (!res.second) {
                //重新映射
                SCCRemap[it->sccID_Life.scc_id] = res.first->sccID_Life.scc_id;
                res.first->sccID_Life.life_time.set(timestamp, true);
            }
            else {
                int newid = newSCCID(this->sccTable);
                SCCRemap[it->sccID_Life.scc_id] = newid;
                res.first->sccID_Life.scc_id = newid;
            }
        }

        //删除其在SCCTable中的表项
        auto findRes = find_if(sccTable.begin(), sccTable.end(), [&uSCCID](const auto& item) { return item.sccID_Life.scc_id == uSCCID; });
        if (findRes != sccTable.end()) {
            findRes->sccID_Life.life_time.set(timestamp, false);
            if (findRes->sccID_Life.life_time.none()) sccTable.erase(findRes);
        }
        else assert(false);

        //这里把每个原始节点的所属SCCID更新
        newGraph.NewGraphSCCIDRemap(SCCRemap);
        newSCCGraph.SCCIDRemap(SCCRemap);
        LOG << "part1(" << double(clock() - part1Timer) << ")ms" << endl;

        //%%%%%%%%%%%%%%%%%%% PART2: 删除旧的节点 %%%%%%%%%%%%%%%%%%%%%%%
        clock_t part2Timer = clock();
        //删除旧的SCC节点
        thisSCCGraph.deleteNode(uSCCID);
        //开始处理新节点, 先把新的SCC节点全部加入,这里内部边已经加入了
        for (const auto it : newSCCGraph.vertices) {
            assert(thisSCCGraph.addNode(it));
        }
        //先删掉S_u节点
        auto it = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), [&uSCCID](const auto& item) { return item.node == uSCCID; });
        if (it == nodeInfoTable.end()) {
            goto ABORT;
        }
        auto& rit = findRefineRecordItem(refineNITable, it->node);
        auto init = it->In.begin();
        while (init != it->In.end()) {
            if (init->lifespan.test(timestamp)) {
                init->lifespan.set(timestamp, false);
                if (init->lifespan.count() == 0) {
                    init = it->In.erase(init);
                }
                else
                    init++;
            }
            else init++;
        }
        auto outit = it->Out.begin();
        while (outit != it->Out.end()) {
            if (outit->lifespan.test(timestamp)) {
                outit->lifespan.set(timestamp, false);
                if (outit->lifespan.count() == 0) {
                    outit = it->Out.erase(outit);
                }
                else
                    outit++;
            }
            else outit++;
        }

        auto nit = rit.Out.begin();
        while (nit != rit.Out.end()) {
            if (nit->partLab == 1) {
                if (nit->lifespan.test(timestamp)) {
                    IG.DeleteEdgeKeepEmptyNode(uSCCID, tx, nit->vertexID, tx);
                    nit = rit.Out.erase(nit);
                }
                else {
                    nit++;
                }
            }
            else {
                nit++;
            }
        }
        Lifespan Lpsi = _getLpSi(rit);
        //out中有t时刻的出边
        if (Lpsi.test(timestamp)) {
            //lpsi 只有tx，N2中只含有tx的项，直接删除此节点（lpsi 与 L2不相交）
            Lifespan newlife = Lpsi;
            newlife.set(timestamp, false);
            if (Lpsi.count() == 1) {
                IG.ModifyNodeOrThrow(uSCCID, Lpsi, newlife);
                deleteN2(rit);
            }
            //lpsi中还有其他时刻，删除tx时刻的节点
            else {
                //先更改<S_i,Lpsi>
                IG.ModifyNodeOrThrow(uSCCID, Lpsi, newlife);
                Lpsi = _getLpSi(rit);
                assert(Lpsi.test(timestamp) == false);
                //删除N2中的项
                auto nit = rit.Out.begin();
                while (nit != rit.Out.end()) {
                    if (nit->partLab == 2) {
                        if (nit->lifespan.test(timestamp)) {
                            if (nit->lifespan.count() == 1) {
                                IG.DeleteEdgeKeepEmptyNode(uSCCID, Lpsi, nit->vertexID, nit->lifespan);
                                nit = rit.Out.erase(nit);
                            }
                            else {
                                IG.DeleteEdgeKeepEmptyNode(uSCCID, Lpsi, nit->vertexID, nit->lifespan);
                                nit->lifespan.set(timestamp, false);
                                IG.InsertEdgeSrcMustExistOrThrow(uSCCID, Lpsi, nit->vertexID, nit->lifespan);
                                nit++;
                            }
                        }
                        else {
                            nit++;
                        }
                    }
                    else {
                        nit++;
                    }
                }
            }
        }
        LOG << "part2(" << double(clock() - part2Timer) << ")ms" << endl;
        //%%%%%%%%%%%%%%%%%%% PART3: 处理内部节点与外部节点的连接 %%%%%%%%%%%%%%%%%%%%%%%
        clock_t part3Timer = clock();
        //处理新加入的节点
        //处理内部与外部之间的关系，这里我们先不处理内部节点，只确保外部节点的正确性
        //处理out节点,(u,v)，v必然不是新节点，只需要将in中的项改了就行
        for (const SCCEdgeInfoItem& it : out) {
            assert(it.sSCC == uSCCID);
            auto vri = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
                [&it](const auto& item) { return item.node == it.dSCC; });
            assert(vri != nodeInfoTable.end());
            //删除原来In中的<u,t>，加入新的<k1,t>,<k2,t>...
            auto findres = find_if(vri->In.begin(), vri->In.end(),
                [&](const Item& item) { return item.vertexID == uSCCID && item.lifespan.test(timestamp); });
            assert(findres != vri->In.end());
            vri->In.erase(findres);

            for (const auto& outedge : it.nodeEdges) {
                int srcSCCID = newSCCGraph.findSCCIDNodeFromOriginNodeID(outedge.src);
                vector<NodeEdge> tmp;
                tmp.push_back(NodeEdge(outedge.src, outedge.dst));
                thisSCCGraph.insertEdgeNodeMustExist(srcSCCID, it.dSCC, tmp);
                //处理v节点的in边，加入到In中即可,先判断是否存在
                auto findres = find_if(vri->In.begin(), vri->In.end(),
                    [&](const Item& item) { return item.vertexID == srcSCCID; });
                if (findres != vri->In.end()) {
                    findres->lifespan.set(timestamp, true);
                }
                else {
                    Item newItem;
                    newItem.vertexID = srcSCCID;
                    newItem.lifespan.set(timestamp, true);
                    vri->In.push_back(newItem);
                }
            }
        }
        //处理in节点(v,u),这里要处理v的out
        for (const SCCEdgeInfoItem& it : in) {
            //首先，删除v的out中的出边
            assert(it.dSCC == uSCCID);
            auto vri = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
                [&it](const auto& item) { return item.node == it.sSCC; });
            assert(vri != nodeInfoTable.end());
            auto& vrri = findRefineRecordItem(refineNITable, it.sSCC);

            //Sv节点有入边的时刻
            Lifespan LsubSv;
            for (const auto init : vri->In) {
                LsubSv = LsubSv | init.lifespan;
            }
            Lifespan Lpsv = _getLpSi(vrri);

            //删除已经存在的Out项<u,L(u)>
            bool deleteFlag = false;
            auto outit = vri->Out.begin();
            while (outit != vri->Out.end()) {
                if (outit->vertexID == uSCCID && outit->lifespan.test(timestamp)) {
                    deleteFlag = true;
                    outit->lifespan.set(timestamp, false);
                    if (outit->lifespan.count() == 0) {
                        outit = vri->Out.erase(outit);
                    }
                    else
                        outit++;
                }
                else outit++;
            }
            assert(deleteFlag == true);
            //删除N1或者N2中的节点
            auto findN1 = find_if(vrri.Out.begin(), vrri.Out.end(),
                [&](const Item& item) {return item.vertexID == uSCCID && item.lifespan.test(timestamp) && item.partLab == 1;});
            auto findN2 = find_if(vrri.Out.begin(), vrri.Out.end(),
                [&](const Item& item) {return item.vertexID == uSCCID && item.lifespan.test(timestamp) && item.partLab == 2;});
            //只能在一个中找到
            assert((findN1 != vrri.Out.end() && findN2 == vrri.Out.end()) || (findN1 == vrri.Out.end() && findN2 != vrri.Out.end()));
            if (findN1 != vrri.Out.end()) {
                vrri.Out.erase(findN1);
                IG.DeleteEdgeKeepEmptyNode(vrri.node, tx, uSCCID, tx);
            }
            else {
                //这里不需要更改Lpsv，因为后续必定插入新的边
                if (findN2->lifespan.count() == 1) {
                    IG.DeleteEdgeKeepEmptyNode(vrri.node, Lpsv, findN2->vertexID, findN2->lifespan);
                    vrri.Out.erase(findN2);
                }
                else {
                    IG.DeleteEdgeKeepEmptyNode(vrri.node, Lpsv, findN2->vertexID, findN2->lifespan);
                    findN2->lifespan.set(timestamp, false);
                    IG.InsertEdgeSrcMustExistOrThrow(vrri.node, Lpsv, findN2->vertexID, findN2->lifespan);
                }
            }

            //把新边添加进入v的out中，注意，这里可能添加多条边，需要确定每一原始边条边所在的SCC
            for (const auto& inedge : it.nodeEdges) {
                int dstSCCID = newSCCGraph.findSCCIDNodeFromOriginNodeID(inedge.dst);
                vector<NodeEdge> tmp;
                tmp.push_back(NodeEdge(inedge.src, inedge.dst));
                thisSCCGraph.insertEdgeNodeMustExist(it.sSCC, dstSCCID, tmp);
                //把边加入v的out中
                auto findInOut = find_if(vri->Out.begin(), vri->Out.end(),
                    [&](const Item& item) {return item.vertexID == dstSCCID; });
                if (findInOut != vri->Out.end()) {
                    findInOut->lifespan.set(timestamp, true);
                }
                else {
                    Item newItem;
                    newItem.vertexID = dstSCCID;
                    newItem.lifespan.set(timestamp, true);
                    vri->Out.push_back(newItem);
                }

                if (LsubSv.test(timestamp)) {
                    //加入到N1中去，必定之前不存在
                    Item newItem;
                    newItem.vertexID = dstSCCID;
                    newItem.lifespan.set(timestamp, true);
                    newItem.partLab = 1;
                    vrri.Out.push_back(newItem);
                    IG.InsertEdgeSrcMustExistOrThrow(vrri.node, tx, dstSCCID, tx);
                }
                else {
                    //加入到N2中去, 先检查之前是存在
                    auto findN2 = find_if(vrri.Out.begin(), vrri.Out.end(),
                        [&](const Item& item) {return item.vertexID == dstSCCID && item.partLab == 2; });
                    if (findN2 != vrri.Out.end()) {
                        //存在此节点，先删后增
                        //assert(findN2->lifespan.test(timestamp) == false);
                        IG.DeleteEdgeKeepEmptyNode(vrri.node, Lpsv, findN2->vertexID, findN2->lifespan);
                        findN2->lifespan.set(timestamp, true);
                        IG.InsertEdgeSrcMustExistOrThrow(vrri.node, Lpsv, findN2->vertexID, findN2->lifespan);
                    }
                    else {
                        //不存在此节点，按照case2加入
                        Item newItem;
                        newItem.vertexID = dstSCCID;
                        newItem.lifespan.set(timestamp, true);
                        newItem.partLab = 2;
                        vrri.Out.push_back(newItem);
                        IG.InsertEdgeSrcMustExistOrThrow(vrri.node, Lpsv, dstSCCID, tx);
                    }

                }

            }
        }
        //删除分裂前的节点
        reconstructEvolvingGraphSequence(timestamp);
        //删除空的NIT项
        deleteEmptyNIT(nodeInfoTable, refineNITable);
        LOG << "part3(" << double(clock() - part3Timer) << ")ms" << endl;
        //%%%%%%%%%%%%%%%%%%% PART4: 处理内部节点 %%%%%%%%%%%%%%%%%%%%%%%
        clock_t part4Timer = clock();
        //处理内部节点
        for (const auto insideNode : newSCCGraph.vertices) {
            auto _ = thisSCCGraph.getInAndOutNodes(insideNode.SCCID);
            vector<int> incoming = _.first;
            vector<int> outgoing = _.second;
            auto reused = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), [&](const auto& item) { return item.node == insideNode.SCCID; });
            //节点被重用
            if (reused != nodeInfoTable.end()) {
                auto ri = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
                    [&](const RecordItem& item) {return item.node == insideNode.SCCID; });
                assert(ri != nodeInfoTable.end());
                auto& rri = findRefineRecordItem(refineNITable, ri->node);
                //在t时刻肯定为空
                //先处理IN
                for (const auto inid : incoming) {
                    auto exist = find_if(ri->In.begin(), ri->In.end(),
                        [&](const Item& item) {return item.vertexID == inid; });
                    if (exist != ri->In.end()) {
                        //存在此in边
                        exist->lifespan.set(timestamp, true);
                    }
                    else {
                        //不存在此in边
                        Item newItem;
                        newItem.vertexID = inid;
                        newItem.lifespan.set(timestamp, true);
                        ri->In.push_back(newItem);
                    }
                }
                //处理Out
                for (const auto outid : outgoing) {
                    //放入out中
                    auto findOut = find_if(ri->Out.begin(), ri->Out.end(),
                        [&](const Item& item) {return item.vertexID == outid; });
                    if (findOut != ri->Out.end()) {
                        //存在此节点
                        findOut->lifespan.set(timestamp, true);
                    }
                    else {
                        //不存在此节点
                        Item newItem;
                        newItem.vertexID = outid;
                        newItem.lifespan.set(timestamp, true);
                        ri->Out.push_back(newItem);
                    }
                }
                if (outgoing.size() != 0) {
                    Lifespan Lsub;
                    for (const auto& _ : ri->In) {
                        Lsub = Lsub | _.lifespan;
                    }
                    //所有出边要么全是N1型要么全是N2型
                    //处理N1型
                    if (Lsub.test(timestamp)) {
                        for (const auto outid : outgoing) {
                            //放到N1中去,不需要判断存在性
                            Item newItem;
                            newItem.vertexID = outid;
                            newItem.lifespan.set(timestamp, true);
                            newItem.partLab = 1;
                            rri.Out.push_back(newItem);
                            IG.InsertEdgeSrcMustExistOrThrow(rri.node, tx, outid, tx);

                        }
                    }
                    //处理N2型，首先需要计算lpsi是否包含了timestamp
                    Lifespan Lp = _getLpSi(rri);
                    assert(!(Lp.test(timestamp)));

                    Lifespan newLp = Lp | tx;
                    IG.ModifyNodeOrThrow(rri.node, Lp, newLp);
                    for (const auto outid : outgoing) {
                        //放到N2中去，首先判断是否已经存在
                        auto findN2 = find_if(rri.Out.begin(), rri.Out.end(),
                            [&](const Item& item) {return item.vertexID == outid && item.partLab == 2; });

                        if (findN2 != rri.Out.end()) {
                            //存在此节点，先删后增
                            assert(findN2->lifespan.test(timestamp) == false);
                            Lifespan Lp = _getLpSi(rri);
                            IG.DeleteEdgeKeepEmptyNode(rri.node, newLp, findN2->vertexID, findN2->lifespan);
                            findN2->lifespan.set(timestamp, true);
                            IG.InsertEdgeSrcMustExistOrThrow(rri.node, newLp, findN2->vertexID, findN2->lifespan);
                        }
                        else {
                            //不存在此节点，按照case2加入
                            Item newItem;
                            newItem.vertexID = outid;
                            newItem.lifespan.set(timestamp, true);
                            newItem.partLab = 2;
                            rri.Out.push_back(newItem);
                            IG.InsertEdgeSrcMustExistOrThrow(rri.node, newLp, outid, tx);
                        }

                    }
                }
            }
            else {
                //是新节点
                RecordItem newItem;
                newItem.node = insideNode.SCCID;
                //处理IN
                for (const auto inid : incoming) {
                    newItem.In.push_back(Item(inid, tx));
                }
                for (const auto outid : outgoing) {
                    newItem.Out.push_back(Item(outid, tx));
                }
                //所有出边要么全是N1型要么全是N2型
                RefineRecordItem newRefineItem;
                newRefineItem.node = insideNode.SCCID;
                if (outgoing.size() != 0) {
                    Lifespan Lsub;
                    for (const auto& _ : newItem.In) {
                        Lsub = Lsub | _.lifespan;
                    }
                    if (Lsub.test(timestamp)) {
                        //放到N1中去,不需要判断存在性
                        //先创建节点<S,t>
                        IG.CreateVertex(newRefineItem.node, tx);
                        for (const auto outid : outgoing) {
                            Item newItem;
                            newItem.vertexID = outid;
                            newItem.lifespan.set(timestamp, true);
                            newItem.partLab = 1;
                            newRefineItem.Out.push_back(newItem);
                            IG.InsertEdgeSrcMustExistOrThrow(newRefineItem.node, tx, outid, tx);
                        }
                    }
                    else {
                        //处理N2型,Lpsi肯定为tx,此时节点肯定不存在
                        //先创建节点<S,t>
                        IG.CreateVertex(newRefineItem.node, tx);
                        for (const auto outid : outgoing) {
                            Item newItem;
                            newItem.vertexID = outid;
                            newItem.lifespan.set(timestamp, true);
                            newItem.partLab = 2;
                            newRefineItem.Out.push_back(newItem);
                            IG.InsertEdgeSrcMustExistOrThrow(newRefineItem.node, tx, outid, tx);
                        }
                    }
                }
                nodeInfoTable.push_back(newItem);
                refineNITable.push_back(newRefineItem);
            }
        }
        LOG << "part4(" << double(clock() - part4Timer) << ")ms" << endl;
        type = UPDATE_DEL_TYPE_SPLIT;
        end = clock();
        IG.rebuildCase3();
        IG.deleteEmptyNode();
    }


FINISH:
    LOG << "delete edge(" << u << "->" << v << ")at timeStamp" << timestamp << " success!" << endl;
    duration = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;
    LOG << "finish(" << duration << "ms)" << endl;
    LOG << "####################" << endl;
    LOG << "type = " << type;
    switch (type)
    {
        case UPDATE_DEL_TYPE_KEEPEDGE:
            LOG << " UPDATE_DEL_TYPE_KEEPEDGE";
            break;
        case UPDATE_DEL_TYPE_NOSPLIT:
            LOG << " UPDATE_DEL_TYPE_NOSPLIT";
            break;
        case UPDATE_DEL_TYPE_SPLIT:
            LOG << " UPDATE_DEL_TYPE_SPLIT";
            break;
        case UPDATE_DEL_TYPE_NOTSAMESCC:
            LOG << " UPDATE_DEL_TYPE_NOTSAMESCC";
            break;
        default:
            LOG << "error type" << endl;
            break;
    }
    LOG << "####################" << endl;
    LOG << endl;
    return true;

ABORT:
    LOG << "####################" << endl;
    end = clock();
    LOG << "update: delete edge failed!" << endl;
    return false;
}

bool HRindex::singleStepUpdateDeleteNode(int u, int timestamp) {
    vector<int> outList;
    vector<int> inList;
    outList = originGraph[timestamp].findOutArcList(u);
    inList = originGraph[timestamp].findInArcList(u);

    for (auto v = outList.begin(); v != outList.end(); ++v) {
        singleStepUpdateDeleteEdge(u, *v, timestamp);
    }
    for (auto v = inList.begin(); v != inList.end(); ++v) {
        singleStepUpdateDeleteEdge(*v, u, timestamp);
    }

    int uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    originGraph[timestamp].DeleteNode(u);
    sccGraphs[timestamp].deleteNode(u);
    DeleteSccTableByID(sccTable, uSCCID, timestamp);
    reconstructEvolvingGraphSequence(timestamp);

    for (auto it = nodeInfoTable.begin(); it != nodeInfoTable.end(); ++it) {
        if (it->node == uSCCID) {
            nodeInfoTable.erase(it);
            break;
        }
    }
    for (auto it = refineNITable.begin(); it != refineNITable.end(); ++it) {
        if (it->node == uSCCID) {
            refineNITable.erase(it);
            break;
        }
    }
    buildIndexGraph();
    return true;
}

void HRindex::printStatistics() {
    vector<int> nodeNums;
    vector<int> edgeNums;
    vector<int> SCCNums;
    vector<int> SCCEdgeNums;
    for (int i = 0; i < timeIntervalLength; i++) {
        nodeNums.push_back(originGraph[i].GetVexNum());
        edgeNums.push_back(originGraph[i].GetEdgeNum());
        SCCNums.push_back(sccGraphs[i].vertices.size());
        SCCEdgeNums.push_back(sccGraphs[i].getEdgeNum());
        LOG << "timeStamp: " << i;
        LOG << " nodeNum: " << originGraph[i].GetVexNum();
        LOG << " edgeNum: " << originGraph[i].GetEdgeNum();
        LOG << " SCCNum: " << sccGraphs[i].vertices.size();
        LOG << " SCCEdgeNum: " << sccGraphs[i].getEdgeNum() << endl;
    }
    int nodeMean = accumulate(nodeNums.begin(), nodeNums.end(), 0) / nodeNums.size();
    int edgeMean = accumulate(edgeNums.begin(), edgeNums.end(), 0) / edgeNums.size();
    int SCCMean = accumulate(SCCNums.begin(), SCCNums.end(), 0) / SCCNums.size();
    int SCCEdgeMean = accumulate(SCCEdgeNums.begin(), SCCEdgeNums.end(), 0) / SCCEdgeNums.size();
    LOG << "nodeMean: " << nodeMean << " edgeMean: " << edgeMean << " SCCMean: " << SCCMean << " SCCEdgeMean: " << SCCEdgeMean << endl;
    LOG << endl;
}

void HRindex::storeOriginGraph(string dir) {
    for (auto g : originGraph) {
        string fileName = dir + "originGraph" + to_string(g.timestamp) + ".json";
        g.StoreGraphJSON(fileName);
    }
}

void HRindex::storeSCCGraph(string dir) {
    for (auto g : sccGraphs) {
        string fileName = dir + "SCCGraph" + to_string(g.timestamp) + ".json";
        g.storeSCCGraphJSON(fileName);
    }
}

void HRindex::storeIG(string dir) {
    IG.StoreFullIndexGraphJSON(dir);
}

void HRindex::sortAll() {
    for (auto& item : nodeInfoTable) {
        sort(item.In.begin(), item.In.end(), [&](const Item& a, const Item& b) {
            return a.vertexID < b.vertexID;
            });
        sort(item.Out.begin(), item.Out.end(), [&](const Item& a, const Item& b) {
            return a.vertexID < b.vertexID;
            });
    }
    for (auto& ritem : refineNITable) {
        sort(ritem.Out.begin(), ritem.Out.end(), [&](const Item& a, const Item& b) {
            return a.vertexID < b.vertexID;
            });
    }
    sort(nodeInfoTable.begin(), nodeInfoTable.end(), compareRecordItem);
    sort(refineNITable.begin(), refineNITable.end(), compareRefineRecordItem);
    for (int i = 0; i < timeIntervalLength; i++) {
        originGraph[i].gsort();
    }
}

#endif