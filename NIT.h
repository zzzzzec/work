#ifndef IG_NOOP_5_NIT_H
#define IG_NOOP_5_NIT_H

#include "common.h"
#include "Lifespan.h"

using namespace std;

//定义表中信息项：(节点，<生存期>)
class Item {
public:
    int vertexID;
    bitset<MNS> lifespan;
    /*partLab标签用于标识该记录被细化后所属的部分：
     * 0：该记录暂未被细化
     * 1：该记录属于Instant-Part——有入边
     * 2：该记录属于Interval-Part——无入边
    */
    int partLab;
public:
    Item() : vertexID(), lifespan(), partLab() {}
    Item(int id): vertexID(id), partLab(0) {}
    Item(int id, bitset<MNS> lifespan): vertexID(id), lifespan(lifespan), partLab(-1) {}
    Item(int id, bitset<MNS> lifespan, int partLab): vertexID(id), lifespan(lifespan), partLab(partLab) {}
    bool operator()(const Item& cur) {
        return vertexID == cur.vertexID;
    }
    bool operator==(const Item &cur) {
        return vertexID == cur.vertexID && lifespan == cur.lifespan;
    }
};

//定义表中每一行记录
class RecordItem {
public:
    int node;
    vector<Item> In;
    vector<Item> Out;
public:
    RecordItem() : node(), In(), Out() {}
    RecordItem(int id) : node(id) {}
    bool operator()(const RecordItem &cur) {
        return node == cur.node;
    }
};

class RefineRecordItem {
public:
    int node;
    vector<Item> Out;
public:
    RefineRecordItem() : node(), Out() {}
    RefineRecordItem(int id) : node(id) {}
    bool operator()(const RefineRecordItem &cur) {
        return node == cur.node;
    }
};
bool compareRecordItem(RecordItem a, RecordItem b) {
    return a.node < b.node;
}
bool compareRefineRecordItem(RefineRecordItem a, RefineRecordItem b) {
    return a.node < b.node;
}

typedef struct vector<RecordItem> NodeInfoTable;
typedef struct vector<RefineRecordItem> RefineNITable;

//定义排序：对Out项按照生存期长度降序排列
bool outItemSort(Item item1, Item item2);
//从读入的信息中提取出节点
vector<int> exFromNode(vector<int> con);
//从读入的信息中提取入节点
vector<int> exToNode(vector<int> con);
NodeInfoTable GetNITable(NodeInfoTable &nodeInfoTable, vector<vector<int>> &evolvingGraphSequence, int timeStamp);
RefineNITable GetRefineNITable(NodeInfoTable& nodeInfoTable);
RecordItem& findRecordItem(NodeInfoTable& nodeInfoTable, int node);
RefineRecordItem& findRefineRecordItem(RefineNITable& refineNITable, int node);

RecordItem& findRecordItem(NodeInfoTable& nodeInfoTable, int node) {
    for (auto item = nodeInfoTable.begin(); item != nodeInfoTable.end(); item++) {
        if ((*item).node == node) {
            return *item;
        }
    }
    throw "No such node in the NodeInfoTable!";
}

RefineRecordItem& findRefineRecordItem(RefineNITable& refineNITable, int node) {
    for (auto item = refineNITable.begin(); item != refineNITable.end(); item++) {
        if ((*item).node == node) {
            return *item;
        }
    }
    throw "No such node in the RefineNITable!";
}

bool outItemSort(Item item1, Item item2) {
    return item1.lifespan.count() > item2.lifespan.count();
}

vector<int> exFromNode(vector<int> con) {
    vector<int> outNodeVector;
    for (int i = 0; i < con.size(); i++) {
        if (i % 2 == 0) {
            outNodeVector.push_back(con[i]);
        }
    }
    return outNodeVector;
}

