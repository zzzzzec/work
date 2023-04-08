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
private:

public:
    int timeIntervalLength;
    string graphDatafileAddHead;
    string recordConstructTime;
    string queryFileAddress;
    string resultFileAddress;
    string updateFileAddress;

    string storeIndexGraphAddress;
    string storeFull_IG_Address;
    string storeSccTableAddress;
    string storeRefineNITableAddress;

    Graph* originGraph;
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

    vector<updateRecord> updateRecordVector;
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

    bool update();
    bool singleStepUpdate();
    int getaNewSCCID();
    bool addSCCnode(int nodeID, int newSCCID, Lifespan lifespan);
    bool addSCCedge(int srcNodeID, int dstNodeID, Lifespan lifespan);
    bool reconstructEvolvingGraphSequence(SCCGraph& sccGraph, int timestamp);
    vector<SCCnode> findCycle();
};

HRindex::HRindex() {}
HRindex::~HRindex() {}

bool HRindex::buildOriginGraph() {
    originGraph = new Graph[timeIntervalLength];
    for (int timeStamp = 1; timeStamp < timeIntervalLength + 1; ++timeStamp) {
        vector<int> dataVector = GetFileData(graphDatafileAddHead, timeStamp);
        auto ne = dataVector.size();
        int num_edges = ne / 2;
        int e = 0;
        for (int i = 0; i < ne; i = i + 2) {
            e++;
            if (e % 1000 == 0) {
                printf("Inserting the %dth / %d---------In the %dth Snapshot.\n", e, num_edges, timeStamp);
            }
            originGraph[timeStamp - 1].InsertEdge(dataVector[i], dataVector[i + 1]);
        }
    }
    return true;
};

bool HRindex::getSCCTable()
{
    this->sccTable = GetSCCTable(timeIntervalLength, originGraph, evolvingGraphSequence, sccEdgeInfoSequence, buildSccTableTime);
    buildSCCGraph();
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
    for (int i = 1; i <= timeIntervalLength; ++i) {
        timestamp = i;
        printf("\t Getting NITable at the %dth / %d snapshot...\n", timestamp, timeIntervalLength);
        nodeInfoTable = GetNITable(nodeInfoTable, evolvingGraphSequence, timestamp);
    }
    buildNIT_endTime = clock();
    buildNIT_time = (double)(buildNIT_endTime - buildNIT_startTime) / CLOCKS_PER_SEC;
    printf("build NITable time: %f\n", buildNIT_time);
    return true;
}

bool HRindex::getRefineNITable() {
    buildRefineNIT_startTime = clock();
    refineNITable = GetRefineNITable(nodeInfoTable);
    buildRefineNIT_endTime = clock();
    buildRefineNIT_time = (double)(buildRefineNIT_endTime - buildRefineNIT_startTime) / CLOCKS_PER_SEC;
    printf("build RefineNITable time: %f\n", buildRefineNIT_time);
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
    printf("build IndexGraph time: %f\n", buildIG_time);
    return true;
}

bool HRindex::stroreIndexGraph() {
    IG.StoreFullIndexGraph(storeFull_IG_Address);
    return true;
}


bool HRindex::update() {
    readUpdateRecords(updateRecordVector, updateFileAddress);
    singleStepUpdate();
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
    evolvingGraphSequence[timestamp - 1] = tmp;
    return true;
}


