#ifndef IG_NOOP_5_INDEXGRAPH_H
#define IG_NOOP_5_INDEXGRAPH_H

#include "NIT.h"
#include "Lifespan.h"
#include "common.h"
#include "SCCGraph.h"

using namespace std;

typedef struct CopyNode {
    int ID;
    int Pos;
    bitset<MNS> lifespan;

    CopyNode() {
        ID = -1;
        Pos = -1;
        lifespan.reset();
    }

    CopyNode(int id, int pos, bitset<MNS> life) {
        ID = id;
        Pos = pos;
        lifespan = life;
    }

} CopyNode;

typedef struct IGNode {
    int nodeID;
    bitset<MNS> nodeLife;
} IGNode;

//定义索引图弧结构
typedef struct IGArcNode {
    int taruuid;
    int tarID;                          //目的节点ID
    bitset<MNS> tarLifespan;            //目的节点Lifespan
    IGArcNode* nextarc;                 //指向下一条弧的指针
    IGArcNode() { nextarc = NULL; }
    IGArcNode(int id, int v, bitset<MNS> t) {
        taruuid = id;
        tarID = v;
        tarLifespan = t;
        nextarc = NULL;
    }
} IGArcNode;

//定义索引图顶点结构
typedef struct IGVerNode {
    int uuid;
    int souID;                          //源节点ID
    bitset<MNS> souLifespan;            //源节点Lifespan
    int sccOfIG;                        //标识源节点位于IG中哪个SCC
    IGArcNode *firstArc;                //该节点第一条出边

    IGVerNode(int id, int u, bitset<MNS> t) {
        uuid = id;
        souID = u;
        souLifespan = t;
        sccOfIG = 0;
        firstArc = NULL;
    }
    
    bool operator==(const IGVerNode& node) const {
        if(souID == node.souID && souLifespan == node.souLifespan) {
            auto p = firstArc;
            while (p != NULL) {
                auto q = node.firstArc;
                while (q != NULL) {
                    if (p->tarID == q->tarID && p->tarLifespan == q->tarLifespan) {
                        break;
                    }
                    q = q->nextarc;
                }
                if (q == NULL) {
                    return false;
                }
                p = p->nextarc;
            }
            return true;
        }
        else {
            return false;
        }
    }
} IGVerNode;
typedef struct vector<IGVerNode> AdjList;

//定义排序:按照生存期规模升序排序
bool copyNodeSort(CopyNode copyNode1, CopyNode copyNode2) {
    return copyNode1.lifespan.count() < copyNode2.lifespan.count();
}

