//
// Created by MoCuishle on 2019/11/29.
//

#ifndef IG_NOOP_5_PROCESS_QUERY_H
#define IG_NOOP_5_PROCESS_QUERY_H

#include <fstream>
#include <iostream>
#include <time.h>
#include <bitset>
#include <algorithm>
#include "IndexGraph.h"
#include "SCCTable.h"
#include "Lifespan.h"
#include "HRindex.h"

using namespace std;

/*
 * 查询文件说明：
 *      源节点 s | 目的节点 t | 查询类型 | 查询区间.起始 l | 查询区间.终止 k |
 * 查询类型说明：
 *      1：非连续区间查询：对于给定区间[l,k]，存在时刻j使得s,t可达即可；
 *      2：连续区间查询：对于给定区间[l,k]，任意时刻j均使得s,t可达。
 */
typedef struct QueryResult {
    int souID;
    int tarID;
    int query_b;
    int query_e;
    int type;
    int reach;
    QueryResult(int sou, int tar, int interBegin, int interEnd, int queryType, int r) {
        souID = sou;
        tarID = tar;
        query_b = interBegin;
        query_e = interEnd;
        type = queryType;
        reach = r;
    }
} QueryResult;

typedef struct ToGrail {
    int souID;
    int tarID;
    int reachability;
} ToGrail;

typedef struct QueryOnIG {
    int souSccId;
    int tarSccId;
    QueryOnIG(int s, int t) {
        souSccId = s;
        tarSccId = t;
    }
} QueryOnIG;

/*
    nodeID - 1 | SCCID - life|
           - 2 | SCCID - life|
           - 3 | SCCID - life|
           ...
    nodeID是原始图中的节点的ID，在一段时间内，可能会对应多个SCC
*/
typedef struct Node2Scc {
    int nodeID;
    vector<SccID_Life> vectorOfSccs;
    bool isFull;
} Node2Scc;

int findInNode2Scc(Node2Scc& node2Scc, int timeStamp) {
    for (int i = 0; i < node2Scc.vectorOfSccs.size(); i++) {
        if (node2Scc.vectorOfSccs[i].life_time.test(timeStamp) == 1) {
            return node2Scc.vectorOfSccs[i].scc_id;
        }
    }
    return -1;
}

vector<QueryResult> ReadQuery(string queryFileAddress);
//1和2的实现一致，区别在于1使用了SCCTable，2使用了OpSccTable
vector<ToGrail>
QueryReachabilityonIG(IGraph& IG, SccTable& sccTable, vector<QueryResult>& queryRecords, double& queryTime);
vector<QueryOnIG>
ProcessEGSDisQuery(IGraph& IG, SccTable& sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double& recordQT);
vector<QueryOnIG>
ProcessEGSConQuery(IGraph &IG, SccTable &sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT);
Node2Scc FindSccsOfNode(SccTable& sccTable, int nodeId, bitset<MNS> queryLife);

vector<ToGrail>
QueryReachabilityonIG2(IGraph &IG, OpSccTable &opSccTable, QueryResult& queryRecord, double &queryTime);
vector<QueryOnIG>
ProcessEGSDisQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT);
vector<QueryOnIG>
ProcessEGSConQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife, double& recordQT);
Node2Scc FindSccsOfNode2(OpSccTable& opSccTable, int nodeId, bitset<MNS> queryLife);

void StoreQueryRecords2Grail(string &resultFileAddress, vector<ToGrail> &toGrail, double &queryTime);

vector<QueryResult> ReadQuery(string queryFileAddress) {
    vector<QueryResult> queryRecords;
    ifstream fin(queryFileAddress);
    if (fin) {
        string str;
        while (getline(fin, str)) {
            int souID, tarID, query_begin, query_end, type, reachability;
            char *ch = (char *) str.c_str();
            sscanf(ch, "%d\t%d\t%d\t%d\t%d\t%d", &souID, &tarID, &query_begin, &query_end, &type, &reachability);
            QueryResult qr(souID, tarID, query_begin, query_end, type, reachability);
            queryRecords.push_back(qr);
        }
    }
    return queryRecords;
}

