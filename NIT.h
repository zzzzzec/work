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
    Item(int id) : vertexID(id), partLab(0) {}
    bool operator()(const Item &cur) {
        return vertexID == cur.vertexID;
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

class NIT {
public:
    vector<RecordItem> NITable;
    vector<RefineRecordItem> RefineNITable;

}


NIT::NIT(){}
NIT::NIT(){
    
}

NIT::~NIT(){}

typedef struct vector<RecordItem> NodeInfoTable;
typedef struct vector<RefineRecordItem> RefineNITable;
void StoreRefineNITable(string storeRefineNIT, RefineNITable refineNITable);
RefineNITable ReadRefineNITable(string storeRefineNIT);



void StoreRefineNITable(string storeRefineNIT, RefineNITable refineNITable) {
    ofstream outfile(storeRefineNIT);
    if (outfile) {
        printf("Storeing the Refine_NI_Table......\n");
        for (auto item = refineNITable.begin(); item != refineNITable.end(); item++) {
            outfile << (*item).node << " # ";

            for (auto outIt = (*item).Out.begin(); outIt != (*item).Out.end(); outIt++) {
                outfile << (*outIt).vertexID << " - ";

                for (int i = 0; i < MNS; ++i) {
                    outfile << (*outIt).lifespan[i] << " ";
                }

                outfile << "@ " << (*outIt).partLab << " | ";
            }
            outfile << " /" << endl;
        }

    }
    outfile.close();
}

RefineNITable ReadRefineNITable(string storeRefineNIT) {
    RefineNITable refineNITable;

    vector<int> tmpVector;
    int souID = -1;
    int tarID = -1;
    int type = -1;
    bitset<MNS> lifetime;
    vector<Item> out;

    ifstream fin(storeRefineNIT);

    if (fin) {
        string str;
        printf("Reading the Refine NI-Table......\n");

        while (fin >> str) {
            if (str != "#" & str != "-" & str != "@" & str != "|" & str != "/") {
                int data = stoi(str);
                tmpVector.push_back(data);
            } else if (str == "#") {
                souID = tmpVector[0];
                tmpVector.clear();
            } else if (str == "-") {
                tarID = tmpVector[0];
                tmpVector.clear();
            } else if (str == "@") {
                lifetime.reset();

                for (int i = 0; i < tmpVector.size(); ++i) {
                    if (tmpVector[i] == 1) {
                        lifetime.set(i);
                    }
                }

                tmpVector.clear();
            } else if (str == "|") {
                type = tmpVector[0];
                tmpVector.clear();

                Item item;
                item.vertexID = tarID;
                item.lifespan = lifetime;
                item.partLab = type;
                out.push_back(item);

                tarID = -1;
                lifetime.reset();
                type = -1;
            } else if (str == "/") {
                RefineRecordItem refineRecordItem;
                refineRecordItem.node = souID;
                refineRecordItem.Out = out;
                refineNITable.push_back(refineRecordItem);

                out.clear();
                souID = -1;
            }
        }
    }

    return refineNITable;
}



#endif
