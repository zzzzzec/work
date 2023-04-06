#ifndef IG_NOOP_5_HRINDEX_H
#define IG_NOOP_5_HRINDEX_H
#include "common.h"
#include "Lifespan.h"
#include "Graph.h"
#include "SCC_Table.h"
#include "Process_Snapshots.h"
#include "Construct_NITable.h"
#include "IndexGraph.h"
#include "Process_Query.h"
#include "update.h"
#include "HRindex.h"
#include "SCCGraph.h"
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
    bool reconstructEvolvingGraphSequence(SCCGraph &sccGraph, int timestamp);
    vector<SCCnode> findCycle();
};

HRindex::HRindex(/* args */)
{
}

HRindex::~HRindex()
{
}

bool HRindex::buildOriginGraph(){
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
    this->sccTable = GetSCCTable(timeIntervalLength, originGraph, evolvingGraphSequence, buildSccTableTime);
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

bool HRindex::getNITable(){
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

bool HRindex::getRefineNITable(){
    buildRefineNIT_startTime = clock();
    refineNITable = GetRefineNITable(nodeInfoTable);
    buildRefineNIT_endTime = clock();
    buildRefineNIT_time = (double)(buildRefineNIT_endTime - buildRefineNIT_startTime) / CLOCKS_PER_SEC;
    printf("build RefineNITable time: %f\n", buildRefineNIT_time);
    return true;
}

bool HRindex::stroreRefineNITable(){
    StoreRefineNITable(storeRefineNITableAddress, refineNITable);
    return true;
}

bool HRindex::buildIndexGraph(){
    buildIG_startTime = clock();
    IG = BuildIndexGraph(refineNITable);
    buildIG_endTime = clock();
    buildIG_time = (double)(buildIG_endTime - buildIG_startTime) / CLOCKS_PER_SEC;
    printf("build IndexGraph time: %f\n", buildIG_time);
    return true;
}

bool HRindex::stroreIndexGraph(){
    IG.StoreFullIndexGraph(storeFull_IG_Address);
    return true;
}


bool HRindex::update(){
    readUpdateRecords(updateRecordVector, updateFileAddress);
    singleStepUpdate();
    return true;
}

int HRindex::getaNewSCCID(){
    int maxSccid = sccTable.begin()->second.scc_id;
    // trasverse the sccTable to find the new sccid
    for (auto it = sccTable.begin(); it != sccTable.end(); ++it) {
        if(it->second.scc_id > maxSccid){
            maxSccid = it->second.scc_id;
        }
    }
    return maxSccid + 1;
}

bool HRindex::addSCCnode(int nodeID, int newSCCID, Lifespan lifespan){
    SccID_Life newSccID_Life;
    newSccID_Life.scc_id = newSCCID;
    newSccID_Life.life_time = lifespan;
    sccTable.insert(pair<set<int>, SccID_Life>(set<int>{nodeID}, newSccID_Life));
    return true;
}

bool HRindex::reconstructEvolvingGraphSequence(SCCGraph &sccGraph, int timestamp){
    vector<int> tmp;
    for (auto it = sccGraph.sccGraphs.begin(); it != sccGraph.sccGraphs.end(); ++it)
    {
        if(it->first == timestamp){
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


bool HRindex::singleStepUpdate(){
    for (int i = 0; i < updateRecordVector.size(); ++i) {
        updateRecord ur = updateRecordVector[i];
        if(ur.type == 1){
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
        }
        else if(ur.type == 3){
            //加入一条边u->v
            originGraph[ur.timestamp - 1].InsertEdge(ur.u, ur.v);
            int uSCCID = originGraph[ur.timestamp - 1].findSCCIDFromNodeId(ur.u);
            int vSCCID = originGraph[ur.timestamp - 1].findSCCIDFromNodeId(ur.v);
            if(uSCCID != vSCCID){
                //循环检测环，直到没有环为止
                sccGraph.addEdge(uSCCID, vSCCID, ur.timestamp);
                vector<SCCnode> cycle = sccGraph.findCycle(uSCCID, ur.timestamp);
                while(cycle.size() != 0){
                    int reusedID = sccGraph.merge(cycle, ur.timestamp, sccTable);
                    cycle = sccGraph.findCycle(reusedID, ur.timestamp);
                }
                reconstructEvolvingGraphSequence(sccGraph, ur.timestamp);

                int id;
                auto exist = [&id](RecordItem &ri){return ri.node == id;};
                for (   auto sccGraphit = sccGraph.sccGraphs[ur.timestamp - 1].second.begin(); 
                        sccGraphit != sccGraph.sccGraphs[ur.timestamp - 1].second.end(); ++sccGraphit){
                    id = sccGraphit->SCCID;
                    vector<int> INlist;
                    vector<int> OUTlist;
                    //遍历整个图来找到这个SCCID的入边
                    for (   auto INlistit = sccGraph.sccGraphs[ur.timestamp - 1].second.begin(); 
                            INlistit != sccGraph.sccGraphs[ur.timestamp - 1].second.end(); ++INlistit){
                        if(INlistit->SCCID == id) continue;
                        else{
                            for (auto arcit = INlistit->firstArc; arcit != NULL; arcit = arcit->next)
                            {
                                if(arcit->dstID == id){
                                    INlist.push_back(INlistit->SCCID);
                                    break;
                                }
                            }
                        }
                    }
                    //找到这个SCCID的出边
                    for (auto outArcit = sccGraphit->firstArc; outArcit != NULL; outArcit = outArcit->next)
                    {
                        OUTlist.push_back(outArcit->dstID);
                    }

                    auto record = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), exist);
                    //先把timeStamp时刻的记录全部删掉
                    for (auto INit = record->In.begin(); INit != record->In.end(); ++INit)
                    {
                        
                    }
                    for (auto OUTit = record->Out.begin(); OUTit != record->Out.end(); ++OUTit)
                    {
                    }
                }
            }
        }
        else if(ur.type == 2){

        }
        else if(ur.type == 4){
        }
        else{
            printf("Error: update type error!\n");
            return false;
        }
    }
    return true;
}

#endif