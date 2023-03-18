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
#include "SCC_Table.h"
#include "Lifespan.h"

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

    QueryResult() {
        souID = -1;
        tarID = -1;
        query_b = -1;
        query_e = -1;
        type = 0;
        reach = 0;
    }

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

    QueryOnIG() {
        souSccId = -1;
        tarSccId = -1;
    }

    QueryOnIG(int s, int t) {
        souSccId = s;
        tarSccId = t;
    }
} QueryOnIG;

typedef struct Node2Scc {
    int nodeID;
    vector<SccID_Life> vectorOfSccs;

    bool isFull;
} Node2Scc;


vector<QueryResult> ReadQuery(string queryFileAddress);

vector<ToGrail>
QueryReachabilityonIG(IGraph &IG, SccTable &sccTable, vector<QueryResult> &queryRecords, double &queryTime);

vector<ToGrail>
QueryReachabilityonIG2(IGraph &IG, OpSccTable &opSccTable, vector<QueryResult> &queryRecords, double &queryTime);

vector<QueryOnIG>
ProcessEGSDisQuery(IGraph &IG, SccTable &sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT);

vector<QueryOnIG>
ProcessEGSConQuery(IGraph &IG, SccTable &sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT);

Node2Scc FindSccsOfNode(SccTable &sccTable, int nodeId, bitset<MNS> queryLife);

Node2Scc FindSccsOfNode2(OpSccTable &opSccTable, int nodeId, bitset<MNS> queryLife);

void StoreQueryRecords2Grail(string &resultFileAddress, vector<ToGrail> &toGrail, double &queryTime);

vector<QueryOnIG>
ProcessEGSDisQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife,
                    double &recordQT);

vector<QueryOnIG>
ProcessEGSConQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife,
                    double &recordQT);
