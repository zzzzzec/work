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

//定义索引图弧结构
typedef struct IGarc {
    int taruuid;
    int tarID;                          //目的节点ID
    bitset<MNS> tarLifespan;            //目的节点Lifespan
    IGarc* nextarc;                 //指向下一条弧的指针
    IGarc() { nextarc = NULL; }
    IGarc(int id, int v, bitset<MNS> t) {
        taruuid = id;
        tarID = v;
        tarLifespan = t;
        nextarc = NULL;
    }
} IGarc;

//定义索引图顶点结构
typedef struct IGnode {
    
    int uuid;
    int souID;
    bitset<MNS> souLifespan; 
    int sccOfIG;                        //标识源节点位于IG中哪个SCC
    IGarc *firstArc;

    IGnode(int id, int u, bitset<MNS> t) {
        uuid = id;
        souID = u;
        souLifespan = t;
        sccOfIG = 0;
        firstArc = NULL;
    }
    
    bool operator==(const IGnode& node) const {
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
} IGnode;
typedef struct vector<IGnode> AdjList;

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
    
    void InsertEdgeWithoutCheck(int souPos, int tarID, bitset<MNS> tarLife);
    void InsertEdge(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void InsertEdgeOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void InsertEdgeSrcMustExistOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    
    void DeleteEdge1(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void DeleteEdge2(int souPos, int tarPos);
    void DeleteEdgeKeepEmptyNode(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);

    void deleteOutComingEdge(IGnode* node, int tarID, bitset<MNS> tarLife);
    void deleteNodeOrThrow(const IGnode node);
    void deleteNodeWhichInCycleAndIncludeTimestamp(const vector<SCCnode>& cycle, int timestamp);
    void deleteEmptyNode();
    
    void CreateVertex(int ID, bitset<MNS> t);
    vector<CopyNode> FindAllCopyNodes(int souID);
    int FindIDonIG(int sccID, bitset<MNS> lifespan);           //根据SCC的id及其生存期，确定其在索引图上的ID
    IGnode* findNode(int id, Lifespan lifespan);
    IGnode* findNodeByPos(int pos) { return &vertices[pos]; }
    
    bool HaveSuperLifespan(vector<CopyNode>& copyNodes, bitset<MNS> t);

    void ProTarget(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife);
    void MaintainRelationship(int verID, bitset<MNS> verLifespan);
    void rebuildCase3();
    void StoreFullIndexGraphJSON(string dir);
    void ConstructOutEdge(int souID, int tarID, bitset<MNS> t, int label, bitset<MNS> intervalUnion);
    
    void ModifyNodeOrThrow(int id, Lifespan lifespan, Lifespan newLifespan);

    void updateAddRefineRecord(const RefineRecordItem& ritem);

    

    bitset<MNS> GetSubVertexUnionLife(int verPos);
    vector<int> GetSouVerticesPos(int tarID, bitset<MNS> tarLife);
    vector<int> GetSubVerticesPos(int tarPos);
    void IDremap(map<int, int> m);
    
    bool isReachable(int u, int v) {
        map<int, bool> visited;
        return isReachableUtil(u, v, visited);
    }

private:
    bool isReachableUtil(const int u, const int v, map<int, bool>& visited) {
        if (u == v) return true;
        visited[vertices[u].uuid] = true;
        for (IGarc* arc = vertices[u].firstArc; arc != NULL; arc = arc->nextarc) {
            int arcPos = VerPos(arc->tarID, arc->tarLifespan);
            if (!visited[vertices[arcPos].uuid] && isReachableUtil(arcPos, v, visited)) return true;
        }
        return false;
    }
    bool hasIncomeEdge(int souID, Lifespan life);
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

//直接插入，如果已经存在则直接返回
void IGraph::InsertEdgeWithoutCheck(int souPos, int tarID, bitset<MNS> tarLife) {
    //尾插法
    int taruuid = -1;
    for (auto node : vertices) {
        if (node.souID == tarID && node.souLifespan == tarLife) {
            taruuid = node.uuid;
            break;
        }
    }
    assert(taruuid != -1);
    IGarc* newArcNode = new IGarc(taruuid ,tarID, tarLife);
    IGarc *temp = vertices[souPos].firstArc;

    if (temp == NULL) {
        //temp为NULL
        vertices[souPos].firstArc = newArcNode;
        edgenum++;
        return;
    } else {
        while (temp->nextarc) {
            //temp非NULL
            if (temp->tarID == tarID && temp->tarLifespan == tarLife) return;
            temp = temp->nextarc;
        }
        if (temp->tarID == tarID && temp->tarLifespan == tarLife) return;
        temp->nextarc = newArcNode;
        edgenum++;
    }
}

void IGraph::InsertEdgeOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);
    if (souPos == -1 || tarPos == -1)
        throw "node not exist";
    InsertEdgeWithoutCheck(souPos, tarID, tarLife);
}

void IGraph::InsertEdgeSrcMustExistOrThrow(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    LOG << __FUNCTION__ << ": <" << souID << "," << souLife.to_string() << "> -> <" << tarID << "," << tarLife.to_string() << ">" << endl;
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);
    if (souPos == -1)
        throw "InsertEdgeSrcMustExistOrThrow: ";
    if (tarPos == -1) {
        int newid = Newuuid();
        IGnode newTarNode(newid, tarID, tarLife);
        vertices.push_back(newTarNode);
        vexnum++;
    }
    InsertEdgeWithoutCheck(souPos, tarID, tarLife);
}

