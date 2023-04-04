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
        else if(ur.type == 2){
            //加入一条边u->v
            originGraph[ur.timestamp].InsertEdge(ur.u, ur.v);
            int uSCCID = originGraph[ur.timestamp].findSCCIDFromNodeId(ur.u);
            int vSCCID = originGraph[ur.timestamp].findSCCIDFromNodeId(ur.v);
            if(uSCCID == vSCCID){
                //do nothing
            }
            else{


            }
        }
        else if(ur.type == 3){
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