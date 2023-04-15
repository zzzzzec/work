#ifndef IG_NOOP_5_GRAPH_H
#define IG_NOOP_5_GRAPH_H

#include "common.h"
#include "Lifespan.h"

using namespace std;

//定义索引图弧结构
typedef struct ArcNode {
    int tarID;                      //目的节点ID
    ArcNode *nextarc;             //指向下一条弧的指针
    ArcNode() { nextarc = NULL; }
    ArcNode(int v) {
        tarID = v;
        nextarc = NULL;
    }
} ArcNode;

//定义索引图顶点结构
typedef struct VerNode {
    int souID;                      //源节点ID
    int sccOfIG;                    //标识源节点位于IG中哪个SCC
    ArcNode *firstArc;              //该节点第一条出边
    VerNode() {
        sccOfIG = 0;
        firstArc = NULL;
    }
    VerNode(int u) {
        souID = u;
        sccOfIG = 0;
        firstArc = NULL;
    }
    VerNode(int id, int sccid) {
        souID = id;
        sccOfIG = sccid;
        firstArc = NULL;
    }
} VerNode;

//定义邻接表
typedef struct vector<VerNode> AdjList_Graph;

typedef struct NodeinG {
    int ID;
    int Pos;
} NodeinG;
 
class Graph {
private:
    AdjList_Graph vertices;             //邻接表
    int vexnum, edgenum;                //节点数&边数
    int connectedCount;                 //强连通分量个数
    stack<NodeinG> reversePost;         //图中各顶点逆后序
    map<int, int> map_ver2scc;          //统计各节点对应的sccID

private:
    void DFS(int vPos, bool *visited);
    void DFSForReversePost(int vPos, bool *visited);            //DFS思想求逆后续
    void DFSForConnection(int vPos, bool *visited);             //用DFS思想来求强连通分量

public:
    Graph();
    ~Graph();
    Graph Reverse();                                    //求反向图
    AdjList_Graph GetVertices();                        //获取图的邻接表
    int GetVexNum();                                    //获取图的节点数
    int GetConnectedCount();                            //获取SCC数量
    map<int, vector<NodeinG>> GetSccOfIGraph();         //获取IGraph中的SCC及其对应ID
    stack<NodeinG> GetReversePost();                    //返回顶点的逆后序序列
    void ClearReversePost();                            //清空栈reversePost中的记录
    void CalReversePost();                              //通过递归调用DFSForReversePost求得逆后序
    void CalculateConnection();                         //求图的强连通分量
    void SumScc();

    map<int, int> GetMapV2S();
    bool NodeIsExists(int nodeID);
    int VerPos(int nodeID);
    void AddOutToSourceNode(int souPos, int tarID);
    bool AddNode(int ID, int SCCID);
    void InsertEdge(int souID, int tarID);
    void InsertEdgeWithCheck(int souID, int tarID);
    void DeleteEdge(int souID, int tarID);
    void DeleteNode(int nodeID);
    void DFSTraverse();
    void CreateVertex(int ID);
    int findSCCIDFromNodeId(int nodeID);
    int SCCIDremap(set<int> nodes, int sccID);
    VerNode& findNodeRefByID(int nodeID);
    vector<int> findOutArcList(int nodeID);
    vector<int> findInArcList(int nodeID);
    int getRandomNodeID() {return vertices[rand() % vertices.size()].souID;}
    bool IsReachable(int src, int dst);
};
typedef vector<Graph> Graphs;
Graph::Graph() {
    vexnum = 0;
    edgenum = 0;
    connectedCount = 0;

    for (int i = 0; i < vexnum; ++i) {
        vertices[i].souID = -1;
        vertices[i].sccOfIG = -1;
        vertices[i].firstArc = NULL;
    }
}

Graph::~Graph() {}

int Graph::VerPos(int nodeID) {
    for (int i = 0; i < vertices.size(); ++i) {
        if (vertices[i].souID == nodeID) {
            return i;
        }
    }
    return -1;
}

void Graph::DFS(int vPos, bool *visited) {
    visited[vPos] = true;
    cout << vertices[vPos].souID << " ";
    ArcNode *p = vertices[vPos].firstArc;
    while (p) {
        int nextId = p->tarID;
        int nextPos = VerPos(nextId);

        if (nextPos != -1) {
            if (!visited[nextPos]) {
                DFS(nextPos, visited);
            }
            p = p->nextarc;
        } else {
            cout << "Don't exists this vertex" << endl;
        }
    }
}

Graph Graph::Reverse() {
    Graph R;
    for (int i = 0; i < vexnum; ++i) {
        for (ArcNode *p = vertices[i].firstArc; p != NULL; p = p->nextarc) {
            R.InsertEdge(p->tarID, vertices[i].souID);
        }
    }
    return R;
}

