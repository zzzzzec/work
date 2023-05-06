#ifndef IG_NOOP_5_SCCGRAPH_H
#define IG_NOOP_5_SCCGRAPH_H
#include "common.h"
#include "Lifespan.h"
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
    vector<SCCnode> vertices;
    int timestamp;
    SCCGraph();
    SCCGraph(vector<int>& evolvingGraph, SccTable& sccTable, int timeStamp);
    ~SCCGraph();
    int IDexist(int SCCID);
    int addNode(int SCCID, set<int> originNodeSet);
    int deleteNode(int SCCID);
    int addEdge(int srcID, int dstID);
    int deleteEdge(int srcID, int dstID);
    bool edgeExist(int srcID, int dstID);
    int getEdgeNum();
    int findSCCIDNodeFromOriginNodeID(int originNodeID);
    void SCCGraphInsertArc(int srcID, int dstID, vector<SCCnode>& thisGraph);
    tuple<vector<int>, int> findCycleAndMerge(int uSCCID, SccTable& sccTable, NodeInfoTable nit);
    SCCnode findSCCnodeFromID(int SCCID);
    
    int merge(const vector<SCCnode>& cycle, SccTable& sccTable);
    vector<SCCnode> findCycle(int SCCIDu);
    pair<int, vector<SCCnode>> findCycles(int SCCIDu, SccTable& st);
    int newNodeID();
    pair<vector<int>, vector<int>> getInAndOutNodes(int SCCID);
    void storeSCCGraphJSON(string path);
};

typedef vector<SCCGraph> SCCGraphs;
SCCGraphs BuildSCCGraphs(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable, int timeIntervalLength) {
    SCCGraphs sccGraphs;
    for (int i = 0; i < timeIntervalLength; i++) {
        SCCGraph sccGraph(evolvingGraphSequence[i], sccTable, i);
        sccGraphs.push_back(sccGraph);
    }
    return sccGraphs;
}
SCCGraph::SCCGraph() {}
SCCGraph::~SCCGraph() {}
SCCGraph::SCCGraph(vector<int>& evolvingGraph, SccTable& sccTable, int ts) {
    timestamp = ts;
    for (auto sccit = sccTable.begin(); sccit != sccTable.end(); sccit++) {
        SCCnode newNode;
        if (sccit->sccID_Life.life_time.test(timestamp)) {
            newNode.originNodeSet = sccit->nodeGroup;
            newNode.SCCID = sccit->sccID_Life.scc_id;
            newNode.firstArc = NULL;
            vertices.push_back(newNode);
        }
    }
    int srcID, dstID;
    for (auto it2 = evolvingGraph.begin(); it2 != evolvingGraph.end(); it2 += 2) {
        srcID = *it2;
        dstID = *(it2 + 1);
        arc* newArc = new arc;
        newArc->dstID = dstID;

        bool found = false;
        for (auto it = vertices.begin(); it != vertices.end(); it++) {
            if (it->SCCID == srcID) {
                newArc->next = it->firstArc;
                it->firstArc = newArc;
                found = true;
                break;
            }
        }
        assert(found);
    }
}

int SCCGraph::IDexist(int SCCID){
    for(auto it = vertices.begin(); it != vertices.end(); it++){
        if(it->SCCID == SCCID){
            return 1;
        }
    }
    return 0;
}

SCCnode SCCGraph::findSCCnodeFromID(int SCCID) {
    auto it = vertices.begin();
    for(; it != vertices.end(); it++){
        if(it->SCCID == SCCID){
            return *it;
        }
    }
    throw "scc node not found";
}

int SCCGraph::findSCCIDNodeFromOriginNodeID(int originNodeID){
    for(auto it = vertices.begin(); it != vertices.end(); it++){
        if (it->originNodeSet.find(originNodeID) != it->originNodeSet.end()) {
            return it->SCCID;
        }
    }
    return -1;
}