bool HRindex::singleStepUpdate() {
    for (int i = 0; i < updateRecordVector.size(); ++i) {
        updateRecord ur = updateRecordVector[i];
        switch (ur.type)
        {
        case(1): {
            //添加一个节点u，这个节点没有与任何其他边相连，因此是孤立的SCC
            originGraph[ur.timestamp].AddSingleNode(ur.u);
            Lifespan newLifespan;
            int newSCCID = getaNewSCCID();
            addSCCnode(ur.u, newSCCID, LifespanBuild(newLifespan, ur.timestamp, ur.timestamp));
            //加入一个新的ONTable的表项
            RecordItem newNITableRecordItem;
            newNITableRecordItem.node = newSCCID;
            nodeInfoTable.push_back(newNITableRecordItem);
            //加入一个新的RefineNITable的表项
            RefineRecordItem newRefineRecordItem;
            newRefineRecordItem.node = newSCCID;
            refineNITable.push_back(newRefineRecordItem);
            break;
        }
        case(3): {
            //加入一条边u->v
            originGraph[ur.timestamp - 1].InsertEdge(ur.u, ur.v);
            int uSCCID = originGraph[ur.timestamp - 1].findSCCIDFromNodeId(ur.u);
            int vSCCID = originGraph[ur.timestamp - 1].findSCCIDFromNodeId(ur.v);
            if (uSCCID != vSCCID) {
                //循环检测环，直到没有环为止
                sccGraph.addEdge(uSCCID, vSCCID, ur.timestamp);
                vector<SCCnode> cycle = sccGraph.findCycle(uSCCID, ur.timestamp);
                while (cycle.size() != 0) {
                    //考虑合并后的SCC与其他SCC相同
                    int reusedID = sccGraph.merge(cycle, ur.timestamp, sccTable);
                    cycle = sccGraph.findCycle(reusedID, ur.timestamp);
                }
                reconstructEvolvingGraphSequence(sccGraph, ur.timestamp);

                int id;
                auto exist = [&id](RecordItem& ri) {return ri.node == id;};
                //整个删除在timeStamp时刻的NIT中的内容，然后重建
                deleteNIT(nodeInfoTable, ur.timestamp);
                for (auto sccGraphit = sccGraph.sccGraphs[ur.timestamp - 1].second.begin();
                    sccGraphit != sccGraph.sccGraphs[ur.timestamp - 1].second.end(); ++sccGraphit) {
                    id = sccGraphit->SCCID;
                    vector<int> INlist;
                    vector<int> OUTlist;
                    //遍历整个图来找到这个SCCID的入边
                    for (auto INlistit = sccGraph.sccGraphs[ur.timestamp - 1].second.begin();
                        INlistit != sccGraph.sccGraphs[ur.timestamp - 1].second.end(); ++INlistit) {
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
                            insertNITin(*record, *inID, ur.timestamp);
                        }
                        for (auto outID = OUTlist.begin(); outID != OUTlist.end(); ++outID) {
                            insertNITout(*record, *outID, ur.timestamp);
                        }
                    }
                    else {
                        RecordItem newNITableRecordItem;
                        newNITableRecordItem.node = id;
                        for (auto inID = INlist.begin(); inID != INlist.end(); ++inID) {
                            insertNITin(newNITableRecordItem, *inID, ur.timestamp);
                        }
                        for (auto outID = OUTlist.begin(); outID != OUTlist.end(); ++outID) {
                            insertNITout(newNITableRecordItem, *outID, ur.timestamp);
                        }
                        nodeInfoTable.push_back(newNITableRecordItem);
                    }
                }

                //更新RefineNITable
                this->refineNITable = GetRefineNITable(nodeInfoTable);
                this->IG = BuildIndexGraph(this->refineNITable);
            }
            break;
        }
        case(2): {
            //删除一个节点u
            
            break;
        }
        case(4): {
            //删除一条边u->v
            originGraph[ur.timestamp - 1].DeleteEdge(ur.u, ur.v);
            int uSCCID = originGraph[ur.timestamp - 1].findSCCIDFromNodeId(ur.u);
            int vSCCID = originGraph[ur.timestamp - 1].findSCCIDFromNodeId(ur.v);
            if (uSCCID != vSCCID) {
                //不在一个SCC内，所以对SCC内部没有影响,并且SCC的节点是没有变化的
                sccGraph.deleteEdge(uSCCID, vSCCID, ur.timestamp);
                reconstructEvolvingGraphSequence(sccGraph, ur.timestamp);
                //删除u->v,要更改u的出边，v的入边
                auto uexist = [&uSCCID](RecordItem& ri) {return ri.node == uSCCID;};
                auto vexist = [&vSCCID](RecordItem& ri) {return ri.node == vSCCID;};
                auto urecord = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), uexist);
                auto vrecord = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), vexist);
                for (auto uOutit = urecord->Out.begin(); uOutit != urecord->Out.end(); ++uOutit) {
                    if (uOutit->vertexID == vSCCID) {
                        uOutit->lifespan.set(ur.timestamp, false);
                        if (uOutit->lifespan.none()) {
                            urecord->Out.erase(uOutit);
                        }
                    }
                }
                for (auto vInit = vrecord->In.begin(); vInit != vrecord->In.end(); ++vInit) {
                    if (vInit->vertexID == uSCCID) {
                        vInit->lifespan.set(ur.timestamp, false);
                        if (vInit->lifespan.none()) {
                            vrecord->In.erase(vInit);
                        }
                    }
                }
                this->refineNITable = GetRefineNITable(nodeInfoTable);
                this->IG = BuildIndexGraph(this->refineNITable);
            }
            else {
                //在同一个SCC内部，先把SCC内部重新分割【
                auto findResult = find_if(sccGraph.sccGraphs[ur.timestamp - 1].second.begin(), sccGraph.sccGraphs[ur.timestamp - 1].second.end(),
                    [&uSCCID](SCCnode& sccnode) { return sccnode.SCCID == uSCCID; });
                set<int> nodeSet = findResult->originNodeSet;
                
                vector<SCCEdge> in;
                vector<SCCEdge> out;
                for (auto it = sccGraph.sccGraphs[ur.timestamp - 1].second.begin(); it != sccGraph.sccGraphs[ur.timestamp - 1].second.end(); ++it) {
                    if (it ->SCCID == uSCCID) {
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
                    VerNode& findRes = originGraph[ur.timestamp - 1].findNodeRefByID(*nodeit);
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
                newSCCTable = GetSCCTableFromOneGraph(ur.timestamp, &newGraph, newEvolvingGraph, newSCCEdgeInfo);
                auto findRes = find_if(sccTable.begin(), sccTable.end(), [&uSCCID](auto mapItem) { return mapItem.sccID_Life.scc_id == uSCCID; });
                if (findRes != sccTable.end()) {
                    findRes->sccID_Life.life_time.set(ur.timestamp, false);
                    if (findRes->sccID_Life.life_time.none()) sccTable.erase(findRes);
                }
                else assert(false);
                map<int, int> SCCIDmap;
                sccGraph.deleteNode(uSCCID, ur.timestamp);
                for (auto it = newSCCTable.begin(); it != newSCCTable.end(); ++it) {
                    auto res = sccTable.insert(*it);
                    if (!res.second) {
                        SCCIDmap[it->sccID_Life.scc_id] = res.first->sccID_Life.scc_id;
                        res.first->sccID_Life.life_time.set(ur.timestamp, true);
                    }
                    else {
                        int newid = newSCCID(this->sccTable);
                        res.first->sccID_Life.scc_id = newid;
                        sccGraph.addNode(newid, ur.timestamp, res.first->nodeGroup);
                        SCCIDmap[it->sccID_Life.scc_id] = newid;
                    }
                }
                //先加入内部的边
                for (auto it = newSCCEdgeInfo.begin(); it != newSCCEdgeInfo.end(); ++it) {
                    sccGraph.addEdge(SCCIDmap[it->first.sScc], SCCIDmap[it->first.tScc], ur.timestamp);
                }
                //将SCC的节点与外部相连
                for(auto it = in.begin(); it != in.end(); ++it) {
                    //todo
                }
                for (auto it = out.begin(); it != out.end(); ++it) {
                    auto dstSCCID = it->tScc;
                    auto srcSCCID = it->sScc;
                    auto sccEdgeInfo = sccEdgeInfoSequence[ur.timestamp - 1];
                    auto findResult = find_if(sccEdgeInfo.begin(), sccEdgeInfo.end(), [&](SCCEdgeInfoItem& item) {return item.first.sScc == srcSCCID && item.first.tScc == dstSCCID; });
                    if (findResult != sccEdgeInfo.end()) {
                        for (auto edgeit = findResult->second.begin(); edgeit != findResult->second.end(); ++edgeit) {
                            auto res = sccGraph.findSCCIDNodeFromOriginNodeID(edgeit->src, ur.timestamp);
                            assert(res != -1);
                            int newSCCSrcID = res;
                            //此边为newSCCSrcID->dstSCCID的边
                            sccGraph.addEdge(newSCCSrcID, dstSCCID, ur.timestamp);
                        }
                    }
                    else assert(false);
                }
            }
            break;
        }
        default: {
            printf("Error: update type error!\n");
            return false;
        }
        }
    }
    return true;
}

#endif