void Graph::InsertEdge(int souID, int tarID) {
    int souPos = VerPos(souID);
    int tarPos = VerPos(tarID);

    if (souPos != -1) {
        //当前存在节点(souID,souLife)
        AddOutToSourceNode(souPos, tarID);
    } else {
        //当前不存在节点(souID,souLife)，需创建该节点
        VerNode newSouNode(souID);
        vertices.push_back(newSouNode);
        vexnum++;
        souPos = vertices.size() - 1;
        AddOutToSourceNode(souPos, tarID);
    }

    if (tarPos == -1) {
        VerNode newTarNode(tarID);
        vertices.push_back(newTarNode);
        vexnum++;
    }
}

void Graph::InsertEdgeWithCheck(int souID, int tarID) {
    int souPos = VerPos(souID);
    int tarPos = VerPos(tarID);
    if(souPos == -1 || tarPos == -1) {
        cout << "InsertEdgeWithCheck: souID or tarID is not exists" << endl;
        exit(0);
    }
    AddOutToSourceNode(souPos, tarID);
}

void Graph::AddOutToSourceNode(int souPos, int tarID) {
//尾插法
    ArcNode *newArcNode = new ArcNode(tarID);
    ArcNode *temp = vertices[souPos].firstArc;

    if (temp == NULL) {
        //temp为NULL
        vertices[souPos].firstArc = newArcNode;
        edgenum++;
        return;
    } else {
        while (temp->nextarc) {
            //temp非NULL
            temp = temp->nextarc;
        }
        temp->nextarc = newArcNode;
        edgenum++;
    }
}

AdjList_Graph Graph::GetVertices() {
    AdjList_Graph temp = vertices;
    return temp;
}

int Graph::GetVexNum() {
    int temp = vexnum;
    return temp;
}

int Graph::GetConnectedCount() {
    return connectedCount;
}

void Graph::ClearReversePost() {
    while (!reversePost.empty()) {
        reversePost.pop();
    }
}

void Graph::CalReversePost() {
    ClearReversePost();
    bool *visited = new bool[vexnum];
    for (int i = 0; i < vexnum; ++i) {
        visited[i] = false;
    }
    for (int j = 0; j < vexnum; ++j) {
        if (!visited[j]) {
            DFSForReversePost(j, visited);
        }
    }
    delete[] visited;
}

void Graph::DFSForReversePost(int vPos, bool *visited) {
    visited[vPos] = true;

    for (ArcNode *p = vertices[vPos].firstArc; p; p = p->nextarc) {
        int nextId = p->tarID;
        int nextPos = VerPos(nextId);

        if (!visited[nextPos]) {
            DFSForReversePost(nextPos, visited);
        }
    }

    NodeinG cnode;
    cnode.ID = vertices[vPos].souID;
    cnode.Pos = vPos;

    reversePost.push(cnode);
}

void Graph::CalculateConnection() {
    connectedCount = 0;
    bool *visited = new bool[vexnum];
    for (int i = 0; i < vexnum; ++i) {
        vertices[i].sccOfIG = 0;
        visited[i] = false;
    }
    //根据本图的反向图的顶点逆后序序列来进行DFS
    //所有在同一个递归DFS调用中被访问到的顶点都在同一个强连通分量中
    Graph R = this->Reverse();
    R.CalReversePost();
    //获取逆后序
    stack<NodeinG> topostack = R.GetReversePost();
    while (!topostack.empty()) {
        int tarID = topostack.top().ID;
        int j = VerPos(tarID);
        topostack.pop();
        if (!visited[j]) {
            connectedCount++;
            DFSForConnection(j, visited);
        }
    }
    delete[] visited;
}

map<int, vector<NodeinG>> Graph::GetSccOfIGraph() {
    map<int, vector<NodeinG>> mSCCinIGraph;

    for (int i = 0; i < vexnum; ++i) {
        int sccID = vertices[i].sccOfIG;

        NodeinG cnode;
        cnode.ID = vertices[i].souID;
        cnode.Pos = i;

        auto findKey = mSCCinIGraph.find(sccID);

        if (findKey != mSCCinIGraph.end()) {
            (*findKey).second.push_back(cnode);
        } else {
            mSCCinIGraph.insert(pair<int, vector<NodeinG>>(sccID, {cnode}));
        };
    }

    return mSCCinIGraph;
}

stack<NodeinG> Graph::GetReversePost() {
    stack<NodeinG> tmp(reversePost);

    return tmp;
}

bool Graph::NodeIsExists(int nodeID) {
    for (int i = 0; i < vertices.size(); ++i) {
        if (vertices[i].souID == nodeID) {
            return true;
        }
    }
    return false;
}

