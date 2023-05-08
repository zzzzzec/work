#ifndef IG_NOOP_5_SCCGRAPH_H
#define IG_NOOP_5_SCCGRAPH_H
#include "common.h"
#include "Lifespan.h"
using namespace std;
/*
    SCCEdge 中 originEdgeSet 的更新策略
    原始图加入(u, v)引起的(Su, Sv)的变化
    1. 原始图插入(u, v)
        if (已经存在 || u == v ) {SCC不变}
        插入成功
        Su = findSCCIDNodeFromOriginNodeID(u)
        Sv = findSCCIDNodeFromOriginNodeID(v)
        if(Su == Sv) 不变
        else
            call insertEdge(Su, Sv, u, v)
            if (Su, Sv)已经存在 合并上u,v
            else 插入(Su, Sv)(u,v)
    2. 原始图删除(u, v)
        if(不存在u,v || u == v ) {SCC不变}
        删除成功
        Su = findSCCIDNodeFromOriginNodeID(u)
        Sv = findSCCIDNodeFromOriginNodeID(v)
        if(Su == Sv)
            SCC 分裂
        else
            Su,Sv deleteEdge(Su, Sv , u,v)
            if 还有其他边
            else
                删除u，v
*/
typedef struct Arc {
    int dstID;
    struct Arc* next;
    set<NodeEdge> originEdgeSet;
} SCCarc;

typedef struct {
    set<int> originNodeSet;
    int SCCID;
    SCCarc *firstArc;
} SCCnode;


class SCCGraph
{
public:
    vector<SCCnode> vertices;
    int timestamp;
    SCCGraph();
    SCCGraph(vector<int>& evolvingGraph, SccTable& sccTable, SCCEdgeInfo& sccEdgeInfo, int ts);
    ~SCCGraph();
    
    int IDexist(int SCCID);
    int addNode(int SCCID, set<int> originNodeSet);
    int deleteNode(int SCCID);
    
    int insertEdgeNotExist(int srcID, int dstID, set<NodeEdge> originEdgeSet);
    int insertEdgeNodeMustExist(int srcID, int dstID, set<NodeEdge> originEdgeSet);
    int deleteEdge(int srcID, int dstID, int u, int v);
    
    int deleteOutcomeEdge(SCCnode& node, int id);
    int deleteOutcomeEdge(SCCnode& node, vector<int> idVec);
    SCCarc* edgeExist(int srcID, int dstID);
    int getEdgeNum();
    int findSCCIDNodeFromOriginNodeID(int originNodeID);
    SCCnode findSCCnodeFromID(int SCCID);

    int merge(const vector<SCCnode>& cycle, SccTable& sccTable);
    vector<SCCnode> findCycle(int SCCIDu);
    pair<int, vector<SCCnode>> findCycles(int SCCIDu, SccTable& st);
    int newNodeID();
    pair<vector<int>, vector<int>> getInAndOutNodes(int SCCID);
    
    void storeSCCGraphJSON(string path);
};

