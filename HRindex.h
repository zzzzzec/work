#ifndef IG_NOOP_5_HRINDEX_H
#define IG_NOOP_5_HRINDEX_H
#include "common.h"
#include "Lifespan.h"
#include "Graph.h"
#include "SCCTable.h"
#include "Process_Snapshots.h"
#include "Construct_NITable.h"
#include "IndexGraph.h"
#include "Process_Query.h"
#include "update.h"
#include "HRindex.h"
#include "SCCGraph.h"
#include "NIT.h"
using namespace std;

class HRindex
{
public:
    int timeIntervalLength;
    string graphDatafileAddHead;
    string recordConstructTime;
    string queryFileAddress;
    string resultFileAddress;

    string storeIndexGraphAddress;
    string storeFull_IG_Address;
    string storeSccTableAddress;
    string storeRefineNITableAddress;

    Graphs originGraph;
    SCCGraph sccGraph;

    vector<vector<int>> evolvingGraphSequence;
    SCCEdgeInfoSequence sccEdgeInfoSequence;
    SccTable sccTable;

    double buildSccTableTime;

    clock_t buildNIT_startTime, buildNIT_endTime;
    NodeInfoTable nodeInfoTable;
    double buildNIT_time;

    clock_t buildRefineNIT_startTime, buildRefineNIT_endTime;
    RefineNITable refineNITable;
    double buildRefineNIT_time;

    clock_t buildIG_startTime, buildIG_endTime;
    IGraph IG;
    double buildIG_time;
    
    HRindex();
    ~HRindex();
    bool buildOriginGraph();
    bool getSCCTable();
    bool stroreSCCTable();
    bool buildSCCGraph();
    bool getNITable();
    bool getRefineNITable();
    bool stroreRefineNITable();
    bool buildIndexGraph();
    bool stroreIndexGraph();

    bool updateFromFile(string updateFileAddress);
    bool updateFromRecords(vector<updateRecord>& updateRecordVector);
    
    int getaNewSCCID();
    bool addSCCnode(int nodeID, int newSCCID, Lifespan lifespan);
    bool addSCCedge(int srcNodeID, int dstNodeID, Lifespan lifespan);
    bool reconstructEvolvingGraphSequence(SCCGraph& sccGraph, int timestamp);
    vector<SCCnode> findCycle();

private:
    bool singleStepUpdate(updateRecord& ur);
    void singleStepUpdateAddNode(int u, int timestamp);
    void singleStepUpdateAddEdge(int u, int v, int timestamp);
    void singleStepUpdateDeleteNode(int u, int timestamp);
    void singleStepUpdateDeleteEdge(int u, int v, int timestamp);
};

HRindex::HRindex() {}
HRindex::~HRindex() {}

bool HRindex::buildOriginGraph() {
    for (int timeStamp = 0; timeStamp < timeIntervalLength; ++timeStamp) {
        Graph g;
        vector<int> dataVector = GetFileData(graphDatafileAddHead, timeStamp);
        auto ne = dataVector.size();
        int num_edges = ne / 2;
        for (int i = 0; i < ne; i = i + 2) {
            g.InsertEdge(dataVector[i], dataVector[i + 1]);
        }
        originGraph.push_back(g);
    }
    return true;
};

bool HRindex::getSCCTable()
{
    this->sccTable = GetSCCTable(timeIntervalLength, originGraph, evolvingGraphSequence, sccEdgeInfoSequence, buildSccTableTime);
    return true;
}

bool HRindex::buildSCCGraph()
{
    sccGraph = SCCGraph(evolvingGraphSequence, sccTable);
    return true;
}

bool HRindex::stroreSCCTable()
{
    StoreSccTable(storeSccTableAddress, sccTable);
    return true;
}

bool HRindex::getNITable() {
    buildNIT_startTime = clock();
    int timestamp;
    for (int i = 0; i <= timeIntervalLength; ++i) {
        timestamp = i;
        nodeInfoTable = GetNITable(nodeInfoTable, evolvingGraphSequence, timestamp);
    }
    buildNIT_endTime = clock();
    buildNIT_time = (double)(buildNIT_endTime - buildNIT_startTime) / CLOCKS_PER_SEC;
    return true;
}

