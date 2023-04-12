#ifndef IG_NOOP_5_SCC_TABLE_H
#define IG_NOOP_5_SCC_TABLE_H

#include "common.h"
#include "NIT.h"
#include "Lifespan.h"

using namespace std;

typedef struct SccID_Life {
    bitset<MNS> life_time;
    int scc_id;
} SccID_Life;

bool SortSccInNode(SccID_Life &a, SccID_Life &b) {
    int a1 = GetFirst1Pos(a.life_time);
    int b1 = GetFirst1Pos(b.life_time);
    return a1 < b1;
}

//SccTable:<{节点集合}，{{SCC ID+SCC 生存期}}>
//[std::set with 1 element = {[0] = 2}] = 
//{life_time = std::bitset = {[1] = 1}, scc_id = 3}
typedef set<int> NodeGroup;
typedef struct SCCTableItem {
    NodeGroup nodeGroup;
    mutable int SCCID;
    mutable bitset<MNS> lifeSpan;
    mutable SccID_Life sccID_Life;
    bool operator<(const SCCTableItem& a) const {
        if (a.nodeGroup == nodeGroup) {
            return false;
        }
        else {
            return nodeGroup.size() <= a.nodeGroup.size();
        }
    }
}SCCTableItem;

typedef set<SCCTableItem> SccTable;
typedef map<int, vector<SccID_Life>> OpSccTable;

typedef struct NodeEdge{
    int src;
    int dst;
    bool operator<(const NodeEdge& a) const{
        if (a.src != src) {
            return a.src > src;
        }
        else {
            return a.dst > dst;
        }
    }
}NodeEdge;

typedef struct sccEdge {
    int sScc;
    int tScc;
    bool operator<(const sccEdge& a) const {
        if (a.sScc != sScc) {
            return a.sScc > sScc;
        } else {
            return a.tScc > tScc;
        }
    }
}SCCEdge;

void StoreSccTable(string storeAddress, SccTable sccTable);

void StoreBuildSccTableTime(string storeAddress, double buildTime);

SccTable ReadSccTable(string storeAddress);

OpSccTable BuildOpSccTable(SccTable sccTable);

void StoreConstructTime(string storeAddress, double buildSCCTableTime, double buildIGTime);

int ReadBuildTime();


void StoreSccTable(string storeAddress, SccTable sccTable) {
    ofstream outfile(storeAddress);
    if (outfile) {
        printf("Storeing the SCC-Table......\n");
        for (auto item = sccTable.begin(); item != sccTable.end(); item++) {
            //存储SccTable中的vertices集合
            for (auto ver = (*item).nodeGroup.begin(); ver != (*item).nodeGroup.end(); ver++) {
                outfile << (*ver) << " ";
            }
            //存储SccTable中的Scc的Lifespan信息
            outfile << " # ";
            outfile << (*item).SCCID;
            outfile << " - ";
            for (int i = 0; i < MNS; ++i) {
                outfile << (*item).lifeSpan[i] << " ";
            }
            outfile << "| " << endl;
        }
    }
    outfile.close();
}

//删除在timeStamp时刻的SCC表中的SccID项
void DeleteSccTableByID(SccTable& sccTable, int sccID, int timeStamp) {
    for (auto item = sccTable.begin(); item != sccTable.end(); item++) {
        if ((*item).SCCID == sccID) {
            (*item).sccID_Life.life_time.set(timeStamp, false);
            if((*item).sccID_Life.life_time.none()){
                sccTable.erase(item);
            }
            break;
        }
    }
}

SccTable ReadSccTable(string storeAddress) {
    SccTable sccTable;

    vector<int> tmpVector;
    set<int> verticesSet;
    int sccID = -1;
    bitset<MNS> lifetime;

    ifstream fin(storeAddress);

    if (fin) {
        string str;
        printf("Reading the SCC-Table......\n");

        while (fin >> str) {

            if (str != "#" & str != "-" & str != "|") {
                int data = stoi(str);
                tmpVector.push_back(data);
            } else if (str == "#") {
                verticesSet.clear();

                for (auto it = tmpVector.begin(); it != tmpVector.end(); it++) {
                    int vectorData = (*it);
                    verticesSet.insert(vectorData);
                }

                tmpVector.clear();
            } else if (str == "-") {
                sccID = tmpVector[0];
                tmpVector.clear();
            } else if (str == "|") {
                lifetime.reset();

                for (int i = 0; i < tmpVector.size(); ++i) {
                    if (tmpVector[i] == 1) {
                        lifetime.set(i);
                    }
                }

                SCCTableItem item;
                item.nodeGroup = verticesSet;
                item.SCCID = sccID;
                item.lifeSpan = lifetime;
                sccTable.insert(item);
                sccID = -1;
                lifetime.reset();
                verticesSet.clear();
                tmpVector.clear();
            }

        }
    }
    fin.close();

    return sccTable;
}

void StoreBuildSccTableTime(string storeAddress, double buildTime) {
    ofstream outfile(storeAddress);
    if (outfile) {
        outfile << "Build SCC-Table Time is: " << buildTime / CLOCKS_PER_SEC << "s.";
    }
    outfile.close();
}

OpSccTable BuildOpSccTable(SccTable sccTable) {
    OpSccTable opSccTable;
    for (auto stIter = sccTable.begin(); stIter != sccTable.end(); stIter++) {
        set<int> nodeSet = (*stIter).nodeGroup;
        SccID_Life sccID_life = (*stIter).sccID_Life;
        int sccId = sccID_life.scc_id;
        for (auto nsIter = nodeSet.begin(); nsIter != nodeSet.end(); nsIter++) {
            int curNode = (*nsIter);
            auto siIter = opSccTable.find(curNode);
            if (siIter != opSccTable.end()) {
                (*siIter).second.push_back(sccID_life);
            } else {
                vector<SccID_Life> curSccID_Life;
                curSccID_Life.push_back(sccID_life);
                opSccTable.insert(pair<int, vector<SccID_Life>>(curNode, curSccID_Life));
            }
        }
    }

    return opSccTable;
}

void StoreConstructTime(string storeAddress, double buildSCCTableTime, double buildIGTime) {
    ofstream outfile(storeAddress);
    if (outfile) {
        outfile << "Build SCC-Table Time is: " << buildSCCTableTime / CLOCKS_PER_SEC << "s." << endl;
        outfile << "Build IndexGraph Time is: " << buildIGTime / CLOCKS_PER_SEC << "s." << endl;

    }
    outfile.close();

}

int newSCCID(SccTable st){
    int maxSCCID = 0;
    for(auto it = st.begin(); it != st.end(); it++){
        int curSCCID = (*it).sccID_Life.scc_id;
        if(curSCCID > maxSCCID){
            maxSCCID = curSCCID;
        }
    }
    return maxSCCID + 1;
}

#endif //IG_NOOP_5_SCC_TABLE_H