typedef vector<SCCGraph> SCCGraphs;
SCCGraphs BuildSCCGraphs(vector<vector<int>> &evolvingGraphSequence, SccTable &sccTable, SCCEdgeInfoSequence sccEdgeInfoSequence, int timeIntervalLength) {
    SCCGraphs sccGraphs;
    for (int i = 0; i < timeIntervalLength; i++) {
        SCCGraph sccGraph(evolvingGraphSequence[i], sccTable, sccEdgeInfoSequence[i], i);
        sccGraphs.push_back(sccGraph);
    }
    return sccGraphs;
}
SCCGraph::SCCGraph() {}
SCCGraph::~SCCGraph() {}
SCCGraph::SCCGraph(vector<int>& evolvingGraph, SccTable& sccTable, SCCEdgeInfo& sccEdgeInfo, int ts) {
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
        auto findres = find_if(sccEdgeInfo.begin(), sccEdgeInfo.end(),
            [&](const SCCEdgeInfoItem& item) { return item.sccEdge.sScc == srcID && item.sccEdge.tScc == dstID; });
        assert(findres != sccEdgeInfo.end());
        insertEdgeNodeMustExist(srcID, dstID, findres->nodeEdges);
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

SCCarc* SCCGraph::edgeExist(int srcID, int dstID){
    for(auto it = vertices.begin(); it != vertices.end(); it++){
        if(it->SCCID == srcID){
            SCCarc *tmp = it->firstArc;
            while(tmp != NULL){
                if(tmp->dstID == dstID){
                    return tmp;
                }
                tmp = tmp->next;
            }
        }
    }
    return NULL;
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
int SCCGraph::insertEdgeNotExist(int srcID, int dstID, set<NodeEdge> originEdgeSet) {
    if (srcID == dstID) throw "insertEdgeMustNotExist: srcID == dstID";
    SCCarc* exist = this->edgeExist(srcID, dstID);
    if (exist != NULL) {
        throw "insertEdgeMustNotExist: edge already exists";
    }
    else {
        SCCarc* newArc = new SCCarc;
        newArc->dstID = dstID;
        newArc->originEdgeSet = originEdgeSet;
        for (auto it = vertices.begin(); it != vertices.end(); it++) {
            if (it->SCCID == srcID) {
                newArc->next = it->firstArc;
                it->firstArc = newArc;
                return 1;
            }
        }
        throw "insertEdgeMustNotExist: srcID not found";
    }
}

int SCCGraph::insertEdgeNodeMustExist(int srcID, int dstID, set<NodeEdge> originEdgeSet) {
    if (srcID == dstID) return 0;
    SCCarc* exist = this->edgeExist(srcID, dstID);
    if (exist != NULL) {
        //已经存在这条边，合并边的原始边集
        exist->originEdgeSet.insert(originEdgeSet.begin(), originEdgeSet.end());
        return 1;
    }
    else {
        if (this->IDexist(srcID) && this->IDexist(dstID)) {
            SCCarc* newArc = new SCCarc;
            newArc->dstID = dstID;
            newArc->originEdgeSet = originEdgeSet;
            for (auto it = vertices.begin(); it != vertices.end(); it++) {
                if (it->SCCID == srcID) {
                    newArc->next = it->firstArc;
                    it->firstArc = newArc;
                    break;
                }
            }
            return 1;
        }
        else throw "insertEdge: node not exist";
    }
}

//返回值: 1 删除成功，0 由于originNodeSet != NULL，没有删除
int SCCGraph::deleteEdge(int srcID, int dstID, int u, int v) {
    if (this->edgeExist(srcID, dstID) == NULL)
        throw "deleteEdge: edge not exist";
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        if (it->SCCID == srcID) {
            SCCarc *tmp = it->firstArc;
            SCCarc *pre = NULL;
            while (tmp != NULL) {
                if (tmp->dstID == dstID) {
                    int res = tmp->originEdgeSet.erase(NodeEdge(u, v));
                    if (res == 0) {
                        throw "deleteEdge: edge not exist";
                    }
                    //只有原始边集合为空的时候才删除
                    if (tmp->originEdgeSet.size() == 0) {
                        if (pre == NULL) {
                            it->firstArc = tmp->next;
                        } else {
                            pre->next = tmp->next;
                        }
                        delete tmp;
                        return 1;
                    }
                    return 0;
                }
                pre = tmp;
                tmp = tmp->next;
            }
        }
    }
    throw "deleteEdge: edge not exist";
}

int SCCGraph::deleteOutcomeEdge(SCCnode& node, int id) {
    SCCarc* current = node.firstArc;
    SCCarc* pre = NULL;
    while (current != NULL) {
        if (current->dstID == id) {
            if (pre == NULL) {
                node.firstArc = current->next;
            } else {
                pre->next = current->next;
            }
            delete current;
            return 1;
        }
        pre = current;
        current = current->next;
    }
    return 0;
}

int SCCGraph::deleteOutcomeEdge(SCCnode& node, vector<int> idVec) {
    SCCarc* current = node.firstArc;
    SCCarc* pre = NULL;
    while (current != NULL) {
        if (find(idVec.begin(), idVec.end(), current->dstID) != idVec.end()) {
            if (pre == NULL) {
                node.firstArc = current->next;
            } else {
                pre->next = current->next;
            }
            delete current;
            return 1;
        }
        pre = current;
        current = current->next;
    }
    return 0;
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
            SCCarc *tmp = it->firstArc;
            SCCarc *pre = NULL;
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
    SCCarc *it = NULL;
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
                    SCCarc* check = newNode.firstArc;
                    while (check != NULL) {
                        if (check->dstID == current->dstID) {
                            find = true;
                            check->originEdgeSet.insert(current->originEdgeSet.begin(), current->originEdgeSet.end());
                            break;
                        }
                        check = check->next;
                    }
                    if (!find) {
                        SCCarc* newarc = new SCCarc;
                        newarc->dstID = current->dstID;
                        newarc->next = newNode.firstArc;
                        newarc->originEdgeSet = current->originEdgeSet;
                        newNode.firstArc = newarc;
                    }
                }
                current = current->next;
            }
            it = vertices.erase(it);
        }
        else{
            //其他节点，需要把出边的目的节点改为新的SCCID
            SCCarc* prev = nullptr;
            SCCarc* current = it->firstArc;
            bool flag = false;
            
            set<NodeEdge> collection;
            while (current != nullptr) {
                if (sccIncycle(current->dstID, cycle)) {
                    flag = true;
                    if (prev == nullptr) {
                        it->firstArc = current->next;
                    }
                    else {
                        prev->next = current->next;
                    }
                    SCCarc* temp = current;
                    //需要收集原始边
                    collection.insert(current->originEdgeSet.begin(), current->originEdgeSet.end());
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
                SCCarc* newArc = new SCCarc;
                newArc->dstID = newNode.SCCID;
                newArc->next = it->firstArc;
                newArc->originEdgeSet = collection;
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
        //string p = "./scc" + to_string(cycleNum) + ".json";
        //storeSCCGraphJSON(p);
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
        for (SCCarc* p = node.firstArc; p; p = p->next) {
            Json::Value JsonArc;
            JsonArc["dstID"] = p->dstID;
            for (auto it : p->originEdgeSet) {
                Json::Value e;
                e["src"] = it.src;
                e["dst"] = it.dst;
                JsonArc["originEdges"].append(e);
            }
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