bool HRindex::getRefineNITable() {
    buildRefineNIT_startTime = clock();
    refineNITable = GetRefineNITable(nodeInfoTable);
    buildRefineNIT_endTime = clock();
    buildRefineNIT_time = (double)(buildRefineNIT_endTime - buildRefineNIT_startTime) / CLOCKS_PER_SEC;
    return true;
}

bool HRindex::stroreRefineNITable() {
    StoreRefineNITable(storeRefineNITableAddress, refineNITable);
    return true;
}

bool HRindex::buildIndexGraph() {
    buildIG_startTime = clock();
    IG = BuildIndexGraph(refineNITable);
    buildIG_endTime = clock();
    buildIG_time = (double)(buildIG_endTime - buildIG_startTime) / CLOCKS_PER_SEC;
    return true;
}

bool HRindex::stroreIndexGraph() {
    IG.StoreFullIndexGraph(storeFull_IG_Address);
    return true;
}


bool HRindex::updateFromFile(string updateFileAddress) {
    vector<updateRecord> updateRecordVector;
    readUpdateRecords(updateRecordVector, updateFileAddress);
    for (int i = 0; i < updateRecordVector.size(); ++i) {
        singleStepUpdate(updateRecordVector[i]);
    }
    return true;
}

bool HRindex::updateFromRecords(vector<updateRecord>& updateRecordVector) {
    for (int i = 0; i < updateRecordVector.size(); ++i) {
        singleStepUpdate(updateRecordVector[i]);
    }
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

bool HRindex::reconstructEvolvingGraphSequence(SCCGraph& sccGraph, int timestamp) {
    vector<int> tmp;
    for (auto it = sccGraph.sccGraphs.begin(); it != sccGraph.sccGraphs.end(); ++it)
    {
        if (it->first == timestamp) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
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

bool HRindex::singleStepUpdate(updateRecord& ur) {
    switch (ur.type)
    {
    case(1): {
        singleStepUpdateAddNode(ur.u, ur.timestamp);
        break;
    }
    case(3): {
        singleStepUpdateAddEdge(ur.u, ur.v, ur.timestamp);
        break;
    }
    case(2): {
        singleStepUpdateDeleteEdge(ur.u, ur.v, ur.timestamp);
        break;
    }
    case(4): {
        singleStepUpdateDeleteNode(ur.u, ur.timestamp);
        break;
    }
    default: {
        printf("Error: update type error!\n");
        return false;
    }
    }
    return true;
}

void HRindex::singleStepUpdateAddNode(int u, int timestamp) {
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
    sccGraph.addNode(newSCCID, timestamp, { u });
    //加入一个新的ONTable的表项
    RecordItem newNITableRecordItem;
    newNITableRecordItem.node = newSCCID;
    nodeInfoTable.push_back(newNITableRecordItem);
    //加入一个新的RefineNITable的表项
    RefineRecordItem newRefineRecordItem;
    newRefineRecordItem.node = newSCCID;
    refineNITable.push_back(newRefineRecordItem);
}

void HRindex::singleStepUpdateAddEdge(int u, int v, int timestamp) {
    //添加一条边(u,v),先检测是否存在
    if (!originGraph[timestamp].NodeIsExists(u)) {
        singleStepUpdateAddNode(u, timestamp);
    }
    if (!originGraph[timestamp].NodeIsExists(v)) {
        singleStepUpdateAddNode(v, timestamp);
    }
    /*需要更改的内容
        1. originGraph:新增一条边 u->v
        2. SCCTable: 不改变
        3. SCCGraph：如果SCCID不同，需要改变
        4. evolovingGraphSequence: 根据SCCGraph重建
        5. sccEdgeInfoSequence：TODO
        6. NIT：重建TimeStamp时刻
        7. RefineNIT：重建
        8. IG：重建
    */
    originGraph[timestamp].InsertEdgeWithCheck(u, v);
    int uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    int vSCCID = originGraph[timestamp].findSCCIDFromNodeId(v);
    assert(uSCCID != -1 && vSCCID != -1);
    if (uSCCID != vSCCID) {
        //循环检测环，直到没有环为止
        sccGraph.addEdge(uSCCID, vSCCID, timestamp);
        vector<SCCnode> cycle = sccGraph.findCycle(uSCCID, timestamp);
        while (cycle.size() != 0) {
            //考虑合并后的SCC与其他SCC相同
            int reusedID = sccGraph.merge(cycle, timestamp, sccTable);
            cycle = sccGraph.findCycle(reusedID, timestamp);
        }
        reconstructEvolvingGraphSequence(sccGraph, timestamp);

        int id;
        auto exist = [&id](RecordItem& ri) {return ri.node == id;};
        //整个删除在timeStamp时刻的NIT中的内容，然后重建
        deleteNIT(nodeInfoTable, timestamp);
        for (auto sccGraphit = sccGraph.sccGraphs[timestamp].second.begin();
            sccGraphit != sccGraph.sccGraphs[timestamp].second.end(); ++sccGraphit) {
            id = sccGraphit->SCCID;
            vector<int> INlist;
            vector<int> OUTlist;
            //遍历整个图来找到这个SCCID的入边
            for (auto INlistit = sccGraph.sccGraphs[timestamp].second.begin();
                INlistit != sccGraph.sccGraphs[timestamp].second.end(); ++INlistit) {
                if (INlistit->SCCID == id) continue;
                else {
                    for (auto arcit = INlistit->firstArc; arcit != NULL; arcit = arcit->next)
                    {
                        if (arcit->dstID == id) {
                            INlist.push_back(INlistit->SCCID);
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

        //更新RefineNITable
        this->refineNITable = GetRefineNITable(nodeInfoTable);
        this->IG = BuildIndexGraph(this->refineNITable);
    }
}

void HRindex::singleStepUpdateDeleteEdge(int u, int v, int timestamp) {
    //删除一条边u->v
    /*需要更改的内容
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
        5. sccEdgeInfoSequence：需要合并
        6. NIT：重建
        7. RefineNIT：重建
        8. IG：重建
    */
    try { originGraph[timestamp].DeleteEdge(u, v); }
    catch (const char* msg) {
        cout << msg << endl;
        goto ABORT;
    }
    
    int uSCCID = originGraph[timestamp].findSCCIDFromNodeId(u);
    int vSCCID = originGraph[timestamp].findSCCIDFromNodeId(v);
    if (uSCCID != vSCCID) {
        //不在一个SCC内，所以对SCC内部没有影响,并且SCC的节点是没有变化的
        try { sccGraph.deleteEdge(uSCCID, vSCCID, timestamp);}
        catch (const char* msg) {
            cout << msg << endl;
            goto ABORT;
        }
        reconstructEvolvingGraphSequence(sccGraph, timestamp);
        for (auto sccEdgeInfoSequenceit = sccEdgeInfoSequence[timestamp].begin();
            sccEdgeInfoSequenceit != sccEdgeInfoSequence[timestamp].end(); ++sccEdgeInfoSequenceit) {
            if (sccEdgeInfoSequenceit->first.sScc == uSCCID && sccEdgeInfoSequenceit->first.tScc == vSCCID) {
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
            cout << "node not in NIT" << endl;
            goto ABORT;
        }
        for (auto uOutit = urecord->Out.begin(); uOutit != urecord->Out.end(); ++uOutit) {
            if (uOutit->vertexID == vSCCID) {
                uOutit->lifespan.set(timestamp, false);
                if (uOutit->lifespan.none()) {
                    urecord->Out.erase(uOutit);
                }
            }
        }
        for (auto vInit = vrecord->In.begin(); vInit != vrecord->In.end(); ++vInit) {
            if (vInit->vertexID == uSCCID) {
                vInit->lifespan.set(timestamp, false);
                if (vInit->lifespan.none()) {
                    vrecord->In.erase(vInit);
                }
            }
        }
        this->refineNITable = GetRefineNITable(nodeInfoTable);
        this->IG = BuildIndexGraph(this->refineNITable);
    }
    else {
        //在同一个SCC内部，先把SCC内部重新分割
        auto findResult = find_if(sccGraph.sccGraphs[timestamp].second.begin(), sccGraph.sccGraphs[timestamp].second.end(),
            [&uSCCID](SCCnode& sccnode) { return sccnode.SCCID == uSCCID; });
        if(findResult == sccGraph.sccGraphs[timestamp].second.end()) {
            cout << "SCC not in SCCGraph" << endl;
            goto ABORT;
        }
        set<int> nodeSet = findResult->originNodeSet;
        //收集此SCC的所有边
        vector<SCCEdge> in;
        vector<SCCEdge> out;
        for (auto it = sccGraph.sccGraphs[timestamp].second.begin(); it != sccGraph.sccGraphs[timestamp].second.end(); ++it) {
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
        //把新图中的SCC与原图之中的SCC合并，本来删除的SCC在SCCtable中是唯一的，但是删除以后分裂的SCC有可能已经存在于SCCtable中
        map<int, int> SCCIDmap;
        sccGraph.deleteNode(uSCCID, timestamp);
        for (auto it = newSCCTable.begin(); it != newSCCTable.end(); ++it) {
            auto res = sccTable.insert(*it);
            if (!res.second) {
                SCCIDmap[it->sccID_Life.scc_id] = res.first->sccID_Life.scc_id;
                res.first->sccID_Life.life_time.set(timestamp, true);
            }
            else {
                int newid = newSCCID(this->sccTable);
                res.first->sccID_Life.scc_id = newid;
                sccGraph.addNode(newid, timestamp, res.first->nodeGroup);
                SCCIDmap[it->sccID_Life.scc_id] = newid;
            }
        }
        //先加入内部的边
        for (auto it = newSCCEdgeInfo.begin(); it != newSCCEdgeInfo.end(); ++it) {
            sccGraph.addEdge(SCCIDmap[it->first.sScc], SCCIDmap[it->first.tScc], timestamp);
        }
        //将外部的节点与SCC相连，外部->内部，需要确定内部是哪一个SCC
        for (auto it = in.begin(); it != in.end(); ++it) {
            auto dstSCCID = it->tScc;
            auto srcSCCID = it->sScc;
            auto sccEdgeInfo = sccEdgeInfoSequence[timestamp];
            auto findResult = find_if(sccEdgeInfo.begin(), sccEdgeInfo.end(), [&](SCCEdgeInfoItem& item) {return item.first.sScc == srcSCCID && item.first.tScc == dstSCCID; });
            if (findResult != sccEdgeInfo.end()) {
                for (auto edgeit = findResult->second.begin(); edgeit != findResult->second.end(); ++edgeit) {
                    int insideSCCID = newGraph.findSCCIDFromNodeId(edgeit->dst);
                    sccGraph.addEdge(srcSCCID, insideSCCID, timestamp);
                } 
            }
        }
        //将新的SCC与外部相连, 内部->外部，需要确定内部是哪一个SCC
        for (auto it = out.begin(); it != out.end(); ++it) {
            auto dstSCCID = it->tScc;
            auto srcSCCID = it->sScc;
            auto sccEdgeInfo = sccEdgeInfoSequence[timestamp];
            auto findResult = find_if(sccEdgeInfo.begin(), sccEdgeInfo.end(), [&](SCCEdgeInfoItem& item) {return item.first.sScc == srcSCCID && item.first.tScc == dstSCCID; });
            if (findResult != sccEdgeInfo.end()) {
                for (auto edgeit = findResult->second.begin(); edgeit != findResult->second.end(); ++edgeit) {
                    auto res = sccGraph.findSCCIDNodeFromOriginNodeID(edgeit->src, timestamp);
                    assert(res != -1);
                    int newSCCSrcID = res;
                    //此边为newSCCSrcID->dstSCCID的边
                    sccGraph.addEdge(newSCCSrcID, dstSCCID, timestamp);
                }
            }
            else assert(false);
        }
        reconstructEvolvingGraphSequence(sccGraph, timestamp);
    }

ABORT:
    cout << "update: delete edge failed!" << endl;
    return;
}

void HRindex::singleStepUpdateDeleteNode(int u, int timestamp) {
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
    sccGraph.deleteNode(u, timestamp);
    DeleteSccTableByID(sccTable, uSCCID, timestamp);
    reconstructEvolvingGraphSequence(sccGraph, timestamp);

    for (auto it = nodeInfoTable.begin(); it != nodeInfoTable.end(); ++it) {
        if (it->node == uSCCID) {
            nodeInfoTable.erase(it);
        }
    }
    for (auto it = refineNITable.begin(); it != refineNITable.end(); ++it) {
        if(it->node == uSCCID) {
            refineNITable.erase(it);
        }
    }
    buildIndexGraph();
}

#endif