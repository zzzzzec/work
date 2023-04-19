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
    int addNode(int SCCID, int timestamp, set<int> originNodeSet);
    int deleteNode(int SCCID, int timestamp);
    int addEdge(int srcID, int dstID, int timestamp);
    int deleteEdge(int srcID, int dstID, int timestamp);
    bool edgeExist(int srcID, int dstID, int timestamp);
    int getEdgeNum(int timeStamp);
    int findSCCIDNodeFromOriginNodeID(int originNodeID, int timestamp);
    void SCCGraphInsertArc(int srcID, int dstID, int timestamp, vector<SCCnode>& thisGraph);
    vector<int> findCycleAndMerge(int uSCCID, int timestamp);
    vector<SCCnode> findCycle(int SCCIDu, int timestamp);
    SCCnode findSCCnodeFromID(int SCCID, int timestamp);
    bool SCCIDexist(int SCCID, int timestamp);
    int merge1(const vector<SCCnode>& cycle, int timestamp);
    int merge2(const vector<SCCnode>& cycle, int timestamp, SccTable& sccTable);
    int newNodeID(int timestamp);
    ~SCCGraph();
};

SCCGraph::SCCGraph() {}
SCCGraph::~SCCGraph() {}
SCCGraph::SCCGraph(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable){
    vector<SCCnode> thisGraph;
    int timeStamp = 0;
    for(auto it = evolvingGraphSequence.begin(); it != evolvingGraphSequence.end(); it++){
        thisGraph.clear();
        //先吧SCC节点加入进来
        for(auto sccit = sccTable.begin(); sccit != sccTable.end(); sccit++){
            SCCnode newNode;
            if(sccit->sccID_Life.life_time.test(timeStamp)){
                newNode.originNodeSet = sccit->nodeGroup;
                newNode.SCCID = sccit->sccID_Life.scc_id;
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
    for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
        if(it->SCCID == SCCID){
            return 1;
        }
    }
    return 0;
}

SCCnode SCCGraph::findSCCnodeFromID(int SCCID, int timestamp){
    auto it = sccGraphs[timestamp].second.begin();
    for(; it != sccGraphs[timestamp].second.end(); it++){
        if(it->SCCID == SCCID){
            return *it;
        }
    }
    throw "scc node not found";
}

int SCCGraph::findSCCIDNodeFromOriginNodeID(int originNodeID, int timestamp){
    for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
        if (it->originNodeSet.find(originNodeID) != it->originNodeSet.end()) {
            return it->SCCID;
        }
    }
    return -1;
}

bool SCCGraph::SCCIDexist(int SCCID, int timestamp) {
    for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
        if(it->SCCID == SCCID){
            return true;
        }
    }
    return false;
}

bool SCCGraph::edgeExist(int srcID, int dstID, int timestamp){
    for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
        if(it->SCCID == srcID){
            arc *tmp = it->firstArc;
            while(tmp != NULL){
                if(tmp->dstID == dstID){
                    return true;
                }
                tmp = tmp->next;
            }
        }
    }
    return false;
}

int SCCGraph::addNode(int SCCID, int timestamp, set<int> originNodeSet) {
    if(this->IDexist(SCCID, timestamp)){
        return 0;
    }
    else{
        SCCnode newNode;
        newNode.SCCID = SCCID;
        newNode.firstArc = NULL;
        newNode.originNodeSet = originNodeSet;
        sccGraphs[timestamp].second.push_back(newNode);
        return 1;
    }
}

int SCCGraph::addEdge(int srcID, int dstID, int timestamp) {
    if (srcID == dstID) return 0;
    if (this->edgeExist(srcID, dstID, timestamp)) return 1;
    if (this->IDexist(srcID, timestamp) && this->IDexist(dstID, timestamp)) {
        arc* newArc = new arc;
        newArc->dstID = dstID;
        //优化一下
        for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
            if(it->SCCID == srcID){
                newArc->next = it->firstArc;
                it->firstArc = newArc;
                break;
            }
        }
        return 1;
    }
    else return 0;
}

int SCCGraph::deleteEdge(int srcID, int dstID, int timestamp) {
    if (!this->edgeExist(srcID, dstID, timestamp))
        throw "deleteEdge: edge not exist";
    for (auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++) {
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
        }
    }
    throw "deleteEdge: edge not exist";
}

int SCCGraph::deleteNode(int SCCID, int timestamp) {
    if(!this->IDexist(SCCID, timestamp)){
        return 0;
    }
    for (auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++) {
        if(it->SCCID == SCCID){
            sccGraphs[timestamp].second.erase(it);
            it = sccGraphs[timestamp].second.begin();
        }
        else {
            arc *tmp = it->firstArc;
            arc *pre = NULL;
            while (tmp != NULL) {
                if (tmp->dstID == SCCID) {
                    if (pre == NULL) {
                        it->firstArc = tmp->next;
                    } else {
                        pre->next = tmp->next;
                    }
                    delete tmp;
                    break;
                }
                pre = tmp;
                tmp = tmp->next;
            }
        }
    }
    return 1;
}