vector<QueryOnIG>
ProcessEGSDisQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT) {
    //Node2Scc 结构： nodeID ：原始图节点的ID， vectorOfSccs：属于的SCC的信息
    Node2Scc source = FindSccsOfNode2(opSccTable, souNode, queryLife);
    Node2Scc target = FindSccsOfNode2(opSccTable, tarNode, queryLife);
    vector<QueryOnIG> queryOnIG;
    /* 查询条目中若存在(-1,-1)，可直接返回true；若存在(-2,-2)，可直接返回false
     * 否则，对查询条目逐一处理：
     *      若其一返回true，则直接返回true；
     *      若均返回false，则最终返回false。*/
    clock_t query_startTime, query_endTime;
    query_startTime = clock();
    if(source.vectorOfSccs.size() == 0 || target.vectorOfSccs.size() == 0) {
        //查询记录中若存在(-2,-2)，可直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
        query_endTime = clock();
        recordQT += (double) (query_endTime - query_startTime) / CLOCKS_PER_SEC;
        return queryOnIG;
    }
    
    bool timeIntervalcrossFlag = false;

    for (auto souIter = source.vectorOfSccs.begin(); souIter != source.vectorOfSccs.end(); souIter++) {
        for (auto tarIter = target.vectorOfSccs.begin(); tarIter != target.vectorOfSccs.end(); tarIter++) {
            bitset<MNS> interLife = LifespanJoin((*souIter).life_time, (*tarIter).life_time);
            if (interLife.any()) {
                timeIntervalcrossFlag = true;
                if ((*souIter).scc_id == (*tarIter).scc_id) {
                    //重叠时间段内位于同一SCC，非连续区间查询完成查询：Yes
                    //非连续查询记录中若存在(-1,-1)，可直接返回true
                    QueryOnIG record(-1, -1);
                    queryOnIG.push_back(record);
                    return queryOnIG;
                }
                else {
                    //重叠时间段内位于不同SCC，记录souSCC与tarSCC及其区间
                    if (interLife.count() == 1) {
                        try {
                            int ID_sou = IG.FindIDonIG((*souIter).scc_id, interLife);
                            int ID_tar = IG.FindIDonIG((*tarIter).scc_id, interLife);
                            QueryOnIG record(ID_sou, ID_tar);
                            queryOnIG.push_back(record);
                        }
                        catch (...) {
                            continue;
                        }
                    }
                    else {
                        vector<int> interLifeTruePos = GetLifespanTruePos(interLife);
                        for (int i = 0; i < interLifeTruePos.size(); ++i) {
                            bitset<MNS> cur;
                            cur.set(interLifeTruePos[i]);//拆分查询区间
                            try {
                                int ID_sou = IG.FindIDonIG((*souIter).scc_id, cur);
                                int ID_tar = IG.FindIDonIG((*tarIter).scc_id, cur);
                                QueryOnIG record(ID_sou, ID_tar);
                                queryOnIG.push_back(record);
                            }
                            catch (...) {
                                continue;
                            }
                        }
                    }
                }
            }

        }
    }
    if (!timeIntervalcrossFlag) {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
    }
    query_endTime = clock();
    recordQT = (double)(query_endTime - query_startTime);
    if (queryOnIG.size() == 0) {
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
    }
    return queryOnIG;
}