class IGraph {
private:
    AdjList vertices;                   //邻接表
    int vexnum, edgenum;                //节点数&边数
    int connectedCount;                 //强连通分量个数
    stack<CopyNode> reversePost;        //图中各顶点

public:
    IGraph();
    ~IGraph();
    int GetVexNum();                            //获取图的节点数
    int GetEdgeNum();                           //获取图的边数
    int VerPos(int nodeID, bitset<MNS> verLife);
    int Newuuid();
    void AddOutToSourceNode(int souPos, int tarID, bitset<MNS> tarLife);
    void InsertEdge(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void InsertEdgeOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void InsertEdgeSrcMustExistOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    
    void DeleteEdge1(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void DeleteEdge2(int souPos, int tarPos);
    void DeleteEdgeKeepEmptyNode(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);

    void deleteOutComingEdge(IGVerNode* node, int tarID, bitset<MNS> tarLife);
    void deleteNodeOrThrow(const IGVerNode node);
    void deleteNodeWhichInCycleAndIncludeTimestamp(const vector<SCCnode>& cycle, int timestamp);
    void deleteEmptyNode();
    
    void CreateVertex(int ID, bitset<MNS> t);
    vector<CopyNode> FindAllCopyNodes(int souID);
    int FindIDonIG(int sccID, bitset<MNS> lifespan);           //根据SCC的id及其生存期，确定其在索引图上的ID
    IGVerNode* findNode(int id, Lifespan lifespan);
    IGVerNode* findNodeByPos(int pos) { return &vertices[pos]; }
    
    bool HaveSuperLifespan(vector<CopyNode>& copyNodes, bitset<MNS> t);

    void ProTarget(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void MaintainRelationship(int verID, bitset<MNS> verLifespan);
    void StoreFullIndexGraph(string storeFull_IG_Address);
    void StoreFullIndexGraphJSON(string storeFull_IG_Address);
    void ConstructOutEdge(int souID, int tarID, bitset<MNS> t, int label, bitset<MNS> intervalUnion);
    
    void ModifyNodeOrThrow(int id, Lifespan lifespan, Lifespan newLifespan);

    void updateAddRefineRecord(const RefineRecordItem& ritem);

    

    bitset<MNS> GetSubVertexUnionLife(int verPos);
    vector<int> GetSouVerticesPos(int tarID, bitset<MNS> tarLife);
    vector<int> GetSubVerticesPos(int tarPos);
    
    bool isReachable(int u, int v) {
        map<int, bool> visited;
        return isReachableUtil(u, v, visited);
    }

private:
    bool isReachableUtil(const int u, const int v, map<int, bool>& visited) {
        if (u == v) return true;
        visited[vertices[u].uuid] = true;
        for (IGArcNode* arc = vertices[u].firstArc; arc != NULL; arc = arc->nextarc) {
            int arcPos = VerPos(arc->tarID, arc->tarLifespan);
            if (!visited[vertices[arcPos].uuid] && isReachableUtil(arcPos, v, visited)) return true;
        }
        return false;
    }
    bool hasIncomeEdge(int uuid);
};

IGraph::IGraph() {
    vexnum = 0;
    edgenum = 0;
    connectedCount = 0;
}

IGraph::~IGraph() {}

int IGraph::GetVexNum() {
    return vertices.size();
}

int IGraph::GetEdgeNum() {
    int tmp = edgenum;
    return tmp;
}

int IGraph::VerPos(int nodeID, bitset<MNS> verLife) {
    for (int i = 0; i < vertices.size(); ++i) {
        if (vertices[i].souID == nodeID && vertices[i].souLifespan == verLife) {
            return i;
        }
    }
    return -1;
}

int IGraph::Newuuid() {
    int id = 0;
    for (int i = 0; i < vertices.size(); ++i) {
        if (vertices[i].uuid > id) {
            id = vertices[i].uuid;
        }
    }
    return id + 1;
}

void IGraph::AddOutToSourceNode(int souPos, int tarID, bitset<MNS> tarLife) {
    //尾插法
    int taruuid = -1;
    for (auto node : vertices) {
        if (node.souID == tarID && node.souLifespan == tarLife) {
            taruuid = node.uuid;
            break;
        }
    }
    assert(taruuid != -1);
    IGArcNode* newArcNode = new IGArcNode(taruuid ,tarID, tarLife);
    IGArcNode *temp = vertices[souPos].firstArc;

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

void IGraph::InsertEdgeOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);
    if (souPos == -1 || tarPos == -1)
        throw "node not exist";
    AddOutToSourceNode(souPos, tarID, tarLife);
}

void IGraph::InsertEdgeSrcMustExistOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);
    if (souPos == -1)
        throw "InsertEdgeSrcMustExistOrThrow: ";
    if (tarPos == -1) {
        int newid = Newuuid();
        IGVerNode newTarNode(newid, tarID, tarLife);
        vertices.push_back(newTarNode);
        vexnum++;
    }
    AddOutToSourceNode(souPos, tarID, tarLife);
}

void IGraph::InsertEdge(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);

    if (tarPos == -1) {
        int newid = Newuuid();
        IGVerNode newTarNode(newid, tarID, tarLife);
        vertices.push_back(newTarNode);
        vexnum++;
    }
    if (souPos != -1) {
        //当前存在节点(souID,souLife)
        AddOutToSourceNode(souPos, tarID, tarLife);
    } else {
        //当前不存在节点(souID,souLife)，需创建该节点
        int newid = Newuuid();
        IGVerNode newSouNode(newid, souID, souLife);
        vertices.push_back(newSouNode);
        vexnum++;
        souPos = vertices.size() - 1;
        AddOutToSourceNode(souPos, tarID, tarLife);
    }

}