void IGraph::InsertEdge(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife) {
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);

    if (tarPos == -1) {
        int newid = Newuuid();
        IGnode newTarNode(newid, tarID, tarLife);
        vertices.push_back(newTarNode);
        vexnum++;
    }
    if (souPos != -1) {
        //当前存在节点(souID,souLife)
        InsertEdgeWithoutCheck(souPos, tarID, tarLife);
    } else {
        //当前不存在节点(souID,souLife)，需创建该节点
        int newid = Newuuid();
        IGnode newSouNode(newid, souID, souLife);
        vertices.push_back(newSouNode);
        vexnum++;
        souPos = vertices.size() - 1;
        InsertEdgeWithoutCheck(souPos, tarID, tarLife);
    }
}

void IGraph::DeleteEdgeKeepEmptyNode(int souID, bitset<MNS> souLife, int tarID, bitset<MNS> tarLife)
{
    LOG << __FUNCTION__ << ": <" << souID << "," << souLife.to_string() <<"> -> <" << tarID << "," << tarLife.to_string() << ">" << endl;
    int souPos = VerPos(souID, souLife);
    int tarPos = VerPos(tarID, tarLife);

    if (souPos == -1 || tarPos == -1) {
        throw "Don't exist this sourceNode!";
    }
    else {
        bool deleteFlag = false;
        IGarc* current = vertices[souPos].firstArc;
        IGarc* pre = NULL;
        if (current == NULL) {
            throw "Don't exist this edge";
        }
        else {
            while (current != NULL) {
                if (current->tarID == tarID && current->tarLifespan == tarLife) {
                    if (pre == NULL) {
                        deleteFlag = true;
                        vertices[souPos].firstArc = current->nextarc;
                        delete current;
                        edgenum--;
                        break;
                    }
                    else {
                        deleteFlag = true;
                        pre->nextarc = current->nextarc;
                        delete current;
                        edgenum--;
                        break;
                    }
                }
                else {
                    pre = current;
                    current = current->nextarc;
                }
            }
        }
        if (!deleteFlag) {
            throw "Don't exist this edge";
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
        IGarc *p = vertices[souPos].firstArc;
        IGarc *q = NULL;

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
    IGarc *p = vertices[souPos].firstArc;
    IGarc *q = NULL;

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
    IGnode newVerNode(newid, ID, t);
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
void IGraph::rebuildCase3() {
    for (auto it : vertices) {
        MaintainRelationship(it.souID, it.souLifespan);
    }
}
//对应case3，加入边(<souID,L1>,<souID,L2>), L1属于L2或者L2属于L1，创建新节点的时候调用
void IGraph::MaintainRelationship(int verID, bitset<MNS> verLifespan) {
    //找到当前u的所有拷贝,将其存入copyNodes中
    vector<CopyNode> copyNodes = FindAllCopyNodes(verID);
    sort(copyNodes.begin(), copyNodes.end(), copyNodeSort);
    for (int j = 0; j < copyNodes.size(); ++j) {
        bitset<MNS> curLife = copyNodes[j].lifespan;
        if (LifespanisProSub(verLifespan, curLife)) {
            //(verID,curLife)是(verID,verLifespan)的子节点
            //必须都是已经存在的节点
            InsertEdgeOrThrow(verID, verLifespan, verID, curLife);
            //InsertEdge(verID, verLifespan, verID, curLife);
        }
        else if (LifespanisProSub(curLife, verLifespan)) {
            //(verID,verLifespan)是(verID,curLife)的子节点
            //必须都是已经存在的节点
            InsertEdgeOrThrow(verID, curLife, verID, verLifespan);
            //InsertEdge(verID, curLife, verID, verLifespan);
        }
    }
}

void IGraph::StoreFullIndexGraphJSON(string dir) {
    Json::Value JsonGraph;
    for (auto node : vertices) {
        Json::Value JsonNode;
        JsonNode["uuid"] = node.uuid;
        JsonNode["souID"] = node.souID;
        JsonNode["souLifespan"] = node.souLifespan.to_string();
        Json::Value JsonArcs;
        for (IGarc *p = node.firstArc; p; p = p->nextarc) {
            Json::Value JsonArc;
            JsonArc["uuid"] = p->taruuid;
            JsonArc["tarID"] = p->tarID;
            JsonArc["tarLifespan"] = p->tarLifespan.to_string();
            JsonArcs.append(JsonArc);
        }
        JsonNode["arcs"] = JsonArcs;
        JsonGraph.append(JsonNode);
    }
    ofstream fout(dir + "IG.json");
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
    IGarc *p = vertices[verPos].firstArc;

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
        IGarc *p = vertices[curPos].firstArc;

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
    IGarc *p = vertices[tarPos].firstArc;

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

IGnode* IGraph::findNode(int id , Lifespan Lifespan) {
    for (int i = 0; i < vertices.size(); i++) {
        IGnode* tmp = &(vertices[i]);
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

bool IGraph::hasIncomeEdge(int souID, Lifespan life) {
    for (auto node : vertices) {
        auto edgeit = node.firstArc;
        while (edgeit != NULL) {
            if (edgeit->tarID== souID && edgeit->tarLifespan == life)
            {
                return true;
            }
            edgeit = edgeit->nextarc;
        }
    }
    return false;
}

void IGraph::deleteOutComingEdge(IGnode* node, int tarID, bitset<MNS> tarLife) {
    IGarc* p = node->firstArc;
    IGarc* q = NULL;
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

void IGraph::deleteNodeOrThrow(const IGnode node) {
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

//这个函数将case3创建的边视为不存在
void IGraph::deleteEmptyNode() {
    auto it = vertices.begin();
    while (it != vertices.end()) {
        IGarc* p = it->firstArc;
        bool deleteFlag = true;
        while (p != NULL) {
            if (p->tarID != it->souID) {
                deleteFlag = false;
                break;
            }
            p = p->nextarc;
        }
        //除了case3出边之外没有出边
        for (const auto node : vertices) {
            //souID相同必是case3中的边
            if (node.souID != it->souID) {
                auto edgeit = node.firstArc;
                while (edgeit != NULL) {
                    if (edgeit->tarID == it->souID && edgeit->tarLifespan == it->souLifespan) {
                        deleteFlag = false;
                        break;
                    }
                    edgeit = edgeit->nextarc;
                }
            }
        }
        if (deleteFlag) {
            LOG << __FUNCTION__ << ": delete node " << it->souID << endl;
            it = vertices.erase(it);
        }
        else {
            it++;
        }
    }
}

void IGraph::IDremap(map<int, int> m) {
    for (auto &it : vertices) {
        it.souID = m[it.souID];
        for (auto it2 = it.firstArc; it2 != nullptr; it2 = it2->nextarc) {
            it2->tarID = m[it2->tarID];
        }
    }
}

#endif //IG_NOOP_5_INDEXGRAPH_H