//---------------------------------------------------------------

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

        if (type == 1) {
            //非连续查询
            printf("\t Finding Disjunctive Query corresponding SCC ID...\n");

            vector<QueryOnIG> disjunctiveQuery = ProcessEGSDisQuery(IG, sccTable, souNode, tarNode, queryInterval,
                                                                    recordQueryTime);

            if (disjunctiveQuery.empty()) {
                printf("Result Set is empty\n");
            }

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
            printf("\t Finding Conjunctive Query corresponding SCC ID...\n");

            vector<QueryOnIG> conjunctiveQuery = ProcessEGSConQuery(IG, sccTable, souNode, tarNode, queryInterval,
                                                                    recordQueryTime);

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

vector<ToGrail>
QueryReachabilityonIG2(IGraph &IG, OpSccTable &opSccTable, vector<QueryResult> &queryRecords, double &queryTime) {
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

        if (type == 1) {
            //非连续查询
            vector<QueryOnIG> disjunctiveQuery = ProcessEGSDisQuery2(IG, opSccTable, souNode, tarNode, queryInterval,
                                                                     recordQueryTime);

            if (disjunctiveQuery.empty()) {
                printf("Result Set is empty\n");
            }

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
            vector<QueryOnIG> conjunctiveQuery = ProcessEGSConQuery2(IG, opSccTable, souNode, tarNode, queryInterval,
                                                                     recordQueryTime);

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

        printf("==============================\n");

    }
    return toGrail;
}

vector<QueryOnIG>
ProcessEGSDisQuery(IGraph &IG, SccTable &sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT) {

    Node2Scc source = FindSccsOfNode(sccTable, souNode, queryLife);
    printf("\t The Sccs of source vertex %d have been found\n", source.nodeID);

    Node2Scc target = FindSccsOfNode(sccTable, tarNode, queryLife);
    printf("\t The Sccs of target vertex %d have been found\n", target.nodeID);

    vector<QueryOnIG> queryOnIG;

    /* 查询条目中若存在(-1,-1)，可直接返回true；若存在(-2,-2)，可直接返回false
     * 否则，对查询条目逐一处理：
     *      若其一返回true，则直接返回true；
     *      若均返回false，则最终返回false。*/
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
                    } else {
                        //重叠时间段内位于不同SCC，记录souSCC与tarSCC及其区间
                        if (interLife.count() == 1) {
                            try {
                                int ID_sou = IG.FindIDonIG((*souIter).scc_id, interLife);

                                printf("\t The ID of source vertex is %d \n", ID_sou);

                                int ID_tar = IG.FindIDonIG((*tarIter).scc_id, interLife);

                                printf("\t The ID of target vertex is %d \n", ID_tar);

                                QueryOnIG record(ID_sou, ID_tar);
                                queryOnIG.push_back(record);
                            } catch (...) {
                                /*QueryOnIG record(-2, -2);
                                queryOnIG.push_back(record);*/

                                continue;
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
                                    continue;
                                }

                            }
                        }
                    }
                }

            }
        }
    } else {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
    }

    query_endTime = clock();
    recordQT = (double) (query_endTime - query_startTime);

    return queryOnIG;
}
// 处理disjunctive query
vector<QueryOnIG>
ProcessEGSDisQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife,
                    double &recordQT) {
    //Node2Scc 结构： nodeID ：原始图节点的ID， vectorOfSccs：属于的SCC的信息
    Node2Scc source = FindSccsOfNode2(opSccTable, souNode, queryLife);
    //printf("\t The Sccs of source vertex %d have been found\n", source.nodeID);

    Node2Scc target = FindSccsOfNode2(opSccTable, tarNode, queryLife);
    //printf("\t The Sccs of target vertex %d have been found\n", target.nodeID);

    vector<QueryOnIG> queryOnIG;

    /* 查询条目中若存在(-1,-1)，可直接返回true；若存在(-2,-2)，可直接返回false
     * 否则，对查询条目逐一处理：
     *      若其一返回true，则直接返回true；
     *      若均返回false，则最终返回false。*/
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
                    } else {
                        //重叠时间段内位于不同SCC，记录souSCC与tarSCC及其区间
                        if (interLife.count() == 1) {
                            try {
                                int ID_sou = IG.FindIDonIG((*souIter).scc_id, interLife);

                                //printf("\t The ID of source vertex is %d \n", ID_sou);

                                int ID_tar = IG.FindIDonIG((*tarIter).scc_id, interLife);

                                //printf("\t The ID of target vertex is %d \n", ID_tar);

                                QueryOnIG record(ID_sou, ID_tar);
                                queryOnIG.push_back(record);
                            } catch (...) {
                                /*QueryOnIG record(-2, -2);
                                queryOnIG.push_back(record);*/

                                continue;
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
                                    continue;
                                }

                            }
                        }
                    }
                }

            }
        }
    } else {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);
    }

    query_endTime = clock();
    recordQT = (double) (query_endTime - query_startTime);

    return queryOnIG;
}

vector<QueryOnIG>
ProcessEGSConQuery(IGraph &IG, SccTable &sccTable, int souNode, int tarNode, bitset<MNS> queryLife, double &recordQT) {
    Node2Scc source = FindSccsOfNode(sccTable, souNode, queryLife);
    printf("\t The Sccs of source vertex %d have been found\n", source.nodeID);

    Node2Scc target = FindSccsOfNode(sccTable, tarNode, queryLife);
    printf("\t The Sccs of target vertex %d have been found\n", target.nodeID);

    vector<QueryOnIG> queryOnIG;

    /* 查询条目中若存在(-1,-1)，可直接返回true；若存在(-2,-2)，可直接返回false
     * 否则，对每条查询条目逐一处理：
     *      若存在其一返回false，则直接返回false；
     *      若均返回true，则最终返回true*/

    clock_t query_startTime, query_endTime;
    query_startTime = clock();

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
                        /* 若重叠时间段内源SCC与目的SCC相同
                         * 说明souSCC与tarSCC在该时间段内可达，
                         * 不计入查询条目中*/
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
            recordQT = (double) (query_endTime - query_startTime);

            return queryOnIG;

        }
    } else {
        //在全部查询区间内源/目的不存在，直接返回false
        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);

        query_endTime = clock();
        recordQT = (double) (query_endTime - query_startTime);

        return queryOnIG;
    }
}

