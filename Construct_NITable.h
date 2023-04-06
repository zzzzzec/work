#ifndef IG_NOOP_5_CONSTRUCT_NITABLE_H
#define IG_NOOP_5_CONSTRUCT_NITABLE_H

#include "common.h"
#include "Lifespan.h"
#include "NIT.h"

using namespace std;

//定义排序：对Out项按照生存期长度降序排列
bool outItemSort(Item item1, Item item2);

//从读入的信息中提取出节点
vector<int> exFromNode(vector<int> con);

//从读入的信息中提取入节点
vector<int> exToNode(vector<int> con);

//获取NI-Table
NodeInfoTable GetNITable(NodeInfoTable &nodeInfoTable, vector<vector<int>> &evolvingGraphSequence, int timeStamp);

//对NITable出边记录进行细化
RefineNITable GetRefineNITable(NodeInfoTable nodeInfoTable);

//-------------------------------------------------------

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
    int curDAG_ID = timeStamp - 1;          //DAG快照编号应从0开始
    vector<int> curDAG_edges = evolvingGraphSequence[curDAG_ID];
    vector<int> sourceNodeVector = exFromNode(curDAG_edges);
    vector<int> targetNodeVector = exToNode(curDAG_edges);
    //当前DAG快照中边数量
    int numOfItems = targetNodeVector.size();
    printf("=============== Getting NI-Table... ===============\n");

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

RefineNITable GetRefineNITable(NodeInfoTable nodeInfoTable) {
    RefineNITable refineNITable;

    for (auto record = nodeInfoTable.begin(); record != nodeInfoTable.end(); record++) {

        RefineRecordItem refineRecordItem;
        refineRecordItem.node = (*record).node;

        //该节点有入边
        if (!(*record).In.empty()) {
            auto numOfInEdge = (*record).In.size();
            auto numOfOutEdge = (*record).Out.size();
            //有入边的时间集合
            bitset<MNS> unionOfIn;
            for (int i = 0; i < numOfInEdge; ++i) {
                bitset<MNS> cur_in = (*record).In[i].lifespan;
                unionOfIn = LifespanUnion(unionOfIn, cur_in);
            }

            //对每条出边记录进行细化处理
            for (int j = 0; j < numOfOutEdge; ++j) {
                int curTarId = (*record).Out[j].vertexID;
                bitset<MNS> cur_out = (*record).Out[j].lifespan;

                bitset<MNS> instantPartLife = LifespanJoin(cur_out, unionOfIn);

                if (instantPartLife.none()) {
                    //不存在Instant-Part记录,该记录为Interval-Part
                    Item intervalRecord;
                    intervalRecord.vertexID = curTarId;
                    intervalRecord.lifespan = cur_out;
                    intervalRecord.partLab = 2;

                    refineRecordItem.Out.push_back(intervalRecord);
                } else {
                    bitset<MNS> intervalPartLife = LifespanDifference(cur_out, instantPartLife);

                    //1.Interval-Part:
                    if (intervalPartLife.any()) {
                        Item intervalRecord;
                        intervalRecord.vertexID = curTarId;
                        intervalRecord.lifespan = intervalPartLife;
                        intervalRecord.partLab = 2;

                        refineRecordItem.Out.push_back(intervalRecord);
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
                        refineRecordItem.Out.push_back(instantRecord);
                    }
                }
            }
        } else {

            for (auto it = (*record).Out.begin(); it != (*record).Out.end(); it++) {
                Item intervalRecord;
                intervalRecord.vertexID = (*it).vertexID;
                intervalRecord.lifespan = (*it).lifespan;
                intervalRecord.partLab = 2;

                refineRecordItem.Out.push_back(intervalRecord);
            }

        }

        refineNITable.push_back(refineRecordItem);
    }

    return refineNITable;
}

//这个函数用于删除NIT中在timestamp时刻的IN和OUT中的记录
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

void insertNITin(RecordItem & record, int id ,int timeStamp){
    for (auto it = record.In.begin(); it != record.In.end(); it++)
    {
        if(it->vertexID == id){
            it->lifespan.set(timeStamp);
            return;
        }
    }
    Item item;
    item.vertexID = id;
    item.lifespan.set(timeStamp);
    record.In.push_back(item);
    return;
}

void insertNITout(RecordItem & record, int id ,int timeStamp){
    for (auto it = record.Out.begin(); it != record.Out.end(); it++)
    {
        if(it->vertexID == id){
            it->lifespan.set(timeStamp);
            return;
        }
    }
    Item item;
    item.vertexID = id;
    item.lifespan.set(timeStamp);
    record.Out.push_back(item);
    return;
}

#endif //IG_NOOP_5_CONSTRUCT_NITABLE_H
