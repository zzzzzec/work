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
    NodeEdge(int s, int d):src(s), dst(d) {}
    bool operator<(const NodeEdge& a) const {
        if (a.src != src) {
            return a.src > src;
        }
        else {
            return a.dst > dst;
        }
    }
    bool operator==(const NodeEdge& a) const {
        return a.src == src && a.dst == dst;
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


OpSccTable BuildOpSccTable(SccTable sccTable);

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
