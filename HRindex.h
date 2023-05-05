#ifndef IG_NOOP_5_HRINDEX_H
#define IG_NOOP_5_HRINDEX_H
#define UPDATE_TYPE_ADD_NODE 1
#define UPDATE_TYPE_ADD_EDGE 2
#define UPDATE_TYPE_DELETE_NODE 3
#define UPDATE_TYPE_DELETE_EDGE 4


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
        string resultFileAddress){
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
        vector<int> dataVector = GetFileData2(graphDatafileAddHead, timeStamp);
        auto ne = dataVector.size();
        int num_edges = ne / 2;
        for (int i = 0; i < ne; i = i + 2) {
            g.InsertEdge(dataVector[i], dataVector[i + 1]);
        }
        originGraph.push_back(g);
    }
    buildOriginGraphEndTime = clock();
    buildOriginGraphTime = (double)(buildOriginGraphEndTime - buildOriginGraphStartTime) / CLOCKS_PER_SEC;
    LOG  << "step1:buildOriginGraph(" << std::fixed << std::setprecision(2) << buildOriginGraphTime << "s)" << endl;
    return true;
};

bool HRindex::getSCCTable()
{
    buildSccTableStartTime = clock();
    this->sccTable = GetSCCTable(timeIntervalLength, originGraph, evolvingGraphSequence, sccEdgeInfoSequence, buildSccTableTime);
    buildSccTableEndTime = clock();
    buildSccTableTime = (double)(buildSccTableEndTime - buildSccTableStartTime) / CLOCKS_PER_SEC;
    LOG  << "step2:buildSccTable(" << std::fixed << std::setprecision(2) << buildSccTableTime << "s)" << endl;
    return true;
}

bool HRindex::buildSCCGraph()
{
    buildSCCGraphStartTime = clock();
    sccGraphs = BuildSCCGraphs(evolvingGraphSequence, sccTable, timeIntervalLength);
    buildSCCGraphEndTime = clock();
    buildSCCGraphTime = (double)(buildSCCGraphEndTime - buildSCCGraphStartTime) / CLOCKS_PER_SEC;
    LOG  << "step3:buildSCCGraph(" << std::fixed << std::setprecision(2) << buildSCCGraphTime << "s)" << endl;
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
    LOG  << "step4:buildNITable(" << std::fixed << std::setprecision(2) << buildNITTime << "s)" << endl;
    return true;
}

bool HRindex::getRefineNITable() {
    buildRefineNITStartTime = clock();
    refineNITable.clear();
    refineNITable = GetRefineNITable(nodeInfoTable);
    buildRefineNITEndTime = clock();
    buildRefineNITTime = (double)(buildRefineNITEndTime - buildRefineNITStartTime) / CLOCKS_PER_SEC;
    LOG  << "step5:buildRefineNITable(" << std::fixed << std::setprecision(2) << buildRefineNITTime << "s)" << endl;
    return true;
}

bool HRindex::buildIndexGraph() {
    buildIGStartTime = clock();
    IG = BuildIndexGraph(refineNITable);
    buildIGEndTime = clock();
    buildIGTime = (double)(buildIGEndTime - buildIGStartTime) / CLOCKS_PER_SEC;
    LOG  << "step6:buildIndexGraph(" << std::fixed << std::setprecision(2) << buildIGTime << "s)" << endl;
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
                arc* a = it2->firstArc;
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
        else {
            if (method == 2)
                return singleStepUpdateAddEdge2(ur.u, ur.v, ur.timestamp);
            else {
                return singleStepUpdateAddEdge3(ur.u, ur.v, ur.timestamp);
            }
        }
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
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC ) * 1000;
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
    LOG << "step2.1 : findSCCIDFromNodeId(" << u <<")(" << d1 << "ms)" << endl;
    auto [d2, vSCCID] = measureTime<int>(std::bind(&Graph::findSCCIDFromNodeId, &originGraph[timestamp], v));
    LOG << "step2.2 : findSCCIDFromNodeId(" << v << ")(" << d1 << "ms)" << endl;
    //int uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    //int vSCCID = originGraph[timestamp].findSCCIDFromNodeId(v);
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        //循环检测环，直到没有环为止
        sccGraphs[timestamp].addEdge(uSCCID, vSCCID);
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

        auto[d3, notuse] = measureTime<bool>(std::bind(&HRindex::reconstructEvolvingGraphSequence, this, timestamp));
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
    for(auto it : ri.In) {
        res = LifespanUnion(res, it.lifespan);
    }
    return res;
}