vector<QueryOnIG>
ProcessEGSConQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife,
                    double &recordQT) {
    Node2Scc source = FindSccsOfNode2(opSccTable, souNode, queryLife);
    Node2Scc target = FindSccsOfNode2(opSccTable, tarNode, queryLife);
    vector<QueryOnIG> queryOnIG;
    clock_t query_startTime, query_endTime;
    query_startTime = clock();
    if(source.vectorOfSccs.size() == 0 || target.vectorOfSccs.size() == 0) {
        //查询记录中若存在(-2,-2)，可直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
        query_endTime = clock();
        recordQT += (double) (query_endTime - query_startTime) / CLOCKS_PER_SEC;
        return queryOnIG;
    }
    
    if (!source.vectorOfSccs.empty() && !target.vectorOfSccs.empty()) {
        if (!source.isFull || !target.isFull) {
            //sou或tar在查询区间内在某一时间段不存在，可立即回答可达性：No
            //连续查询记录中若存在(-2,-2)，可直接返回false
            QueryOnIG record(-2, -2);
            queryOnIG.push_back(record);
            return queryOnIG;
        } else {
            bool flag = true;
            for (auto souIter = source.vectorOfSccs.begin(); souIter != source.vectorOfSccs.end(); souIter++) {
                for (auto tarIter = target.vectorOfSccs.begin(); tarIter != target.vectorOfSccs.end(); tarIter++) {
                    bitset<MNS> interLife = LifespanJoin((*souIter).life_time, (*tarIter).life_time);
                    if (interLife.any()) {
                        //存在重叠时间段
                        if ((*souIter).scc_id == (*tarIter).scc_id) {
                            continue;
                        } else {
                            flag = false;
                            if (interLife.count() == 1) {
                                try {
                                    int ID_sou = IG.FindIDonIG((*souIter).scc_id, interLife);
                                    int ID_tar = IG.FindIDonIG((*tarIter).scc_id, interLife);
                                    QueryOnIG record(ID_sou, ID_tar);
                                    queryOnIG.push_back(record);
                                } catch (...) {
                                    QueryOnIG record(-2, -2);
                                    queryOnIG.push_back(record);
                                    return queryOnIG;
                                }
                            } else {
                                vector<int> interLifeTruePos = GetLifespanTruePos(interLife);
                                for (int i = 0; i < interLifeTruePos.size(); ++i) {
                                    bitset<MNS> cur;
                                    cur.set(interLifeTruePos[i]);
                                    try {
                                        int ID_sou = IG.FindIDonIG((*souIter).scc_id, cur);
                                        int ID_tar = IG.FindIDonIG((*tarIter).scc_id, cur);
                                        QueryOnIG record(ID_sou, ID_tar);
                                        queryOnIG.push_back(record);
                                    } catch (...) {
                                        QueryOnIG record(-2, -2);
                                        queryOnIG.push_back(record);
                                        return queryOnIG;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (flag) {
                //查询区间内各时间段sou和tar对应SCC均相等，可直接回答可达性：Yes
                QueryOnIG record(-1, -1);
                queryOnIG.push_back(record);
            }
            query_endTime = clock();
            recordQT = (double)(query_endTime - query_startTime);
            return queryOnIG;
        }
    } else {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
        query_endTime = clock();
        recordQT = (double)(query_endTime - query_startTime);
        return queryOnIG;
    }
    
}

Node2Scc FindSccsOfNode2(OpSccTable& opSccTable, int nodeId, bitset<MNS> queryLife) {
    Node2Scc node2Scc;
    node2Scc.nodeID = nodeId;
    node2Scc.vectorOfSccs.clear();
    bitset<MNS> haveFound;
    haveFound.reset();
    auto opSTIter = opSccTable.find(nodeId);
    if (opSTIter != opSccTable.end()) {
        for (auto vIter = (*opSTIter).second.begin(); vIter != (*opSTIter).second.end(); vIter++) {
            bitset<MNS> sccLife = (*vIter).life_time;
            bitset<MNS> interLife;
            interLife = LifespanJoin(queryLife, sccLife);
            if (interLife.any()) {
                SccID_Life sccID_life;
                sccID_life.scc_id = (*vIter).scc_id;
                sccID_life.life_time = interLife;
                node2Scc.vectorOfSccs.push_back(sccID_life);
                haveFound = LifespanUnion(haveFound, interLife);
            }
        }
    }
    else return node2Scc;
    bitset<MNS> judgeLife = LifespanDifference(queryLife, haveFound);
    node2Scc.isFull = judgeLife.none();
    sort(node2Scc.vectorOfSccs.begin(), node2Scc.vectorOfSccs.end(), SortSccInNode);
    return node2Scc;
}

vector<ToGrail>
QueryReachabilityonIG2(IGraph& IG, OpSccTable& opSccTable, QueryResult& queryRecord, double& queryTime) {
    vector<ToGrail> toGrail;
    int souNode = queryRecord.souID;
    int tarNode = queryRecord.tarID;
    int qBegin = queryRecord.query_b;
    int qEnd = queryRecord.query_e;
    int type = queryRecord.type;
    double recordQueryTime = 0;
    bitset<MNS> queryInterval;
    queryInterval = LifespanBuild(queryInterval, qBegin, qEnd);
    if (type == 1) {
        //非连续查询
        //(-1,-1):在某个时刻src和dst属于同一个SCC，直接返回true
        //(-2,-2):src和dst的存在时间没有交集，直接返回false
        vector<QueryOnIG> disjunctiveQuery = ProcessEGSDisQuery2(IG, opSccTable, souNode, tarNode, queryInterval, recordQueryTime);
        if (disjunctiveQuery.empty()) printf("Result Set is empty\n");
        //QueryOnIG中存放的是下标
        for (auto record = disjunctiveQuery.begin(); record != disjunctiveQuery.end(); record++) {
            ToGrail record2Grail;
            record2Grail.souID = (*record).souSccId;
            record2Grail.tarID = (*record).tarSccId;
            record2Grail.reachability = 1;
            toGrail.push_back(record2Grail);
        }
    }
    else {
        //连续查询
        //（-1，-1）：在每个时刻src和dst都在一个SCC中，返回true
        //（-2，-2）：src和dst至少有一个在查询区间内的某一时刻不存在，返回false
        vector<QueryOnIG> conjunctiveQuery = ProcessEGSConQuery2(IG, opSccTable, souNode, tarNode, queryInterval, recordQueryTime);
        for (auto record = conjunctiveQuery.begin(); record != conjunctiveQuery.end(); record++) {
            ToGrail record2Grail;
            record2Grail.souID = (*record).souSccId;
            record2Grail.tarID = (*record).tarSccId;
            record2Grail.reachability = 1;
            toGrail.push_back(record2Grail);
        }
    }
    queryTime += recordQueryTime;
    return toGrail;
}


void
StoreQueryRecords2Grail(string &resultFileAddress, vector<ToGrail> &toGrail, double &queryTime) {
    ofstream outfile(resultFileAddress);
    if (outfile) {
        printf("Storing the query records...\n");
        for (auto record = toGrail.begin(); record != toGrail.end(); record++) {
            int sou = (*record).souID;
            int tar = (*record).tarID;
            int reach = (*record).reachability;
            outfile << sou << " " << tar << " " << reach << endl;
        }
        outfile << "-----------------" << endl;
        outfile << "Query Time is: " << queryTime / CLOCKS_PER_SEC << " s / 1000 query records." << endl;
        printf("Finished~\n");
    }
    outfile.close();
}

/*

vector<ToGrail>
QueryReachabilityonIG(IGraph &IG, SccTable &sccTable, vector<QueryResult> &queryRecords, double &queryTime) {
    vector<ToGrail> toGrail;
    for (int recordItem = 0; recordItem < queryRecords.size(); ++recordItem) {
        int souNode = queryRecords[recordItem].souID;
        int tarNode = queryRecords[recordItem].tarID;
        int qBegin = queryRecords[recordItem].query_b;
        int qEnd = queryRecords[recordItem].query_e;
        int type = queryRecords[recordItem].type;
        double recordQueryTime = 0;
        bitset<MNS> queryInterval;
        queryInterval = LifespanBuild(queryInterval, qBegin, qEnd);
        //非连续查询
        if (type == 1) {
            vector<QueryOnIG> disjunctiveQuery = ProcessEGSDisQuery(IG, sccTable, souNode, tarNode, queryInterval, recordQueryTime);
            if (disjunctiveQuery.empty()) printf("Result Set is empty\n");
            int flag = 1;
            for (auto record = disjunctiveQuery.begin(); record != disjunctiveQuery.end(); record++) {
                if ((*record).souSccId == -1 && (*record).tarSccId == -1) {
                    flag = 0;
                } else if ((*record).souSccId == -2 && (*record).tarSccId == -2) {
                    flag = 0;
                }
            }
            if (flag == 1) {
                for (auto record = disjunctiveQuery.begin(); record != disjunctiveQuery.end(); record++) {
                    ToGrail record2Grail;
                    record2Grail.souID = (*record).souSccId;
                    record2Grail.tarID = (*record).tarSccId;
                    record2Grail.reachability = 1;
                    toGrail.push_back(record2Grail);
                }
            }
        } else {
            //连续查询
            vector<QueryOnIG> conjunctiveQuery = ProcessEGSConQuery(IG, sccTable, souNode, tarNode, queryInterval, recordQueryTime);
            int flag = 1;
            for (auto record = conjunctiveQuery.begin(); record != conjunctiveQuery.end(); record++) {
                if ((*record).souSccId == -1 && (*record).tarSccId == -1) {
                    flag = 0;
                } else if ((*record).souSccId == -2 && (*record).tarSccId == -2) {
                    flag = 0;
                }
            }
            if (flag == 1) {
                for (auto record = conjunctiveQuery.begin(); record != conjunctiveQuery.end(); record++) {
                    ToGrail record2Grail;
                    record2Grail.souID = (*record).souSccId;
                    record2Grail.tarID = (*record).tarSccId;
                    record2Grail.reachability = 1;
                    toGrail.push_back(record2Grail);
                }
            }
        }
        queryTime += recordQueryTime;
    }
    return toGrail;
}
vector<QueryOnIG>
ProcessEGSDisQuery(IGraph &IG, SccTable &sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT) {

    Node2Scc source = FindSccsOfNode(sccTable, souNode, queryLife);
    Node2Scc target = FindSccsOfNode(sccTable, tarNode, queryLife);
    vector<QueryOnIG> queryOnIG;
    clock_t query_startTime, query_endTime;
    query_startTime = clock();

    if (!source.vectorOfSccs.empty() && !target.vectorOfSccs.empty()) {
        for (auto souIter = source.vectorOfSccs.begin(); souIter != source.vectorOfSccs.end(); souIter++) {
            for (auto tarIter = target.vectorOfSccs.begin(); tarIter != target.vectorOfSccs.end(); tarIter++) {
                bitset<MNS> interLife = LifespanJoin((*souIter).life_time, (*tarIter).life_time);
                if (interLife.any()) {
                    if ((*souIter).scc_id == (*tarIter).scc_id) {
                        //重叠时间段内位于同一SCC，非连续区间查询完成查询：Yes
                        //非连续查询记录中若存在(-1,-1)，可直接返回true
                        QueryOnIG record(-1, -1);
                        queryOnIG.push_back(record);
                        return queryOnIG;
                    }
                    else {
                        //重叠时间段内位于不同SCC，记录souSCC与tarSCC及其区间
                        if (interLife.count() == 1) {
                            try {
                                int ID_sou = IG.FindIDonIG((*souIter).scc_id, interLife);
                                int ID_tar = IG.FindIDonIG((*tarIter).scc_id, interLife);
                                QueryOnIG record(ID_sou, ID_tar);
                                queryOnIG.push_back(record);
                            }
                            catch (...) {
                                continue;
                            }
                        }
                        else {
                            vector<int> interLifeTruePos = GetLifespanTruePos(interLife);
                            for (int i = 0; i < interLifeTruePos.size(); ++i) {
                                bitset<MNS> cur;
                                cur.set(interLifeTruePos[i]);
                                try {
                                    int ID_sou = IG.FindIDonIG((*souIter).scc_id, cur);
                                    int ID_tar = IG.FindIDonIG((*tarIter).scc_id, cur);
                                    QueryOnIG record(ID_sou, ID_tar);
                                    queryOnIG.push_back(record);
                                }
                                catch (...) {
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
    }
    return queryOnIG;
}

vector<QueryOnIG> ProcessEGSConQuery(IGraph& IG, SccTable& sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double& recordQT) {
    Node2Scc source = FindSccsOfNode(sccTable, souNode, queryLife);
    Node2Scc target = FindSccsOfNode(sccTable, tarNode, queryLife);
    vector<QueryOnIG> queryOnIG;
    clock_t query_startTime, query_endTime;
    query_startTime = clock();

    if (!source.vectorOfSccs.empty() && !target.vectorOfSccs.empty()) {
        if (!source.isFull || !target.isFull) {
            //sou或tar在查询区间内在某一时间段不存在，可立即回答可达性：No
            //连续查询记录中若存在(-2,-2)，可直接返回false
            QueryOnIG record(-2, -2);
            queryOnIG.push_back(record);

            return queryOnIG;
        }
        else {
            bool flag = true;

            for (auto souIter = source.vectorOfSccs.begin(); souIter != source.vectorOfSccs.end(); souIter++) {
                for (auto tarIter = target.vectorOfSccs.begin(); tarIter != target.vectorOfSccs.end(); tarIter++) {
                    bitset<MNS> interLife = LifespanJoin((*souIter).life_time, (*tarIter).life_time);

                    if (interLife.any()) {
                        if ((*souIter).scc_id == (*tarIter).scc_id) {
                            continue;
                        }
                        else {
                            flag = false;

                            if (interLife.count() == 1) {
                                try {
                                    int ID_sou = IG.FindIDonIG((*souIter).scc_id, interLife);
                                    int ID_tar = IG.FindIDonIG((*tarIter).scc_id, interLife);

                                    QueryOnIG record(ID_sou, ID_tar);
                                    queryOnIG.push_back(record);
                                }
                                catch (...) {
                                    QueryOnIG record(-2, -2);
                                    queryOnIG.push_back(record);

                                    return queryOnIG;
                                }
                            }
                            else {
                                vector<int> interLifeTruePos = GetLifespanTruePos(interLife);
                                for (int i = 0; i < interLifeTruePos.size(); ++i) {
                                    bitset<MNS> cur;
                                    cur.set(interLifeTruePos[i]);

                                    try {
                                        int ID_sou = IG.FindIDonIG((*souIter).scc_id, cur);
                                        int ID_tar = IG.FindIDonIG((*tarIter).scc_id, cur);

                                        QueryOnIG record(ID_sou, ID_tar);
                                        queryOnIG.push_back(record);
                                    }
                                    catch (...) {
                                        QueryOnIG record(-2, -2);
                                        queryOnIG.push_back(record);

                                        return queryOnIG;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (flag) {
                //查询区间内各时间段sou和tar对应SCC均相等，可直接回答可达性：Yes
                QueryOnIG record(-1, -1);
                queryOnIG.push_back(record);
            }
            query_endTime = clock();
            recordQT = (double)(query_endTime - query_startTime);
            return queryOnIG;

        }
    }
    else {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);

        query_endTime = clock();
        recordQT = (double)(query_endTime - query_startTime);

        return queryOnIG;
    }
}
Node2Scc FindSccsOfNode(SccTable &sccTable, int nodeId, bitset<MNS> queryLife) {
    Node2Scc node2Scc;
    node2Scc.nodeID = nodeId;
    bitset<MNS> haveFound;
    haveFound.reset();
    for (auto sccTableIter = sccTable.begin(); sccTableIter != sccTable.end(); sccTableIter++) {
        auto nodePos = (*sccTableIter).nodeGroup.find(nodeId);
        if (nodePos != (*sccTableIter).nodeGroup.end()) {
            //该行记录的SCC含有此节点，判断其生存期是否有交集:
            bitset<MNS> sccLife = (*sccTableIter).sccID_Life.life_time;
            bitset<MNS> interLife;
            interLife = LifespanJoin(queryLife, sccLife);
            if (interLife.any()) {
                SccID_Life sccID_life;
                sccID_life.scc_id = (*sccTableIter).sccID_Life.scc_id;
                sccID_life.life_time = interLife;
                node2Scc.vectorOfSccs.push_back(sccID_life);
                haveFound = LifespanUnion(haveFound, interLife);
            }
        }
    }
    //这里比较节点在查询周期内的存在情况，如果在查询周期内都存在，则isFull为true
    bitset<MNS> judgeLife = LifespanDifference(queryLife, haveFound);
    node2Scc.isFull = judgeLife.none();
    sort(node2Scc.vectorOfSccs.begin(), node2Scc.vectorOfSccs.end(), SortSccInNode);
    return node2Scc;
}
*/

#endif //IG_NOOP_5_PROCESS_QUERY_H
