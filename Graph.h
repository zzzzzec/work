//
// Created by MoCuishle on 2019/11/29.
//

#ifndef IG_NOOP_5_GRAPH_H
#define IG_NOOP_5_GRAPH_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stack>
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
    void InsertEdge(int souID, int tarID);
    void DeleteEdge(int souID, int tarID);
    void DFSTraverse();
    void CreateVertex(int ID);
    bool AddSingleNode(int ID);
    int findSCCIDFromNodeId(int nodeID);
    int SCCIDremap(set<int> nodes, int sccID);
};

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

Graph::~Graph() {

}

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

    if (souPos == -1 || tarPos == -1) {
        cout << "Don't exist this sourceNode/targetNode!" << endl;
    } else {
        ArcNode *p = vertices[souPos].firstArc;
        ArcNode *q = NULL;

        if (p == NULL) {
            cout << "Don't exist this edge" << endl;
        } else {
            q = p;
            if (p->tarID == tarID) {
                p = p->nextarc;
                delete (q);
                vertices[souPos].firstArc = p;

                edgenum--;
            } else {
                while (p != NULL) {
                    if (p->tarID == tarID) {
                        p = p->nextarc;
                        delete (q->nextarc);
                        q->nextarc = p;

                        edgenum--;
                        break;
                    } else {
                        q = p;
                        p = p->nextarc;
                    }
                }
            }
        }
    }

    if (vertices[tarPos].firstArc == NULL) {
        vertices.erase(vertices.begin() + tarPos);
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

bool Graph::AddSingleNode(int ID){
    for (auto it = vertices.begin(); it != vertices.end(); it++){
        if (it->souID == ID) {
            return false;
        }
    }
    VerNode newVerNode(ID);
    vertices.push_back(newVerNode);
    vexnum++;
    connectedCount ++;
    return true;
}

void Graph::DFSForConnection(int vPos, bool *visited) {
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


#endif //IG_NOOP_5_GRAPH_H