/*更新IG
如果一个节点的IN和OUT都没有变化，那么 NIT 不变 -> RefineNIT不变 -> IG 不变
对于cycleList中的节点消失，会产生变化的有
in1          out1
in2 -> Si -> out2 
in3          out3
更新伪代码
for item in NIT：
    if node not in  cycleList:
        for in in item.in:
            if in.node in cycleList:
                change = true
                if in.liefe.test(tx): in.node.set(tx = false)
                if(in.node.size == 0) remove in
        for out in item.out:
            if out.node in cycleList:
                change = true
                if in.liefe.test(tx): out.node.set(tx = false)
                if(out.node.size == 0) remove out
        union1 = U(it in item.IN)

        for out in refineNIT.out:
            if tag == 1 && node in cycleList && time = tx
                remove out
                delete(<Si,tx> -> <Sj,tx>) [must exist]
            if tag == 2 && node in cycleList && tx -> time
                if out.time == tx:
                    remove out
                    delete(<Si,L+(Si)> -> <Sj,tx>)
                if out.time include tx:
                    out.time.set(tx, false)
                    delete(<Si, L+(Si)> -> (Sj, L2(Si,Sj)))
                    add(<Si, L+(Si)> -> (Sj, L2(Si,Sj) - tx))
        union2 = U(it in refineTable.out.tag==2)
        if tx not in union2 && tx not in union1 : <Si, L+(Si)>) change to <Si, L+(Si) - tx> (must not exist)

        if tx in union: pass
        else:
            for it in refineTable.N1:
                move it to N2
                delete(<Si,tx> -> <Sj,tx>) [must exist]
                if Sj in N2:
                    delete<Si,L+(Si) -> <Sj,L2(Si,Sj)>>
                    add<Si,L+(Si)> -> <Sj, L2(Si,Sj)>
                else:
                    add<Si,L+(Si)> -> <Sj, tx>
    else:
        delete (nit.in, tx)
        if out.size == 0 : return
        delete (nit.out, tx)
        for it in N1:
            delete<Si, tx> -> <Sj,tx>
        <Si, L+(Si)> -> <Si, L+(Si) - tx>
        for it in N2:
            delete<Si,L+(Si) - tx> -> <Sj, L2(Si,Sj)>
            add<Si, L+(Si) - tx> -> <Sj, L2(Si, Sj) - tx>
        delete (refiennit.out, tx)

    //处理新增节点
    in, out = getINOUT()
    if newSCC exist:
        //这里会带来什么变化 TODO
        //1. IN中增加了tx时刻
        //2. out中
        in, out -> update

    else:


*/
bool HRindex::singleStepUpdateAddEdge2(int u, int v, int timestamp) {
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
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC ) * 1000;
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
    LOG << "step2.1 : findSCCIDFromNodeId(" << u <<")(" << d1 << "ms)" << endl;
    auto [d2, vSCCID] = measureTime<int>(std::bind(&Graph::findSCCIDFromNodeId, &originGraph[timestamp], v));
    LOG << "step2.2 : findSCCIDFromNodeId(" << v << ")(" << d1 << "ms)" << endl;
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        SCCGraph& thisSCCGraph = sccGraphs[timestamp];
        sccGraphs[timestamp].addEdge(uSCCID, vSCCID);
        //循环检测环，直到没有环为止
        clock_t findCycleStart, findCycleEnd;
        double findCycleDuration;
        findCycleStart = clock();
        int cycleNum = 0;
        //这里或许可以改进一下，现在的算法是循环检测环，了解一下coloring算法
        Lifespan tx = LifespanBuild(timestamp, timestamp);
        vector<SCCnode> cycle = sccGraphs[timestamp].findCycle(uSCCID);
        
        while (cycle.size() != 0) {
            int newSCCID = sccGraphs[timestamp].merge(cycle, sccTable);
            reconstructEvolvingGraphSequence(timestamp);
            cycleNum++;
            //加入新的SCC节点, 使用SCCGraph来获取出入边信息
            auto newSCCFindRes = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
                [&](RecordItem& item) {return item.node == newSCCID;});
            vector<int> SCCNewNodein;
            vector<int> SCCNewNodeout;
            auto inoutPair = sccGraphs[timestamp].getInAndOutNodes(newSCCID);
            SCCNewNodein = inoutPair.first;
            SCCNewNodeout = inoutPair.second;
            sort(nodeInfoTable.begin(), nodeInfoTable.end(), compareRecordItem);
            sort(refineNITable.begin(), refineNITable.end(), compareRefineRecordItem);
            //先处理新增的SCCNode
            //合并后的SCC在其他的时刻有出现
            if (newSCCFindRes != nodeInfoTable.end()) {
                for (auto init : SCCNewNodein) {
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
                for (auto outit : SCCNewNodeout) {
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
            
            for (auto& item : nodeInfoTable) {
                RefineRecordItem& ritem = findRefineRecordItem(refineNITable, item.node);
                int Si = item.node;

                //if node not in  cycleList:
                if (!sccIncycle(item.node, cycle)) {
                    /*
                        需要考虑新增的NewSCCID如何加入，一共有两种情况，最多只能是其中的一种
                        1. 新增的在IN， 不会导有入边的时间的变化，Out中只需要删除cycle中的节点
                        2. 新增的在OUT，导致LpSi的变化
                    */
                    //添加新的节点
                    auto findINRes = find_if(SCCNewNodein.begin(), SCCNewNodein.end(), [&](auto i) {return item.node == i;});
                    auto findOUTRes = find_if(SCCNewNodeout.begin(), SCCNewNodeout.end(), [&](auto i) {return item.node == i;});
                    //不能两个都找到
                    assert(!(findINRes != SCCNewNodein.end() && findOUTRes != SCCNewNodeout.end()));
                    Lifespan unionIN;
                    for (auto in : item.In) unionIN = unionIN | in.lifespan;
                    bool hasInattx = unionIN.test(timestamp);

                    if (findINRes != SCCNewNodein.end()) {
                        //在IN中找到，要加在这个节点的out中
                        Item newItem;
                        newItem.vertexID = newSCCID;
                        newItem.lifespan = tx;
                        Item newRefineItem;
                        newRefineItem.vertexID = newSCCID;
                        newRefineItem.lifespan = tx;
                        item.Out.push_back(newItem);
                        
                        if (unionIN.test(timestamp)) {
                            //放到N1中
                            newRefineItem.partLab = 1;
                            ritem.Out.push_back(newRefineItem);
                            IG.InsertEdge(item.node, tx, newSCCID, tx);
                        }
                        else {
                            //放到N2中
                            Lifespan LpSi = _getLpSi(ritem);
                            newRefineItem.partLab = 2;
                            auto returnValue = refineNITINsertOrThrow(ritem, newRefineItem);
                            assert(returnValue.first);

                            if (LpSi.none()) {
                                //本来N2就是空的
                                IG.InsertEdge(item.node, tx, newSCCID, tx);
                            }
                            else {
                                //之前的N2必定不空
                                if (LpSi.test(timestamp)) {
                                    //本来就含有tx时刻，直接插入
                                    IG.InsertEdgeSrcMustExistOrThrow(item.node, LpSi, newSCCID, tx);
                                }
                                else {
                                    //本来不含有tx时刻，需要修改LpSi
                                    Lifespan newlife = LpSi;
                                    newlife.set(timestamp, true);
                                    IG.ModifyNodeOrThrow(Si, LpSi, newlife);
                                    LpSi = _getLpSi(ritem);
                                    IG.InsertEdgeSrcMustExistOrThrow(Si, LpSi, newSCCID, tx);
                                }
                            }
                        }
                    }
                    if(findOUTRes != SCCNewNodeout.end()) {
                        //在out中找到，要加在这个节点的IN中
                        Item newItme;
                        newItme.vertexID = newSCCID;
                        newItme.lifespan = tx;
                        item.In.push_back(newItme);
                    }

                    //删除cycle中的节点部分
                    auto in = item.In.begin();
                    while (in != item.In.end()) {
                        if (sccIncycle(in->vertexID, cycle)) {
                            if (in->lifespan.test(timestamp))
                                in->lifespan.set(timestamp, false);
                            if (in->lifespan.none())
                                in = item.In.erase(in);
                            else
                                ++in;
                        }
                        else {
                            ++in;
                        }
                    }

                    auto out = item.Out.begin();
                    while (out != item.Out.end()) {
                        if (sccIncycle(out->vertexID, cycle)) {
                            if (out->lifespan.test(timestamp))
                                out->lifespan.set(timestamp, false);
                            if (out->lifespan.none())
                                out = item.In.erase(in);
                            else
                                ++out;
                        }
                        else {
                            ++out;
                        }
                    }

                    Lifespan union1;
                    for (const auto &in : item.In) union1 = union1 | in.lifespan;

                    auto rit = ritem.Out.begin();
                    Lifespan LpSi = _getLpSi(ritem);
                    
                    while (rit != ritem.Out.end()) {
                        if (rit->partLab == 1 && sccIncycle(rit->vertexID, cycle) && rit->lifespan.test(timestamp)) {
                            //delete(<Si,tx> -> <Sj,tx>) [must exist]
                            IG.DeleteEdgeKeepEmptyNode(Si, tx, rit->vertexID, tx);
                            rit = ritem.Out.erase(rit);
                            continue;
                        }
                        if (rit->partLab == 2 && sccIncycle(rit->vertexID, cycle) && LifespanisSub(rit->lifespan, tx)) {
                            //timestamp = tx
                            if (rit->lifespan.count() == 1 && rit->lifespan.test(timestamp)) {
                                //delete(<Si,L+(Si)> -> <Sj,tx>)
                                IG.DeleteEdgeKeepEmptyNode(Si, LpSi, rit->vertexID, tx);
                                rit = ritem.Out.erase(rit);
                                continue;
                            }
                            else {
                                //lifespan include tx
                                assert(rit->lifespan.test(timestamp));
                                IG.DeleteEdgeKeepEmptyNode(Si, LpSi, rit->vertexID, rit->lifespan);
                                rit->lifespan.set(timestamp, false);
                                assert(!rit->lifespan.none());
                                IG.InsertEdgeSrcMustExistOrThrow(Si, LpSi, rit->vertexID, rit->lifespan);
                                rit++;
                                continue;
                            }
                        }
                        rit++;
                    }

                    Lifespan union2 = _getLpSi(ritem);
                    if ((!union1.test(timestamp)) && (!union2.test(timestamp))) {
                        //if tx not in union2 && tx not in union1 : <Si, L+(Si)>) change to <Si, L+(Si) - tx> (must not exist)
                        Lifespan newLife = LpSi;
                        newLife.set(timestamp, false);
                        IG.ModifyNodeOrThrow(Si, LpSi, newLife);
                    }

                    LpSi = _getLpSi(ritem);
                    if (!(union1.test(timestamp))) {
                        //如果IN中cycle中的一条边<Cycle, Si>的消失，那么必然会被加入<newSCCID, Si>, 所以至始至终有入边的时间段是不变的
                        assert(false);
                        auto it = ritem.Out.begin();
                        while (it != ritem.Out.end()) {
                            if (it->partLab == 1) {
                                //delete(<Si,tx> -> <Sj,tx>) [must exist]
                                IG.DeleteEdgeKeepEmptyNode(Si, tx, it->vertexID, tx);
                                //move it to N2 [<Sj, tx>]
                                bool foundInN2 = false;
                                for (auto it2 : ritem.Out) {
                                    if (it2.partLab == 2 && it2.vertexID == it->vertexID) {
                                        assert(!it2.lifespan.test(timestamp));
                                        /*
                                        if Sj in N2:
                                            delete<Si,L+(Si) -> <Sj,L2(Si,Sj)>>
                                            add<Si,L+(Si)> -> <Sj, L2(Si,Sj)>
                                        */
                                        IG.DeleteEdgeKeepEmptyNode(Si, LpSi, it->vertexID, it->lifespan);
                                        it2.lifespan.set(timestamp, true);
                                        IG.InsertEdgeSrcMustExistOrThrow(Si, LpSi, it->vertexID, it->lifespan);
                                        //IG.InsertEdge(Si, LpSi, )
                                        foundInN2 = true;
                                        break;
                                    }
                                }
                                if (!foundInN2) {
                                    Item newItem;
                                    newItem.partLab = 2;
                                    newItem.vertexID = it->vertexID;
                                    newItem.lifespan = LifespanBuild(timestamp, timestamp);
                                    ritem.Out.push_back(newItem);
                                    IG.InsertEdgeSrcMustExistOrThrow(Si, LpSi, it->vertexID, tx);
                                }
   
                                it = ritem.Out.erase(it);
                            }
                            else {
                                it++;
                            }
                        }
                    }

                    //插入边对于UnionIN不会有任何影响
                    for (auto in : item.In) unionIN = unionIN | in.lifespan;
                    bool hasInattx2 = unionIN.test(timestamp);
                    assert(hasInattx == hasInattx2);
                }
                else {
                    //node in cycleList
                    deleteNITItemIN(item, timestamp);
                    if (item.Out.size() == 0) continue;
                    deleteNITItemOut(item, timestamp);
                    for (auto &it : ritem.Out) {
                        if (it.partLab == 1 && it.lifespan.test(timestamp)) {
                            IG.DeleteEdgeKeepEmptyNode(Si, tx, it.vertexID, tx);
                        }
                    }
                    Lifespan Lpsi = _getLpSi(ritem);
                    for (auto &it : ritem.Out) {
                        if (it.partLab == 2 && it.lifespan.test(timestamp)) {
                            IG.DeleteEdgeKeepEmptyNode(Si, Lpsi, it.vertexID, it.lifespan);
                            Lifespan newlife = LifespanTestAndUnset(it.lifespan, timestamp);
                            IG.InsertEdgeSrcMustExistOrThrow(Si, Lpsi, it.vertexID, newlife);
                        }
                    }
                    deleteRefineNITItem(ritem, timestamp);
                }
            }
            //删除所有在cycle中且含有tx的节点
            IG.StoreFullIndexGraphJSON("./beforeIG.json");
            IG.deleteNodeWhichInCycleAndIncludeTimestamp(cycle, timestamp);
            IG.StoreFullIndexGraphJSON("./afterIG.json");
            IG.deleteEmptyNode();
            cycle = sccGraphs[timestamp].findCycle(newSCCID);
            int a = 1;
            a++;
        }
    }
    end = clock();
    duration = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;
    LOG << "finish(" << duration << "ms)" << endl;
    LOG << endl;
    return true;
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
    LOG << "singleStepUpdateAddEdge: " << u << "->" << v << " " << timestamp << endl;
    //添加一条边(u,v),先检测是否存在
    if (!originGraph[timestamp].NodeIsExists(u)) {
        addNodeStart = clock();
        singleStepUpdateAddNode(u, timestamp);
        addNodeEnd = clock();
        addNodeDuration = ((double)(addNodeEnd - addNodeStart) / CLOCKS_PER_SEC ) * 1000;
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
    LOG << "step2.1 : findSCCIDFromNodeId(" << u <<")(" << d1 << "ms)" << endl;
    auto [d2, vSCCID] = measureTime<int>(std::bind(&Graph::findSCCIDFromNodeId, &originGraph[timestamp], v));
    LOG << "step2.2 : findSCCIDFromNodeId(" << v << ")(" << d1 << "ms)" << endl;
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        SCCGraph& thisSCCGraph = sccGraphs[timestamp];
        sccGraphs[timestamp].addEdge(uSCCID, vSCCID);
        //循环检测环，直到没有环为止
        clock_t findCycleStart, findCycleEnd;
        double findCycleDuration;
        findCycleStart = clock();
        
        int cycleNum = 0;

        Lifespan tx = LifespanBuild(timestamp, timestamp);
        
        //这里需要循环检测环，TODO
        vector<SCCnode> cycle = thisSCCGraph.findCycle(uSCCID);
        int newSCCID = thisSCCGraph.merge(cycle, sccTable);
        assert(thisSCCGraph.findCycle(newSCCID).size() == 0);
        reconstructEvolvingGraphSequence(timestamp);
        //加入新的SCC节点, 使用SCCGraph来获取出入边信息
        auto newSCCFindRes = find_if(nodeInfoTable.begin(), nodeInfoTable.end(),
            [&](RecordItem& item) {return item.node == newSCCID;});
        vector<int> SCCNewNodein;
        vector<int> SCCNewNodeout;
        auto inoutPair = sccGraphs[timestamp].getInAndOutNodes(newSCCID);
        SCCNewNodein = inoutPair.first;
        SCCNewNodeout = inoutPair.second;
        sort(nodeInfoTable.begin(), nodeInfoTable.end(), compareRecordItem);
        sort(refineNITable.begin(), refineNITable.end(), compareRefineRecordItem);
        //先处理新增的SCCNode
        //合并后的SCC在其他的时刻有出现
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

        for (auto& item : nodeInfoTable) {
            LOG << "porcessing node " << item.node << endl;
            RefineRecordItem& ritem = findRefineRecordItem(refineNITable, item.node);
            int Si = item.node;

            //if node not in  cycleList:
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
                            IG.InsertEdgeSrcMustExistOrThrow(Si, Lpsi, newSCCID, tx);
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
        LOG << "deleteNodeWhichInCycleAndIncludeTimestamp" << endl;
        IG.deleteNodeWhichInCycleAndIncludeTimestamp(cycle, timestamp);
        IG.StoreFullIndexGraphJSON("./");
        LOG << "delEmptyNode" << endl;
        IG.deleteEmptyNode();
        IG.rebuildCase3();
    }
    end = clock();
    duration = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;
    LOG << "finish(" << duration << "ms)" << endl;
    LOG << endl;
    return true;
}


static SCCEdgeInfo _SCCEdgeInfoRemap(SCCEdgeInfo& s, map<int, int>& remap) {
    SCCEdgeInfo newS;
    for (auto it = s.begin(); it != s.end(); ++it) {
        SCCEdgeInfoItem newItem = *it;
        newItem.sccEdge.sScc = remap[newItem.sccEdge.sScc];
        newItem.sccEdge.tScc = remap[newItem.sccEdge.tScc];
        newS.insert(newItem);
    }
    return newS;
}

typedef struct {
    int SCCsrc;
    int SCCdst;
    int nodeSrc;
    int nodeDst;
} _updateSCCEdgeInfoParam;
typedef vector<_updateSCCEdgeInfoParam> _updateSCCEdgeInfoParamList;
static void _updateSCCEdgeInfo(SCCEdgeInfo& s, _updateSCCEdgeInfoParamList& paramList) {
    for (auto it = paramList.begin(); it != paramList.end(); ++it) {
        int SCCsrc = it->SCCsrc;
        int SCCdst = it->SCCdst;
        int nodeSrc = it->nodeSrc;
        int nodeDst = it->nodeDst;
        NodeEdge tmp;
        tmp.src = nodeSrc;
        tmp.dst = nodeDst;
        for (auto it2 = s.begin(); it2 != s.end(); ++it2) {
            if (it2->sccEdge.sScc == SCCsrc && it2->sccEdge.tScc == SCCdst) {
                it2->nodeEdges.insert(tmp);
                continue;
            }
            SCCEdgeInfoItem newItem;
            newItem.sccEdge.sScc = SCCsrc;
            newItem.sccEdge.tScc = SCCdst;
            newItem.nodeEdges.insert(tmp);
            s.insert(newItem);
        }
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

    
    try {
        auto d1 = measureTime(std::bind(&Graph::DeleteEdge, &originGraph[timestamp], u, v));
        LOG << "step1 : DeleteEdge(" << d1 << "ms)" << endl;
    }
    catch (const char* msg) {
        LOG  << msg << endl;
        goto ABORT;
    }
    
    uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    vSCCID = originGraph[timestamp].findSCCIDFromNodeId(v);
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        //不在一个SCC内，所以对SCC内部没有影响,并且SCC的节点是没有变化的
        //需要注意，一个SCC的边可能对应多个原始图中的边，因此先检查是由还有其他边的存在
        for (auto it = sccEdgeInfoSequence[timestamp].begin(); it != sccEdgeInfoSequence[timestamp].end(); ++it) {
            if (it->sccEdge.sScc == uSCCID && it->sccEdge.tScc == vSCCID) {
                auto findres = find_if(it->nodeEdges.begin(), it->nodeEdges.end(), [&u, &v](const NodeEdge& ei) {return ei.src == u && ei.dst == v; });
                if (findres == it->nodeEdges.end()) {
                    LOG  << "Error: can not find the edge in sccEdgeInfoSequence" << endl;
                    goto ABORT;
                }
                it->nodeEdges.erase(findres);
                if (it->nodeEdges.empty()) {
                    //SCC边已经不对应原始边了，可以删除
                    sccEdgeInfoSequence[timestamp].erase(it);
                }
                else return true;
                break;
            }
        }
        
        try { sccGraphs[timestamp].deleteEdge(uSCCID, vSCCID); }
        catch (const char* msg) {
            LOG  << msg << endl;
            goto ABORT;
        }
        reconstructEvolvingGraphSequence(timestamp);
        for (auto sccEdgeInfoSequenceit = sccEdgeInfoSequence[timestamp].begin();
            sccEdgeInfoSequenceit != sccEdgeInfoSequence[timestamp].end(); ++sccEdgeInfoSequenceit) {
            if (sccEdgeInfoSequenceit->sccEdge.sScc == uSCCID && sccEdgeInfoSequenceit->sccEdge.tScc == vSCCID) {
                sccEdgeInfoSequence[timestamp].erase(sccEdgeInfoSequenceit);
                break;
            }
        }
        //删除u->v,要更改u的出边，v的入边
        auto uexist = [&uSCCID](RecordItem& ri) {return ri.node == uSCCID;};
        auto vexist = [&vSCCID](RecordItem& ri) {return ri.node == vSCCID;};
        auto urecord = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), uexist);
        auto vrecord = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), vexist);
        if(urecord == nodeInfoTable.end() || vrecord == nodeInfoTable.end()) {
            LOG  << "node not in NIT" << endl;
            goto ABORT;
        }
        for (auto uOutit = urecord->Out.begin(); uOutit != urecord->Out.end();++uOutit) {
            if (uOutit->vertexID == vSCCID) {
                uOutit->lifespan.set(timestamp, false);
                if (uOutit->lifespan.none()) {
                    urecord->Out.erase(uOutit);
                }
                break;
            }
        }
        for (auto vInit = vrecord->In.begin(); vInit != vrecord->In.end(); ++vInit) {
            if (vInit->vertexID == uSCCID) {
                vInit->lifespan.set(timestamp, false);
                if (vInit->lifespan.none()) {
                    vrecord->In.erase(vInit);
                }
                break;
            }
        }
        getRefineNITable();
        buildIndexGraph();
    }
    else {
        //在同一个SCC内部，先把SCC内部重新分割
        auto findResult = find_if(sccGraphs[timestamp].vertices.begin(), sccGraphs[timestamp].vertices.end(),
            [&uSCCID](SCCnode& sccnode) { return sccnode.SCCID == uSCCID; });
        if(findResult == sccGraphs[timestamp].vertices.end()) {
            LOG  << "SCC not in SCCGraph" << endl;
            goto ABORT;
        }
        set<int> nodeSet = findResult->originNodeSet;
        //收集此SCC的所有边
        vector<SCCEdge> in;
        vector<SCCEdge> out;
        for (auto it = sccGraphs[timestamp].vertices.begin(); it != sccGraphs[timestamp].vertices.end(); ++it) {
            if (it->SCCID == uSCCID) {
                for (auto arcit = it->firstArc; arcit != NULL; arcit = arcit->next) {
                    SCCEdge newEdge;
                    newEdge.sScc = uSCCID;
                    newEdge.tScc = arcit->dstID;
                    out.push_back(newEdge);
                }
            }
            else {
                for (auto arcit = it->firstArc; arcit != NULL; arcit = arcit->next) {
                    if (arcit->dstID == uSCCID) {
                        SCCEdge newEdge;
                        newEdge.sScc = it->SCCID;
                        newEdge.tScc = uSCCID;
                        in.push_back(newEdge);
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
        auto findRes = find_if(sccTable.begin(), sccTable.end(), [&uSCCID](auto mapItem) { return mapItem.sccID_Life.scc_id == uSCCID; });
        if (findRes != sccTable.end()) {
            findRes->sccID_Life.life_time.set(timestamp, false);
            if (findRes->sccID_Life.life_time.none()) sccTable.erase(findRes);
        }
        else assert(false);
        map<int, int> SCCRemap;
        //把新图中的SCC与原图之中的SCC合并，本来删除的SCC在SCCtable中是唯一的，但是删除以后分裂的SCC有可能已经存在于SCCtable中
        for (auto it = newSCCTable.begin(); it != newSCCTable.end(); ++it) {
            auto res = sccTable.insert(*it);
            if (!res.second) {
                //重新映射
                SCCRemap[it->sccID_Life.scc_id] = res.first->sccID_Life.scc_id;
                res.first->sccID_Life.life_time.set(timestamp, true);
                sccGraphs[timestamp].addNode(res.first->sccID_Life.scc_id, res.first->nodeGroup);
            }
            else {
                int newid = newSCCID(this->sccTable);
                SCCRemap[it->sccID_Life.scc_id] = newid;
                res.first->sccID_Life.scc_id = newid;
                sccGraphs[timestamp].addNode(newid, res.first->nodeGroup);
            }
        }
        newGraph.NewGraphSCCIDRemap(SCCRemap);
        newSCCEdgeInfo = _SCCEdgeInfoRemap(newSCCEdgeInfo, SCCRemap);
        sccGraphs[timestamp].deleteNode(uSCCID);
        //先加入内部的边
        for (auto it = newSCCEdgeInfo.begin(); it != newSCCEdgeInfo.end(); ++it) {
            if (sccGraphs[timestamp].addEdge(it->sccEdge.sScc, it->sccEdge.tScc) == 0)
                assert(false);
        }
        
        //将外部的节点与SCC相连，外部->内部，需要确定内部是哪一个SCC
        _updateSCCEdgeInfoParamList INparamList;
        _updateSCCEdgeInfoParamList OUTparamList;
        SCCEdgeInfo& sccEdgeInfo = sccEdgeInfoSequence[timestamp];
        for (auto it = in.begin(); it != in.end(); ++it) {
            auto dstSCCID = it->tScc;
            auto srcSCCID = it->sScc;
            auto findResult = find_if(sccEdgeInfo.begin(), sccEdgeInfo.end(), [&](const SCCEdgeInfoItem& item) {return item.sccEdge.sScc == srcSCCID && item.sccEdge.tScc == dstSCCID; });
            if (findResult != sccEdgeInfo.end()) {
                for (auto edgeit = findResult->nodeEdges.begin(); edgeit != findResult->nodeEdges.end(); ++edgeit) {
                    int insideSCCID = newGraph.findSCCIDFromNodeId(edgeit->dst);
                    if(sccGraphs[timestamp].addEdge(srcSCCID, insideSCCID) == 0)
                        assert(false);
                    INparamList.push_back({ srcSCCID, insideSCCID, edgeit->src, edgeit->dst });
                }
            }
        }
        _updateSCCEdgeInfo(sccEdgeInfo, INparamList);
        //将新的SCC与外部相连, 内部->外部，需要确定内部是哪一个SCC
        for (auto it = out.begin(); it != out.end(); ++it) {
            auto dstSCCID = it->tScc;
            auto srcSCCID = it->sScc;
            auto findResult2 = find_if(sccEdgeInfo.begin(), sccEdgeInfo.end(), [&](const SCCEdgeInfoItem& item) {return item.sccEdge.sScc == srcSCCID && item.sccEdge.tScc == dstSCCID; });
            if (findResult2 != sccEdgeInfo.end()) {
                for (auto edgeit = findResult2->nodeEdges.begin(); edgeit != findResult2->nodeEdges.end(); ++edgeit) {
                    auto res = sccGraphs[timestamp].findSCCIDNodeFromOriginNodeID(edgeit->src);
                    assert(res != -1);
                    int newSCCSrcID = res;
                    //此边为newSCCSrcID->dstSCCID的边
                    if (sccGraphs[timestamp].addEdge(newSCCSrcID, dstSCCID) == 0)
                        assert(false);
                    OUTparamList.push_back({ newSCCSrcID, dstSCCID, edgeit->src, edgeit->dst });
                }
            }
            else assert(false);
        }
        _updateSCCEdgeInfo(sccEdgeInfo, OUTparamList);
        //删除原本SCC的SCCEdgeinfo
        for (auto it = sccEdgeInfo.begin(); it != sccEdgeInfo.end();) {
            if (it->sccEdge.sScc == uSCCID || it->sccEdge.tScc == uSCCID) {
                it = sccEdgeInfo.erase(it);
            }
            else ++it;
        }
        int sccEdgeInfoSize = sccEdgeInfo.size();
        sccEdgeInfo.insert(newSCCEdgeInfo.begin(), newSCCEdgeInfo.end());
        assert(sccEdgeInfo.size() == sccEdgeInfoSize + newSCCEdgeInfo.size());
        reconstructEvolvingGraphSequence(timestamp);
        getNITable();
        getRefineNITable();
        buildIndexGraph();
    }
    LOG  << "delete edge(" << u <<"->"<< v << ")at timeStamp" << timestamp <<" success!" << endl;
    return true;

ABORT:
    LOG  << "update: delete edge failed!" << endl;
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
        if(it->node == uSCCID) {
            refineNITable.erase(it);
            break;
        }
    }
    buildIndexGraph();
    return true;
}

void HRindex::printStatistics() {
    for (int i = 0; i < timeIntervalLength; i++) {
        LOG << "timeStamp: " << i;
        LOG << " nodeNum: " << originGraph[i].GetVexNum();
        LOG << " edgeNum: " << originGraph[i].GetEdgeNum();
        LOG << " SCCNum: " << sccGraphs[i].vertices.size();
        LOG << " SCCEdgeNum: " << sccGraphs[i].getEdgeNum() << endl;
    }
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
    for (auto & item : nodeInfoTable) {
        sort(item.In.begin(), item.In.end(), [&](const Item& a, const Item& b) {
            return a.vertexID < b.vertexID;
            });
        sort(item.Out.begin(), item.Out.end(), [&](const Item& a, const Item& b) {
            return a.vertexID < b.vertexID;
            });
    }
    for (auto & ritem : refineNITable) {
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