int SCCGraph::newNodeID(int timestamp){
    int maxID = 0;
    for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
        if(it->SCCID > maxID){
            maxID = it->SCCID;
        }
    }
    return maxID + 1;
}

vector<SCCnode> SCCGraph::findCycle(int SCCIDu, int timestamp) {
    //这里检测环，然后把检测到的环都按照第一个的ID合并，因为SRC->DST，所以如果有环，那么一定包含这两个节点
    vector<SCCnode> cycle;
    map<int, bool> visited;
    for(auto it = sccGraphs[timestamp].second.begin(); it != sccGraphs[timestamp].second.end(); it++){
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

int sccIncycle(int SCCID, const vector<SCCnode> &cycle){
    for(auto it = cycle.begin(); it != cycle.end(); it++){
        if(it->SCCID == SCCID){
            return 1;
        }
    }
    return 0;
}
//仅更新SCCGraph
int SCCGraph::merge1(const vector<SCCnode>& cycle, int timestamp) {
        SCCnode newNode;
    //合并后的节点ID不可以重用，因为代码中的逻辑是SCCID对应唯一的原始图节点集合，这个集合是全局的
    //所以这里需要重新分配一个ID
    newNode.SCCID = newNodeID(timestamp);
    newNode.firstArc = NULL;
    arc* arcit;
    auto it = sccGraphs[timestamp].second.begin();
    while(it != sccGraphs[timestamp].second.end()){
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
            it = sccGraphs[timestamp].second.erase(it);
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
    sccGraphs[timestamp].second.push_back(newNode);
    return newNode.SCCID;
}

//更新SCCGraph和SccTable
int SCCGraph::merge2(const vector<SCCnode>& cycle, int timestamp, SccTable& sccTable) {
    SCCnode newNode;
    //合并后的节点ID不可以重用，因为代码中的逻辑是SCCID对应唯一的原始图节点集合，这个集合是全局的
    //所以这里需要重新分配一个ID
    newNode.SCCID = newSCCID(sccTable);
    newNode.firstArc = NULL;
    arc* arcit;
    auto it = sccGraphs[timestamp].second.begin();
    while(it != sccGraphs[timestamp].second.end()){
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
            it = sccGraphs[timestamp].second.erase(it);
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
    sccGraphs[timestamp].second.push_back(newNode);

    //更新sccTable
    auto sccTableit = sccTable.begin();
    //删除合并前的SCC集合cycle
    while (sccTableit != sccTable.end())
    {
        if (sccTableit->sccID_Life.life_time.test(timestamp) && sccIncycle(sccTableit->sccID_Life.scc_id, cycle)) {
            sccTableit->sccID_Life.life_time.set(timestamp, false);
            sccTableit = sccTableit->sccID_Life.life_time.none() ? sccTable.erase(sccTableit) : sccTableit++;
        }
        else{
            sccTableit++;
        }
    }
    SCCTableItem newitem;
    newitem.nodeGroup = newNode.originNodeSet;
    auto res = sccTable.insert(newitem);
    if(!res.second){
        res.first->sccID_Life.life_time.set(timestamp, true);
    }
    else {
        SccID_Life newlife;
        newlife.scc_id = newNode.SCCID;
        bitset<MNS> newlifespan;
        newlife.life_time = LifespanBuild(newlifespan, timestamp, timestamp);
        res.first->sccID_Life = newlife;
    }
    return newNode.SCCID;
}

vector<int> SCCGraph::findCycleAndMerge(int uSCCID, int timestamp) {
    int cycleNum = 0;
    vector<int> effectedSCC;
    map<int, int> tree;
    vector<SCCnode> cycle = findCycle(uSCCID, timestamp);
    while (cycle.size() != 0) {
        //考虑合并后的SCC与其他SCC相同
        int newID = merge1(cycle, timestamp);
        for (auto it = cycle.begin(); it != cycle.end(); it++) {
            tree[it->SCCID] = newID;
        }
        tree[newID] = -1;
        cycle = findCycle(newID, timestamp);
        //新产生的SCC必定包含了原本的SCCid
        assert(sccIncycle(newID, cycle) == 1);
        cycleNum++;
    }
}

static int _getEdgeNumOneNode(SCCnode& node) {
    auto tmp = node.firstArc;
    int num = 0;
    while (tmp != NULL) {
        num++;
        tmp = tmp->next;
    }
    return num;
}

int SCCGraph::getEdgeNum(int timestamp) {
    vector<SCCnode>& vertices = sccGraphs[timestamp].second;
    int num = 0;
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        num += _getEdgeNumOneNode(*it);
    }
    return num;
}
#endif