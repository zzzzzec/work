#ifndef IG_NOOP_5_SCCGRAPH_H
#define IG_NOOP_5_SCCGRAPH_H
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

using namespace std;

// 一个图数据结构，使用邻接表存储
typedef struct Arc{
    int dstID;
    struct Arc *next;
} arc;

typedef struct {
    int SCCID;
    arc *firstArc;
} SCCnode;

class SCCGraph
{
private:
    vector<pair<int, vector<SCCnode>>> sccGraphs;

public:
    SCCGraph(/* args */);
    SCCGraph(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable);
    void SCCGraphInsertArc(int srcID, int dstID, int timestamp, vector<SCCnode>& thisGraph);
    vector<SCCnode> findCycle(int SCCIDu, int SCCIDv, int timestamp);
    SCCnode findSCCnodeFromID(int SCCID, int timestamp);
    ~SCCGraph();
};

SCCGraph::SCCGraph(/* args */)
{

}

SCCGraph::~SCCGraph()
{
}

SCCGraph::SCCGraph(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable){
    vector<SCCnode> thisGraph;
    int timeStamp = 1;
    for(auto it = evolvingGraphSequence.begin(); it != evolvingGraphSequence.end(); it++){
        thisGraph.clear();
        //先吧SCC节点加入进来
        for(auto sccit = sccTable.begin(); sccit != sccTable.end(); sccit++){
            SCCnode newNode;
            if(sccit->second.life_time.test(timeStamp)){
                newNode.SCCID = sccit->second.scc_id;
                newNode.firstArc = NULL;
                thisGraph.push_back(newNode);
            }
        }
        //然后插入边
        int srcID, dstID;
        for(auto it2 = it->begin(); it2 != it->end(); it2 += 2){
            srcID = *it2;
            dstID = *(it2 + 1);
            arc *newArc = new arc;
            newArc->dstID = dstID;

            for(auto it = thisGraph.begin(); it != thisGraph.end(); it++){
                if(it->SCCID == srcID){
                    newArc->next = it->firstArc;
                    it->firstArc = newArc;
                    break;
                }
            }
        }
        sccGraphs.push_back(make_pair(timeStamp, thisGraph));
        timeStamp++;
    }
}

SCCnode SCCGraph::findSCCnodeFromID(int SCCID, int timestamp){
    for(auto it = sccGraphs[timestamp-1].second.begin(); it != sccGraphs[timestamp-1].second.end(); it++){
        if(it->SCCID == SCCID){
            return *it;
        }
    }
}

vector<SCCnode> SCCGraph::findCycle(int SCCIDu, int SCCIDv, int timestamp){
    vector<SCCnode> cycle;
    map<int, bool> visited;
    for(auto it = sccGraphs[timestamp-1].second.begin(); it != sccGraphs[timestamp-1].second.end(); it++){
        visited.insert(make_pair(it->SCCID, false));
    }

    stack<SCCnode> s;
    s.push(findSCCnodeFromID(SCCIDu, timestamp));
    while (!s.empty())
    {
        SCCnode tmp = s.top();
        s.pop();
        if(tmp.SCCID == SCCIDv){
            while (!s.empty())
            {
                SCCnode tmp2 = s.top();
                s.pop();
                cycle.push_back(tmp2);
            }
            break;
        }
        if(visited[tmp.SCCID] == false){
            visited[tmp.SCCID] = true;
            }
        }
    }
    
}

#endif