vector<int> exToNode(vector<int> con) {
    vector<int> inNodeVector;

    for (int i = 0; i < con.size(); i++) {
        if (i % 2 != 0) {
            inNodeVector.push_back(con[i]);
        }
    }
    return inNodeVector;
}
//SCC图就是一个DAG，evolvingGraphSequence包含了所有DAG图的信息
//在timespan时间戳下，从evergingGraphSequence中获取NITable
//NITable其实就是对于DAG的描述，每一个表项都有自己指向和指向自己的节点集合
NodeInfoTable GetNITable(NodeInfoTable &nodeInfoTable, vector<vector<int>> &evolvingGraphSequence, int timeStamp) {
    int curDAG_ID = timeStamp;          //DAG快照编号应从0开始
    vector<int> curDAG_edges = evolvingGraphSequence[curDAG_ID];
    vector<int> sourceNodeVector = exFromNode(curDAG_edges);
    vector<int> targetNodeVector = exToNode(curDAG_edges);
    //当前DAG快照中边数量
    int numOfItems = targetNodeVector.size();

    //逐边处理
    for (int i = 0; i < numOfItems; ++i) {
        //该边源节点
        int curSourceNode = sourceNodeVector[i];
        //该边目的节点
        int curTargetNode = targetNodeVector[i];
        //对源节点:
        //判断当前NITable中是否存在该源节点的记录
        auto sourcePos = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), RecordItem(curSourceNode));
        if (sourcePos != nodeInfoTable.end()) {
            //当前NITable中存在该源节点记录
            //判断该源节点记录Out项中是否有目的节点信息
            auto targetItemPos = find_if((*sourcePos).Out.begin(), (*sourcePos).Out.end(), Item(curTargetNode));
            if (targetItemPos != (*sourcePos).Out.end()) {
                //Out项中含有该目的节点信息，只需更新其生存期
                (*targetItemPos).lifespan.set(timeStamp);
            } else {
                //Out项中不含有该目的节点信息，需新增信息项
                //创建新的信息项
                Item item;
                item.vertexID = curTargetNode;
                item.lifespan.set(timeStamp);
                (*sourcePos).Out.push_back(item);
            }
        } else {
            //当前NITable中不存在该源节点记录
            //在表中增加新的节点记录条目(curSourceNode,In,Out)，并在其Out项中加入该边
            //构建新的记录
            RecordItem recordItem;
            recordItem.node = curSourceNode;

            Item item;
            item.vertexID = curTargetNode;
            item.lifespan.set(timeStamp);

            recordItem.Out.push_back(item);
            nodeInfoTable.push_back(recordItem);
        }

        //对目的节点:
        //判断当前NITable中是否存在该目的节点的记录
        auto targetPos = find_if(nodeInfoTable.begin(), nodeInfoTable.end(), RecordItem(curTargetNode));
        if (targetPos != nodeInfoTable.end()) {
            //当前NITable中存在该目的节点记录
            //判断该目的节点记录In项中是否有源节点信息
            auto sourceItemPos = find_if((*targetPos).In.begin(), (*targetPos).In.end(), Item(curSourceNode));
            if (sourceItemPos != (*targetPos).In.end()) {
                //In项中含有该源节点信息，只需更新其生存期
                (*sourceItemPos).lifespan.set(timeStamp);
            } else {
                //In项中不含有该源节点信息，添加该源节点的信息项
                Item item;
                item.vertexID = curSourceNode;
                item.lifespan.set(timeStamp);

                (*targetPos).In.push_back(item);
            }
        } else {
            //当前NITable中不存在该目的节点记录
            //在表中增加新的节点记录条目(curTargetNode,In,Out)，并在其In项中加入该边
            RecordItem recordItem;
            recordItem.node = curTargetNode;

            Item item;
            item.vertexID = curSourceNode;
            item.lifespan.set(timeStamp);

            recordItem.In.push_back(item);
            nodeInfoTable.push_back(recordItem);
        }
    }
    return nodeInfoTable;
}