void Graph::DeleteEdge(int souID, int tarID) {
    int souPos = VerPos(souID);
    int tarPos = VerPos(tarID);
    if (souPos == -1) {
        throw "DeleteEdge: source node is not exists";
    }
    if (tarPos == -1) {
        throw "DeleteEdge: target node is not exists";
    }
    else {
        ArcNode* p = vertices[souPos].firstArc;
        ArcNode* q = NULL;
        while (p) {
            if (p->tarID == tarID) {
                if (q == NULL) {
                    vertices[souPos].firstArc = p->nextarc;
                }
                else {
                    q->nextarc = p->nextarc;
                }
                delete p;
                edgenum--;
                return;
            }
            q = p;
            p = p->nextarc;
        }
        throw "DeleteEdge: edge is not exists";
    }
}

void Graph::DFSTraverse() {
    bool *visited = new bool[vexnum];
    for (int i = 0; i < vexnum; ++i) {
        visited[i] = false;
    }
    for (int j = 0; j < vexnum; ++j) {
        if (!visited[j]) {
            DFS(j, visited);
        }
    }
    delete[] visited;
}

void Graph::CreateVertex(int ID) {
    VerNode newVerNode(ID);
    vertices.push_back(newVerNode);
    vexnum++;
}

bool Graph::AddNode(int ID, int SCCID){
    for (auto it = vertices.begin(); it != vertices.end(); it++){
        if (it->souID == ID) {
            return false;
        }
    }
    VerNode newVerNode(ID, SCCID);
    vertices.push_back(newVerNode);
    vexnum++;
    connectedCount ++;
    return true;
}

void Graph::DeleteNode(int ID) {
    int pos = VerPos(ID);
    if (pos == -1) {
        cout << "Don't exist this node!" << endl;
    } else {
        vertices.erase(vertices.begin() + pos);
        vexnum--;
    }
}

void Graph::DFSForConnection(int vPos, bool* visited) {
    visited[vPos] = true;
    vertices[vPos].sccOfIG = connectedCount - 1;

    for (ArcNode *p = vertices[vPos].firstArc; p; p = p->nextarc) {
        int nextId = p->tarID;
        int nextPos = VerPos(nextId);

        if (!visited[nextPos]) {
            DFSForConnection(nextPos, visited);
        }
    }
}

void Graph::SumScc() {
    for (int i = 0; i < vexnum; ++i) {
        int curVid = vertices[i].souID;
        int curSid = vertices[i].sccOfIG;
        map_ver2scc.insert(pair<int, int>(curVid, curSid));
    }
}

map<int, int> Graph::GetMapV2S() {
    return map_ver2scc;
}

int Graph::findSCCIDFromNodeId(int nodeID) {
    for(auto it = vertices.begin(); it != vertices.end(); it++){
        if (it->souID == nodeID){
            return it->sccOfIG;
        }
    }
    return -1;
}

int Graph::SCCIDremap(set<int> nodes, int sccID){
    for (auto it = this->vertices.begin(); it != this->vertices.end(); it++){
        if (nodes.find(it->souID) != nodes.end()){
            it->sccOfIG = sccID;
        }
    }
    return 1;
}

VerNode& Graph::findNodeRefByID(int nodeID){
    for (auto it = this->vertices.begin(); it != this->vertices.end(); it++){
        if (it->souID == nodeID){
            return *it;
        }
    }
    throw "No such node!";
}

vector<int> Graph::findOutArcList(int nodeID) {
    vector<int> outArcList;
    auto node = findNodeRefByID(nodeID);
    for (auto it = node.firstArc; it != NULL; it = it->nextarc){
        outArcList.push_back(it->tarID);
    }
    return outArcList;
}

vector<int> Graph::findInArcList(int nodeID) {
    vector<int> inArcList;
    for (auto it = this->vertices.begin(); it != this->vertices.end(); it++){
        for (auto it2 = it->firstArc; it2 != NULL; it2 = it2->nextarc){
            if (it2->tarID == nodeID){
                inArcList.push_back(it->souID);
            }
        }
    }
    return inArcList;
}

bool Graph::IsReachable(int src, int dst) {
    if(NodeIsExists(src) == false || NodeIsExists(dst) == false)
        return false;
    if (src == dst)
        return true;
    map<int,bool> visited;
    stack<int> s;
    s.push(src);
    while (!s.empty()) {
        int u = s.top();
        s.pop();
        int spos = VerPos(u);
        for (ArcNode* v = vertices[spos].firstArc; v != NULL; v = v->nextarc) {
            if (v->tarID == dst)
                return true;
            if (!visited[v->tarID]) {
                visited[v->tarID] = true;
                s.push(v->tarID);
            }
        }
    }
    return false;
}

#endif //IG_NOOP_5_GRAPH_H
