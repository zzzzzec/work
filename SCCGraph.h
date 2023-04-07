#ifndef IG_NOOP_5_SCCGRAPH_H
#define IG_NOOP_5_SCCGRAPH_H
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
#include "NIT.h"

using namespace std;

// 一个图数据结构，使用邻接表存储
typedef struct Arc{
    int dstID;
    struct Arc *next;
} arc;

typedef struct {
    set<int> originNodeSet;
    int SCCID;
    arc *firstArc;
} SCCnode;

class SCCGraph
{
public:
    vector<pair<int, vector<SCCnode>>> sccGraphs;
    SCCGraph();
    SCCGraph(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable);
    int IDexist(int SCCID, int timestamp);
    int addEdge(int srcID, int dstID, int timestamp);
    int deleteEdge(int srcID, int dstID, int timestamp);
    void SCCGraphInsertArc(int srcID, int dstID, int timestamp, vector<SCCnode>& thisGraph);
    vector<SCCnode> findCycle(int SCCIDu, int timestamp);
    SCCnode findSCCnodeFromID(int SCCID, int timestamp);
    bool SCCIDexist(int SCCID, int timestamp);
    int merge(vector<SCCnode> &cycle, int timestamp, SccTable &sccTable);
    ~SCCGraph();
};

SCCGraph::SCCGraph() {}
SCCGraph::~SCCGraph() {}
SCCGraph::SCCGraph(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable){
    vector<SCCnode> thisGraph;
    int timeStamp = 1;
    for(auto it = evolvingGraphSequence.begin(); it != evolvingGraphSequence.end(); it++){
        thisGraph.clear();
        //先吧SCC节点加入进来
        for(auto sccit = sccTable.begin(); sccit != sccTable.end(); sccit++){
            SCCnode newNode;
            if(sccit->second.life_time.test(timeStamp)){
                newNode.originNodeSet = sccit->first;
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

int SCCGraph::IDexist(int SCCID, int timestamp){
    for(auto it = sccGraphs[timestamp-1].second.begin(); it != sccGraphs[timestamp-1].second.end(); it++){
        if(it->SCCID == SCCID){
            return 1;
        }
    }
    return 0;
}

SCCnode SCCGraph::findSCCnodeFromID(int SCCID, int timestamp){
    auto it = sccGraphs[timestamp - 1].second.begin();
    for(; it != sccGraphs[timestamp-1].second.end(); it++){
        if(it->SCCID == SCCID){
            return *it;
        }
    }
    return *it;
}

bool SCCGraph::SCCIDexist(int SCCID, int timestamp){
    for(auto it = sccGraphs[timestamp-1].second.begin(); it != sccGraphs[timestamp-1].second.end(); it++){
        if(it->SCCID == SCCID){
            return true;
        }
    }
    return false;
}

int SCCGraph::addEdge(int srcID, int dstID, int timestamp){
    if(this->IDexist(srcID, timestamp) && this->IDexist(dstID, timestamp)){
        arc* newArc = new arc;
        newArc->dstID = dstID;
        //优化一下
        for(auto it = sccGraphs[timestamp-1].second.begin(); it != sccGraphs[timestamp-1].second.end(); it++){
            if(it->SCCID == srcID){
                newArc->next = it->firstArc;
                it->firstArc = newArc;
                break;
            }
        }
        return 1;
    }
    else
        return 0;
}

int SCCGraph::deleteEdge(int srcID, int dstID, int timestamp){
    for (auto it = sccGraphs[timestamp - 1].second.begin(); it != sccGraphs[timestamp - 1].second.end(); it++) {
        if (it->SCCID == srcID) {
            arc *tmp = it->firstArc;
            arc *pre = NULL;
            while (tmp != NULL) {
                if (tmp->dstID == dstID) {
                    if (pre == NULL) {
                        it->firstArc = tmp->next;
                    } else {
                        pre->next = tmp->next;
                    }
                    delete tmp;
                    return 1;
                }
                pre = tmp;
                tmp = tmp->next;
            }
            printf("delete edge error\n");
        }
    }
    return 1;
}

vector<SCCnode> SCCGraph::findCycle(int SCCIDu, int timestamp){
    //这里检测环，然后把检测到的环都按照第一个的ID合并，因为时SRC->DST，所以如果有环，那么一定包含这两个节点
    vector<SCCnode> cycle;
    map<int, bool> visited;
    for(auto it = sccGraphs[timestamp-1].second.begin(); it != sccGraphs[timestamp-1].second.end(); it++){
        visited.insert(make_pair(it->SCCID, false));
    }

    stack<SCCnode> s;
    s.push(findSCCnodeFromID(SCCIDu, timestamp));
    SCCnode* tmp = NULL;
    arc *it = NULL;
    SCCnode* neighbor = NULL;
    while (!s.empty())
    {
        tmp = &(s.top());
        it = tmp -> firstArc;
        //it是邻居
        while(it != NULL){
            if(it->dstID == SCCIDu){
                while(!s.empty()){
                    cycle.push_back(s.top());
                    s.pop();
                }
                return cycle;
            }
            if(!visited[it->dstID]){
                //有一个没有访问过，就把它加入栈中
                visited[it->dstID] = true;
                s.push(findSCCnodeFromID(it->dstID, timestamp));
                break;
            }
            else{
                it = it->next;
            }
        }
        if(it == NULL){
            //没有合适的邻居，就弹出
            s.pop();
        }
    }
    return cycle;
}

int sccIncycle(int SCCID, vector<SCCnode> &cycle){
    for(auto it = cycle.begin(); it != cycle.end(); it++){
        if(it->SCCID == SCCID){
            return 1;
        }
    }
    return 0;
}

int SCCGraph::merge(vector<SCCnode> &cycle, int timestamp, SccTable &sccTable){
    SCCnode newNode;
    //合并后的节点ID不可以重用，因为代码中的逻辑是SCCID对应唯一的原始图节点集合，这个集合是全局的
    //所以这里需要重新分配一个ID
    newNode.SCCID = newSCCID(sccTable);
    newNode.firstArc = NULL;
    arc* arcit;
    auto it = sccGraphs[timestamp - 1].second.begin();
    while(it != sccGraphs[timestamp-1].second.end()){
        arcit = it->firstArc;
        if(sccIncycle(it->SCCID, cycle)){
            //cycle中的节点，需要合并出边
            while(arcit != NULL){
                if(!sccIncycle(arcit->dstID, cycle)){
                    arc* newarc = new arc;
                    newarc->dstID = arcit->dstID;
                    newarc->next = newNode.firstArc;
                    newNode.firstArc = newarc;
                }
                arcit = arcit->next;
            }
            set_union(  newNode.originNodeSet.begin(), newNode.originNodeSet.end(), 
                        it->originNodeSet.begin(), it->originNodeSet.end(), 
                        inserter(newNode.originNodeSet, newNode.originNodeSet.begin()));
            it = sccGraphs[timestamp - 1].second.erase(it);
        }
        else{
            //其他节点，需要把出边的目的节点改为新的SCCID
            while(arcit != NULL){
                if(sccIncycle(arcit->dstID, cycle)){
                    arcit -> dstID = newNode.SCCID;
                }
                arcit = arcit->next;
            }
            it++;
        }
    }
    sccGraphs[timestamp - 1].second.push_back(newNode);

    //更新sccTable
    auto sccTableit = sccTable.begin();
    while (sccTableit != sccTable.end())
    {
        if (sccTableit->second.life_time.test(timestamp) && sccIncycle(sccTableit->second.scc_id, cycle)) {
            sccTableit->second.life_time.set(timestamp, false);
            sccTableit = sccTableit->second.life_time.none() ? sccTable.erase(sccTableit) : sccTableit++;
        }
        else{
            sccTableit++;
        }
    }
    SccID_Life newlife;
    newlife.scc_id = newNode.SCCID;
    bitset<MNS> newlifespan;
    newlife.life_time = LifespanBuild(newlifespan, timestamp, timestamp);
    sccTable.insert(make_pair(newNode.originNodeSet, newlife));

    return newNode.SCCID;
}

#endif