RefineRecordItem getRefineRecordItem(RecordItem& ri) {
    RefineRecordItem refineRi;
    refineRi.node = ri.node;
    refineRi.Out.clear();
    if (!ri.In.empty()) {
        auto numOfInEdge = ri.In.size();
        auto numOfOutEdge = ri.Out.size();
        //有入边的时间集合
        bitset<MNS> unionOfIn;
        for (int i = 0; i < numOfInEdge; ++i) {
            bitset<MNS> cur_in = ri.In[i].lifespan;
            unionOfIn = LifespanUnion(unionOfIn, cur_in);
        }
        //对每条出边记录进行细化处理
        for (int j = 0; j < numOfOutEdge; ++j) {
            int curTarId = ri.Out[j].vertexID;
            bitset<MNS> cur_out = ri.Out[j].lifespan;
            bitset<MNS> instantPartLife = LifespanJoin(cur_out, unionOfIn);
            if (instantPartLife.none()) {
                //不存在Instant-Part记录,该记录为Interval-Part
                Item intervalRecord;
                intervalRecord.vertexID = curTarId;
                intervalRecord.lifespan = cur_out;
                intervalRecord.partLab = 2;
                refineRi.Out.push_back(intervalRecord);
            }
            else {
                bitset<MNS> intervalPartLife = LifespanDifference(cur_out, instantPartLife);
                //1.Interval-Part:
                if (intervalPartLife.any()) {
                    Item intervalRecord;
                    intervalRecord.vertexID = curTarId;
                    intervalRecord.lifespan = intervalPartLife;
                    intervalRecord.partLab = 2;
                    refineRi.Out.push_back(intervalRecord);
                }
                //2.Instant-Part:
                auto sizeOfInstantPart = instantPartLife.count();
                vector<int> vectorOfInstantLife = GetLifespanTruePos(instantPartLife);

                for (int k = 0; k < vectorOfInstantLife.size(); ++k) {
                    Item instantRecord;
                    bitset<MNS> itsLife;
                    itsLife.set(vectorOfInstantLife[k]);

                    instantRecord.vertexID = curTarId;
                    instantRecord.lifespan = itsLife;
                    instantRecord.partLab = 1;
                    refineRi.Out.push_back(instantRecord);
                }
            }
        }
    }
    else {
        for (auto it = ri.Out.begin(); it != ri.Out.end(); it++) {
            Item intervalRecord;
            intervalRecord.vertexID = (*it).vertexID;
            intervalRecord.lifespan = (*it).lifespan;
            intervalRecord.partLab = 2;
            refineRi.Out.push_back(intervalRecord);
        }
    }

    return refineRi;
}

RefineNITable GetRefineNITable(NodeInfoTable& nodeInfoTable) {
    RefineNITable refineNITable;
    for (auto record = nodeInfoTable.begin(); record != nodeInfoTable.end(); record++) {
        RefineRecordItem refineRecordItem = getRefineRecordItem(*record);
        refineNITable.push_back(refineRecordItem);
    }
    return refineNITable;
}