void IGraph::DeleteEdgeKeepEmptyNode(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife)
{
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);

    if (souPos == -1 || tarPos == -1) {
        throw "Don't exist this sourceNode!";
    } else {
        IGArcNode *p = vertices[souPos].firstArc;
        IGArcNode *q = NULL;

        if (p == NULL) {
            throw "Don't exist this edge";
        } else {
            q = p;
            if (p->tarID == tarID && p->tarLifespan == tarLife) {
                p = p->nextarc;
                delete (q);
                vertices[souPos].firstArc = p;
                edgenum--;
            } else {
                while (p != NULL) {
                    if (p->tarID == tarID && p->tarLifespan == tarLife) {
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
}

// must exist
void IGraph::DeleteEdge1(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);

    if (souPos == -1 || tarPos == -1) {
        throw "Don't exist this sourceNode!";
    } else {
        IGArcNode *p = vertices[souPos].firstArc;
        IGArcNode *q = NULL;

        if (p == NULL) {
            throw "Don't exist this edge";
        } else {
            q = p;
            if (p->tarID == tarID && p->tarLifespan == tarLife) {
                p = p->nextarc;
                delete (q);
                vertices[souPos].firstArc = p;
                edgenum--;
            } else {
                while (p != NULL) {
                    if (p->tarID == tarID && p->tarLifespan == tarLife) {
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
        vexnum--;
    }
}

void IGraph::DeleteEdge2(int souPos, int tarPos) {
    IGArcNode *p = vertices[souPos].firstArc;
    IGArcNode *q = NULL;

    int tarID = vertices[tarPos].souID;
    bitset<MNS> tarLife = vertices[tarPos].souLifespan;

    if (p == NULL) {
        cout << "Don't exist this edge" << endl;
    } else {
        q = p;
        if (p->tarID == tarID && p->tarLifespan == tarLife) {
            p = p->nextarc;
            delete (q);
            vertices[souPos].firstArc = p;

            edgenum--;
        } else {
            while (p != NULL) {
                if (p->tarID == tarID && p->tarLifespan == tarLife) {
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

    /*if (vertices[tarPos].firstArc == NULL) {
        vertices.erase(vertices.begin() + tarPos);
    }*/
}

void IGraph::CreateVertex(int ID, bitset<MNS> t) {
    int newid = Newuuid();
    IGVerNode newVerNode(newid, ID, t);
    vertices.push_back(newVerNode);
    vexnum++;
}

vector<CopyNode> IGraph::FindAllCopyNodes(int souID) {
    vector<CopyNode> copyNodes;
    for (int i = 0; i < vertices.size(); ++i) {
        if (vertices[i].souID == souID) {
            CopyNode cnode(souID, i, vertices[i].souLifespan);

            copyNodes.push_back(cnode);
        }
    }
    return copyNodes;
}

bool IGraph::HaveSuperLifespan(vector<CopyNode> &copyNodes, bitset<MNS> t) {
    for (int i = 0; i < copyNodes.size(); ++i) {
        bitset<MNS> curLife = copyNodes[i].lifespan;

        if (LifespanisSub(curLife, t)) {
            return true;
        }
    }
    return false;
}

void IGraph::ProTarget(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int tarPos = VerPos(tarID, tarLife);

    if (tarPos != -1) {
        //当前IG中存在节点(tarID,tarLife),将(souID,souLife)和(tarID,tarLife)相连构成出边(souID,tarID,t)
        InsertEdge(souID, souLife, tarID, tarLife);
    } else {
        CreateVertex(tarID, tarLife);
        MaintainRelationship(tarID, tarLife);
        InsertEdge(souID, souLife, tarID, tarLife);
    }
}

//对应case3，加入边(<souID,L>,<souID,tx>), verLifespan为tx
void IGraph::MaintainRelationship(int verID, bitset<MNS> verLifespan) {
    //找到当前u的所有拷贝,将其存入copyNodes中
    vector<CopyNode> copyNodes = FindAllCopyNodes(verID);
    sort(copyNodes.begin(), copyNodes.end(), copyNodeSort);

    for (int j = 0; j < copyNodes.size(); ++j) {
        bitset<MNS> curLife = copyNodes[j].lifespan;

        if (LifespanisProSub(verLifespan, curLife)) {
            //(verID,curLife)是(verID,verLifespan)的子节点
            InsertEdge(verID, verLifespan, verID, curLife);
        } else if (LifespanisProSub(curLife, verLifespan)) {
            //(verID,verLifespan)是(verID,curLife)的子节点
            InsertEdge(verID, curLife, verID, verLifespan);
        }
    }
}


void IGraph::StoreFullIndexGraph(string storeFull_IG_Address) {
    vector<vector<IGNode>> edgeOfIG;

    for (int i = 0; i < vexnum; ++i) {
        vector<IGNode> curNodeVector;
        int curNodeID = vertices[i].souID;
        bitset<MNS> curNodeLife = vertices[i].souLifespan;
        IGNode curSouNode;
        curSouNode.nodeID = curNodeID;
        curSouNode.nodeLife = curNodeLife;
        curNodeVector.push_back(curSouNode);

        for (IGArcNode *p = vertices[i].firstArc; p; p = p->nextarc) {
            int tarID = p->tarID;
            bitset<MNS> tarLife = p->tarLifespan;

            IGNode curTarNode;
            curTarNode.nodeID = tarID;
            curTarNode.nodeLife = tarLife;
            curNodeVector.push_back(curTarNode);
        }

        edgeOfIG.push_back(curNodeVector);
    }

    ofstream fout(storeFull_IG_Address);
    if (fout) {
        for (auto it = edgeOfIG.begin(); it != edgeOfIG.end(); it++) {
            auto souIt = (*it).begin();
            fout << (*souIt).nodeID << "-{" << (*souIt).nodeLife << "} :" << endl;

            auto tmp = souIt;
            tmp++;

            for (auto tarIter = tmp; tarIter != (*it).end(); tarIter++) {
                fout << "\t # " << (*tarIter).nodeID << "-{" << (*tarIter).nodeLife << "}" << endl;
            }
            fout << "-----------------------" << endl;
        }
    }
}

void IGraph::StoreFullIndexGraphJSON(string path) {
    Json::Value JsonGraph;
    for (auto node : vertices) {
        Json::Value JsonNode;
        JsonNode["uuid"] = node.uuid;
        JsonNode["souID"] = node.souID;
        JsonNode["souLifespan"] = node.souLifespan.to_string();
        Json::Value JsonArcs;
        for (IGArcNode *p = node.firstArc; p; p = p->nextarc) {
            Json::Value JsonArc;
            JsonArc["uuid"] = p->taruuid;
            JsonArc["tarID"] = p->tarID;
            JsonArc["tarLifespan"] = p->tarLifespan.to_string();
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

//处理ONTable里的一个表项里的一条边，intervalUnion是N2+的时间并集(论文里的case2)
void IGraph::ConstructOutEdge(int souID, int tarID, bitset<MNS> t, int label, bitset<MNS> intervalUnion) {
    if (label == 1) {
        //该记录属于Instant-Part, 只有一个时间点tx，创建边((souID,tx),(tarID,tx))
        int souPos = VerPos(souID, t);
        if (souPos != -1) {
            //存在(u,t)节点,以其为源节点构建
            ProTarget(souID, t, tarID, t);
        } else {
            //不存在(u,t)节点,以其为源节点构建
            CreateVertex(souID, t);
            //case3,理论上此时的t来自L1，只有一个有效位
            MaintainRelationship(souID, t);
            if (t.count() != 1) {
                cout << "error" << endl;
            }
            ProTarget(souID, t, tarID, t);
        }
    } else if (label == 2) {
        //该记录属于Interval-Part, 创建边((souID, intervalUnion),(tarID,t))
        //确定IG中是否存在Interval-Part的源节点
        int souPos = VerPos(souID, intervalUnion);
        if (souPos != -1) {
            //源节点存在
            ProTarget(souID, intervalUnion, tarID, t);
        } else {
            //源节点不存在
            CreateVertex(souID, intervalUnion);
            ProTarget(souID, intervalUnion, tarID, t);
        }
    }
}

int IGraph::FindIDonIG(int sccID, bitset<MNS> lifespan) {
    //找到当前u的所有拷贝,将其存入copyNodes中
    vector<CopyNode> copyNodes;
    for (int i = 0; i < vertices.size(); ++i) {
        if (vertices[i].souID == sccID) {
            bitset<MNS> curLife = vertices[i].souLifespan;
            bitset<MNS> interLife = LifespanJoin(lifespan, curLife);
            if (interLife.any()) {
                CopyNode copyNode(sccID, i, vertices[i].souLifespan);
                copyNodes.push_back(copyNode);
            }
        }
    }
    //找到生命周期最小的一个
    if (!copyNodes.empty()) {
        CopyNode minSup = copyNodes[0];
        for (auto iter = copyNodes.begin(); iter != copyNodes.end(); iter++) {
            bitset<MNS> tmpLife = (*iter).lifespan;
            bitset<MNS> minSupLife = minSup.lifespan;
            if (LifespanisProSub(minSupLife, tmpLife)) {
                minSup = (*iter);
            }
        }
        int resultID = minSup.Pos;

        return resultID;
    } else {
        throw "There is no this vertex";
    }
}

bitset<MNS> IGraph::GetSubVertexUnionLife(int verPos) {
    bitset<MNS> subVertexUnionLife;
    subVertexUnionLife.reset();
    int curID = vertices[verPos].souID;
    IGArcNode *p = vertices[verPos].firstArc;

    while (p) {
        int curTarID = p->tarID;
        if (curTarID == curID) {
            bitset<MNS> subVertexLife = p->tarLifespan;
            subVertexUnionLife = LifespanUnion(subVertexUnionLife, subVertexLife);
        }
        p = p->nextarc;
    }
    return subVertexUnionLife;
}

vector<int> IGraph::GetSouVerticesPos(int tarID, bitset<MNS> tarLife) {
    vector<int> souPosVector;

    for (int i = 0; i < vertices.size(); ++i) {
        int curPos = i;
        IGArcNode *p = vertices[curPos].firstArc;

        while (p) {
            int curTarID = p->tarID;
            bitset<MNS> curTarLife = p->tarLifespan;
            if (curTarID == tarID && curTarLife == tarLife) {
                souPosVector.push_back(curPos);
            }
            p = p->nextarc;
        }
    }

    return souPosVector;
}

vector<int> IGraph::GetSubVerticesPos(int tarPos) {
    vector<int> subPosVector;
    int curID = vertices[tarPos].souID;
    IGArcNode *p = vertices[tarPos].firstArc;

    while (p) {
        int curTarID = p->tarID;
        if (curTarID == curID) {
            bitset<MNS> curTarLife = p->tarLifespan;
            int curTarPos = VerPos(curTarID, curTarLife);

            if (curTarPos != -1) {
                subPosVector.push_back(curTarPos);
            }
        }
        p = p->nextarc;
    }
    return subPosVector;
}

IGVerNode* IGraph::findNode(int id , Lifespan Lifespan) {
    for (int i = 0; i < vertices.size(); i++) {
        IGVerNode* tmp = &(vertices[i]);
        if (tmp->souID == id && tmp->souLifespan == Lifespan)
            return tmp;
    }
    return NULL;
}

void IGraph::ModifyNodeOrThrow(int id, Lifespan lifespan, Lifespan newLifespan) {
    auto res = findNode(id, lifespan);
    if (res == nullptr)
        throw "node not exist";
    res->souLifespan = newLifespan;
    for (auto it : vertices) {
        auto edgeit = it.firstArc;
        while (edgeit != NULL) {
            if (edgeit->tarID == id && edgeit->tarLifespan == lifespan)
                edgeit->tarLifespan = newLifespan;
            edgeit = edgeit->nextarc;
        }
    }
}

void IGraph::updateAddRefineRecord(const RefineRecordItem& ritem){
    bitset<MNS> intervalUnion;
    intervalUnion.reset(); //处理所有node节点的出边
    for (int i = 0; i < ritem.Out.size(); ++i) {
        if (ritem.Out[i].partLab == 2) { //如果这个node节点的这个出边是没有入边的 A->B 类型
            bitset<MNS> cur_life = ritem.Out[i].lifespan;
            intervalUnion = LifespanUnion(cur_life, intervalUnion);
        }
    }
    for (int j = 0; j < ritem.Out.size(); ++j) {
        int tarID = ritem.Out[j].vertexID;
        int label = ritem.Out[j].partLab;
        bitset<MNS> edgeLife = ritem.Out[j].lifespan;
        ConstructOutEdge(ritem.node, tarID, edgeLife, label, intervalUnion);
    }
}

IGraph BuildIndexGraph(RefineNITable refineNITable);
IGraph ReadIndexGraph(string storeFull_IG_Address);
IGraph BuildIndexGraph(RefineNITable refineNITable) {
    IGraph IG;
    int c = 0;
    int sizeOfreNIT = refineNITable.size();
    //一个元素代表ONTable里的一个表项(SCC)
    for (auto recordOfNIT = refineNITable.begin(); recordOfNIT != refineNITable.end(); recordOfNIT++) {
        c++;
        int souID = (*recordOfNIT).node;
        bitset<MNS> intervalUnion;
        intervalUnion.reset(); //处理所有node节点的出边
        
        for (int i = 0; i < (*recordOfNIT).Out.size(); ++i) {
            if ((*recordOfNIT).Out[i].partLab == 2) { //如果这个node节点的这个出边是没有入边的 A->B 类型
                bitset<MNS> cur_life = (*recordOfNIT).Out[i].lifespan;
                intervalUnion = LifespanUnion(cur_life, intervalUnion);
            }
        }

        int sizeOfOut = (*recordOfNIT).Out.size();
        for (int j = 0; j < (*recordOfNIT).Out.size(); ++j) {
            int tarID = (*recordOfNIT).Out[j].vertexID;
            int label = (*recordOfNIT).Out[j].partLab;
            bitset<MNS> edgeLife = (*recordOfNIT).Out[j].lifespan;
            IG.ConstructOutEdge(souID, tarID, edgeLife, label, intervalUnion);
        }
    }
    return IG;
}

IGraph ReadIndexGraph(string storeFull_IG_Address) {
    IGraph iGraph;

    ifstream fin(storeFull_IG_Address);
    if (fin) {
        string s;
        while (getline(fin, s)) {
            if (s[0] != '\t' && s[0] != '-') {
                vector<string> sv = split(s, "-{}:");
                string s1, s2;
                s1 = sv[0];
                s2 = sv[1];

                int souNodeID = stoi(s1);
                bitset<MNS> souNodeLife = StringToLifespan(s2);

                iGraph.CreateVertex(souNodeID, souNodeLife);

            } else if (s[0] == '\t') {
                vector<string> sv = split(s, "\t #-{}:");
                string s1, s2;
                s1 = sv[0];
                s2 = sv[1];

                int tarNodeID = stoi(s1);
                bitset<MNS> tarNodeLife = StringToLifespan(s2);

                int curSouPos = iGraph.GetVexNum() - 1;
                iGraph.AddOutToSourceNode(curSouPos, tarNodeID, tarNodeLife);
            } else {
                continue;
            }
        }
    }

    return iGraph;
}

bool IGraph::hasIncomeEdge(int uuid) {
    for (auto node : vertices) {
        auto edgeit = node.firstArc;
        while (edgeit != NULL) {
            if (edgeit->tarID == uuid)
                return true;
            edgeit = edgeit->nextarc;
        }
    }
    return false;
}

void IGraph::deleteOutComingEdge(IGVerNode* node, int tarID, bitset<MNS> tarLife) {
    IGArcNode* p = node->firstArc;
    IGArcNode* q = NULL;
    while (p != NULL) {
        if (p->tarID == node->souID && p->tarLifespan == node->souLifespan) {
            if (q == NULL) {
                node->firstArc = p->nextarc;
                delete p;
                cout << "delete edge " << node->souID << node->souLifespan.to_string()<< " -> " << tarID << tarLife.to_string() << endl;
                break;
            }
            else {
                q->nextarc = p->nextarc;
                delete p;
                cout << "delete edge " << node->souID << node->souLifespan.to_string()<< " -> " << tarID << tarLife.to_string() << endl;
                break;
            }
        }
        else {
            q = p;
            p = p->nextarc;
        }
    }
}

void IGraph::deleteNodeOrThrow(const IGVerNode node) {
    auto it = vertices.begin();
    bool find = false;
    while (it != vertices.end()) {
        if (it->souID == node.souID && it->souLifespan == node.souLifespan) {
            find = true;
            it = vertices.erase(it);
        }
        else {
            deleteOutComingEdge(&(*it), node.souID, node.souLifespan);
            it++;
        }
    }
    if (!find) {
        throw "node not found";
    }
}

void IGraph::deleteNodeWhichInCycleAndIncludeTimestamp(const vector<SCCnode>& cycle, int timestamp) {
    auto it = vertices.begin();
    while (it != vertices.end()) {
        if (sccIncycle(it->souID, cycle) && it->souLifespan.test(timestamp)) {
            for (auto it2 = vertices.begin(); it2 != vertices.end(); it2++) {
                if(it2->souID != it->souID && it2->souLifespan != it->souLifespan)
                    deleteOutComingEdge(&(*it2), it->souID, it->souLifespan);
            } 
            it = vertices.erase(it);
        }
        else {
            it++;
        }
    }
}

void IGraph::deleteEmptyNode() {
    auto it = vertices.begin();
    while (it != vertices.end()) {
        if (it->firstArc == nullptr && !hasIncomeEdge(it->souID)) {
            it = vertices.erase(it);
        }
        else {
            it++;
        }
    }
}


#endif //IG_NOOP_5_INDEXGRAPH_H