bool SCCGraph::edgeExist(int srcID, int dstID){
    for(auto it = vertices.begin(); it != vertices.end(); it++){
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

int SCCGraph::addNode(int SCCID, set<int> originNodeSet) {
    if(this->IDexist(SCCID)){
        return 0;
    }
    else{
        SCCnode newNode;
        newNode.SCCID = SCCID;
        newNode.firstArc = NULL;
        newNode.originNodeSet = originNodeSet;
        vertices.push_back(newNode);
        return 1;
    }
}

int SCCGraph::addEdge(int srcID, int dstID) {
    if (srcID == dstID) return 0;
    if (this->edgeExist(srcID, dstID)) return 1;
    if (this->IDexist(srcID) && this->IDexist(dstID)) {
        arc* newArc = new arc;
        newArc->dstID = dstID;
        //优化一下
        for(auto it = vertices.begin(); it != vertices.end(); it++){
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

int SCCGraph::deleteEdge(int srcID, int dstID) {
    if (!this->edgeExist(srcID, dstID))
        throw "deleteEdge: edge not exist";
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
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

int SCCGraph::deleteNode(int SCCID) {
    if(!this->IDexist(SCCID)){
        return 0;
    }
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        if(it->SCCID == SCCID){
            vertices.erase(it);
            it = vertices.begin();
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

int SCCGraph::newNodeID(){
    int maxID = 0;
    for(auto it = vertices.begin(); it != vertices.end(); it++){
        if(it->SCCID > maxID){
            maxID = it->SCCID;
        }
    }
    return maxID + 1;
}

vector<SCCnode> SCCGraph::findCycle(int SCCIDu) {
    //这里检测环，然后把检测到的环都按照第一个的ID合并，因为SRC->DST，所以如果有环，那么一定包含这两个节点
    vector<SCCnode> cycle;
    map<int, bool> visited;
    for(auto it = vertices.begin(); it != vertices.end(); it++){
        visited.insert(make_pair(it->SCCID, false));
    }
    stack<SCCnode> s;
    s.push(findSCCnodeFromID(SCCIDu));
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
                s.push(findSCCnodeFromID(it->dstID));
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

//更新SCCGraph和SccTable
int SCCGraph::merge(const vector<SCCnode>& cycle, SccTable& sccTable) {
    SCCnode newNode;
    //合并后的节点ID不可以重用，因为代码中的逻辑是SCCID对应唯一的原始图节点集合，这个集合是全局的
    //所以这里需要重新分配一个ID
    //首先需要确定合并后的新节点是否已经存在于SCCTable
    newNode.originNodeSet.clear();
    for (auto it : vertices) {
        if (sccIncycle(it.SCCID, cycle)) {
            set_union(  newNode.originNodeSet.begin(), newNode.originNodeSet.end(), 
                        it.originNodeSet.begin(), it.originNodeSet.end(), 
                        inserter(newNode.originNodeSet, newNode.originNodeSet.begin()));
        }
    }
    auto findRes = find_if(sccTable.begin(), sccTable.end(), [&](const SCCTableItem& item) {return item.nodeGroup == newNode.originNodeSet;});
    if (findRes != sccTable.end()) {
        newNode.SCCID = findRes->sccID_Life.scc_id;
        assert(!findRes->sccID_Life.life_time.test(timestamp));
        findRes->sccID_Life.life_time.set(timestamp);
    }
    else {
        newNode.SCCID = newSCCID(sccTable);
        SCCTableItem newitem;
        newitem.nodeGroup = newNode.originNodeSet;
        SccID_Life newlife;
        newlife.scc_id = newNode.SCCID;
        bitset<MNS> newlifespan;
        newlife.life_time = LifespanBuild(newlifespan, timestamp, timestamp);
        newitem.sccID_Life = newlife;
        newitem.SCCID = newNode.SCCID;
        newitem.lifeSpan = newlifespan;
        auto insertRes = sccTable.insert(newitem);
        assert(insertRes.second);
    }
    //删除合并前的SCC集合cycle
    auto sccTableit = sccTable.begin();
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
    
    newNode.firstArc = NULL;
    auto it = vertices.begin();
    while(it != vertices.end()){
        if(sccIncycle(it->SCCID, cycle)){
            //cycle中的节点，需要合并出边
            Arc* current = it->firstArc;
            while (current != NULL) {
                if (!sccIncycle(current->dstID, cycle)) {
                    //插入前检查
                    bool find = false;
                    arc* check = newNode.firstArc;
                    while (check != NULL) {
                        if (check->dstID == current->dstID) {
                            find = true;
                            break;
                        }
                        check = check->next;
                    }
                    if (!find) {
                        arc* newarc = new arc;
                        newarc->dstID = current->dstID;
                        newarc->next = newNode.firstArc;
                        newNode.firstArc = newarc;
                    }
                }
                current = current->next;
            }
            it = vertices.erase(it);
        }
        else{
            //其他节点，需要把出边的目的节点改为新的SCCID
            arc* prev = nullptr;
            arc* current = it->firstArc;
            bool flag = false;
            while (current != nullptr) {
                if (sccIncycle(current->dstID, cycle)) {
                    flag = true;
                    if (prev == nullptr) {
                        it->firstArc = current->next;
                    }
                    else {
                        prev->next = current->next;
                    }
                    arc* temp = current;
                    current = current->next;
                    delete temp;
                }
                else {
                    prev = current;
                    current = current->next;
                }
            }
            //再把新节点加上, 头插法
            if (flag) {
                arc* newArc = new arc;
                newArc->dstID = newNode.SCCID;
                newArc->next = it->firstArc;
                it->firstArc = newArc;
            }
            it++;
        }
    }
    vertices.push_back(newNode);
    return newNode.SCCID;
}

pair<int, vector<SCCnode>> SCCGraph::findCycles(int SCCIDu, SccTable& st) {
    vector<SCCnode> all;
    int oldid, newid;
    int cycleNum = 0;
    auto cycle = findCycle(SCCIDu);
    while (cycle.size() != 0) {
        cycleNum++;
        newid = merge(cycle, st);
        vector<int> cycint;
        cycint.resize(cycle.size());
        transform(cycle.begin(), cycle.end(), cycint.begin(),
            [](const SCCnode& obj) {return obj.SCCID;});
        
        LOG << "cycleNum = " << cycleNum << endl;
        LOG << "cycle : " << vec2string(cycint) << " -> " << newid << endl;
        string p = "./scc" + to_string(cycleNum) + ".json";
        storeSCCGraphJSON(p);
        if (all.size() == 0) {
            all.insert(all.end(), cycle.begin(), cycle.end());
            oldid = newid;
        }
        else {
            auto findres = find_if(cycle.begin(), cycle.end(), [&](const SCCnode& it) {return it.SCCID == oldid;});
            //如果还有环，那么必定包含之前得到的节点
            assert(findres != cycle.end());
            for (auto it : cycle) {
                if (it.SCCID != oldid) {
                    all.push_back(it);
                }
            }
            oldid = newid;
        }
        cycle = findCycle(newid);
    }
    vector<int> allint;
    allint.resize(all.size());
    transform(all.begin(), all.end(), allint.begin(),
        [](const SCCnode& obj) {return obj.SCCID;});
    LOG << "All : " << vec2string(allint) << endl;
    return make_pair(newid, all);
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

int SCCGraph::getEdgeNum() {
    int num = 0;
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        num += _getEdgeNumOneNode(*it);
    }
    return num;
}

pair<vector<int>, vector<int>> SCCGraph::getInAndOutNodes(int SCCID) {
    SCCnode node = findSCCnodeFromID(SCCID);
    vector<int> in;
    vector<int> out;
    auto edgeit = node.firstArc;
    while (edgeit != NULL)
    {
        out.push_back(edgeit->dstID);
        edgeit = edgeit->next;
    }
    for (auto it : vertices) {
        auto it2 = it.firstArc;
        while (it2 != NULL)
        {
            if (it2->dstID == SCCID) {
                in.push_back(it.SCCID);
                break;
            }
            it2 = it2->next;
        }
    }
    return make_pair(in, out);
}

void SCCGraph::storeSCCGraphJSON(string path) {
    Json::Value JsonGraph;
    for (auto node : vertices) {
        Json::Value JsonNode;
        JsonNode["ID"] = node.SCCID;
        JsonNode["time"] = timestamp;
        Json::Value JsonArcs;
        Json::Value nodeset;
        for (auto i : node.originNodeSet) {
            Json::Value n;
            n["originID"] = i;
            nodeset.append(n);
        }
        JsonNode["originNodeSet"] = nodeset;
        for (arc* p = node.firstArc; p; p = p->next) {
            Json::Value JsonArc;
            JsonArc["dstID"] = p->dstID;
            JsonArcs.append(JsonArc);
        }
        JsonNode["arcs"] = JsonArcs;
        JsonGraph.append(JsonNode);
    }
    ofstream fout(path);
    if (fout) {
        fout << JsonGraph.toStyledString();
    }
    else {
        cout << __FUNCTION__ <<"open file error" << endl;
    }
}

#endif