void deleteNITItemIN(RecordItem& ri, int timeStamp) {
    for (auto it = ri.In.begin(); it != ri.In.end();)
    {
        if (it->lifespan.test(timeStamp))
        {
            it->lifespan.set(timeStamp, false);
        }
        if (it->lifespan.count() == 0)
        {
            it = ri.In.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void deleteNITItemOut(RecordItem& ri, int timeStamp) {
    for (auto it = ri.Out.begin(); it != ri.Out.end();)
    {
        if (it->lifespan.test(timeStamp))
        {
            it->lifespan.set(timeStamp, false);
        }
        if (it->lifespan.count() == 0)
        {
            it = ri.Out.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
//这个函数用于删除NIT中id节点在timestamp时刻的IN和OUT中的记录
void deleteNITItem(RecordItem& ri, int timeStamp)
{
    deleteNITItemIN(ri, timeStamp);
    deleteNITItemOut(ri, timeStamp);
}

void deleteRefineNITItem(RefineRecordItem& rri, int timeStamp) {
    for (auto it = rri.Out.begin(); it != rri.Out.end(); ) {
        if (it->lifespan.test(timeStamp)) {
            it->lifespan.set(timeStamp, false);
        }
        if (it->lifespan.count() == 0)
        {
            it = rri.Out.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

//这个函数用于删除NIT中在timestamp时刻的IN和OUT中的记录s
void deleteNIT(NodeInfoTable &table, int timeStamp)
{
    for (auto record = table.begin(); record != table.end(); record++)
    {
        auto INit = record->In.begin();
        while (INit != record->In.end())
        {
            if (INit->lifespan.test(timeStamp))
            {
                INit->lifespan.set(timeStamp, false);
            }
            if (INit->lifespan.count() == 0)
            {
                INit = record->In.erase(INit);
            }
            else
            {
                ++INit;
            }
        }
        // 同理删掉out
        auto OUTit = record->Out.begin();
        while (OUTit != record->Out.end())
        {
            if (OUTit->lifespan.test(timeStamp))
            {
                OUTit->lifespan.set(timeStamp, false);
            }

            if (OUTit->lifespan.count() == 0)
            {
                OUTit = record->Out.erase(OUTit);
            }
            else
            {
                ++OUTit;
            }
        }
    }
}

//插入一个时刻为tx的节点，若在其他时刻已经存在，返回true，否则返回false
bool insertNITin(RecordItem& record, int id, int timeStamp) {
    for (auto it = record.In.begin(); it != record.In.end(); it++)
    {
        if(it->vertexID == id){
            it->lifespan.set(timeStamp);
            return true;
        }
    }
    Item item;
    item.vertexID = id;
    item.lifespan.set(timeStamp);
    record.In.push_back(item);
    return false;
}

//插入一个时刻为tx的节点，若在其他时刻已经存在，返回true，否则返回false
bool insertNITout(RecordItem & record, int id ,int timeStamp){
    for (auto it = record.Out.begin(); it != record.Out.end(); it++)
    {
        if(it->vertexID == id){
            it->lifespan.set(timeStamp);
            return true;
        }
    }
    Item item;
    item.vertexID = id;
    item.lifespan.set(timeStamp);
    record.Out.push_back(item);
    return false;
}

pair<bool, bool> refineNITINsertOrThrow(RefineRecordItem & ritem, const Item& i) {
    Item newItem;
    assert(i.lifespan.count() == 1);
    int pos = i.lifespan._Find_first();
    bool success = false;
    bool createNew = false;
    if (i.partLab == 1) {
        auto res = find_if(ritem.Out.begin(), ritem.Out.end(),
            [&](Item& a) {return (a.partLab == 1 && a.vertexID == i.vertexID); });
        assert(res == ritem.Out.end());
        newItem = i;
        ritem.Out.push_back(newItem);
        createNew = true;
        success = true;
    }
    else if (i.partLab == 2) {
        auto res = find_if(ritem.Out.begin(), ritem.Out.end(),
            [&](Item& a) {return (a.partLab == 2 && a.vertexID == i.vertexID); });
        if (res != ritem.Out.end()) {
            assert(!res->lifespan.test(pos));
            res->lifespan.set(pos, true);
        }
        else {
            newItem = i;
            ritem.Out.push_back(newItem);
            createNew = true;
        }
        success = true;
    }
    else {
        throw "partLab is not 1 or 2";
    }
    return make_pair(success, createNew);
}

void deleteN2(RefineRecordItem& rit) {
    auto it = rit.Out.begin();
    while (it != rit.Out.end()) {
        if (it->partLab == 2) {
            it = rit.Out.erase(it);
        }
        else {
            ++it;
        }
    }
}

#endif