vector<QueryOnIG>
ProcessEGSConQuery2(IGraph &IG, OpSccTable &opSccTable, int souNode, int tarNode, bitset<MNS> queryLife,
                    double &recordQT) {
    //printf("Processing source vertex %d ...\n", souNode);
    Node2Scc source = FindSccsOfNode2(opSccTable, souNode, queryLife);

    //printf("Processing target vertex %d ...\n", tarNode);
    Node2Scc target = FindSccsOfNode2(opSccTable, tarNode, queryLife);
    //printf("------\n");

    vector<QueryOnIG> queryOnIG;

    clock_t query_startTime, query_endTime;
    query_startTime = clock();

    if (!source.vectorOfSccs.empty() && !target.vectorOfSccs.empty()) {
        if (!source.isFull || !target.isFull) {
            //sou或tar在查询区间内在某一时间段不存在，可立即回答可达性：No
            //连续查询记录中若存在(-2,-2)，可直接返回false
            //printf("\t#False---Case1:the lifespan of sou/tar can't cover query interval#\n");

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
                                    //printf("\t#False---Case2: Other#\n");

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
                                        //printf("\t#False---Case3: Other#\n");
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
                //printf("\t#True--Case: All instant in the same SCC#\n");

                QueryOnIG record(-1, -1);
                queryOnIG.push_back(record);

            }

            query_endTime = clock();
            recordQT = (double) (query_endTime - query_startTime);

            return queryOnIG;

        }
    } else {
        //在全部查询区间内源/目的不存在，直接返回false
        //printf("\t#False---Case4:the lifespan of sou/tar can't cover query interval#\n");

        QueryOnIG record(-2, -2);
        queryOnIG.push_back(record);

        query_endTime = clock();
        recordQT = (double) (query_endTime - query_startTime);

        return queryOnIG;
    }
}

Node2Scc FindSccsOfNode(SccTable &sccTable, int nodeId, bitset<MNS> queryLife) {
    Node2Scc node2Scc;
    node2Scc.nodeID = nodeId;

    bitset<MNS> haveFound;
    haveFound.reset();
    for (auto sccTableIter = sccTable.begin(); sccTableIter != sccTable.end(); sccTableIter++) {
        auto nodePos = (*sccTableIter).first.find(nodeId);
        if (nodePos != (*sccTableIter).first.end()) {
            //该行记录的SCC含有此节点，判断其生存期是否有交集:
            bitset<MNS> sccLife = (*sccTableIter).second.life_time;
            bitset<MNS> interLife;
            interLife = LifespanJoin(queryLife, sccLife);

            if (interLife.any()) {
                SccID_Life sccID_life;
                sccID_life.scc_id = (*sccTableIter).second.scc_id;
                sccID_life.life_time = interLife;

                node2Scc.vectorOfSccs.push_back(sccID_life);

                haveFound = LifespanUnion(haveFound, interLife);
            }
        }
    }

    bitset<MNS> judgeLife = LifespanDifference(queryLife, haveFound);

    if (judgeLife.none()) {
        node2Scc.isFull = true;
    } else {
        node2Scc.isFull = false;
    }

    sort(node2Scc.vectorOfSccs.begin(), node2Scc.vectorOfSccs.end(), SortSccInNode);

    return node2Scc;
}

Node2Scc FindSccsOfNode2(OpSccTable &opSccTable, int nodeId, bitset<MNS> queryLife) {
    Node2Scc node2Scc;
    node2Scc.nodeID = nodeId;

    bitset<MNS> haveFound;
    haveFound.reset();

    auto opSTIter = opSccTable.find(nodeId);
    if (opSTIter != opSccTable.end()) {
        printf("\tFind %d in opSccTable~\n", nodeId);

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

    } else {
        printf("\tDon't find %d in opSccTable~\n", nodeId);
    }

    bitset<MNS> judgeLife = LifespanDifference(queryLife, haveFound);

    if (judgeLife.none()) {
        node2Scc.isFull = true;
        printf("\t#Query interval is covered with node's lifespan#\n");

    } else {
        node2Scc.isFull = false;
        printf("\t#Query interval isn't covered with node's lifespan#\n");

    }

    sort(node2Scc.vectorOfSccs.begin(), node2Scc.vectorOfSccs.end(), SortSccInNode);

    return node2Scc;
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


#endif //IG_NOOP_5_PROCESS_QUERY_H
