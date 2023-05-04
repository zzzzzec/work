#ifndef AEFB69C0_5322_4A8A_9A27_AB27D571511F
#define AEFB69C0_5322_4A8A_9A27_AB27D571511F

#include "HRindex.h"
using namespace std;
bool IsReachable(Graph* graph, int src, int dst);
vector<map<int,bool>> ReachabilityQueryValidation(HRindex& hrindex, vector<QueryResult>& queryRecords) {
    vector<map<int,bool>> results;
    for (auto query = queryRecords.begin(); query != queryRecords.end(); query++) {
        map<int, bool> result;
        int src = query->souID;
        int dst = query->tarID;
        int begin = query->query_b;
        int end = query->query_e;
        int type = query->type;
        for (int i = begin; i <= end; i++) {
            result[i] = hrindex.originGraph[i].IsReachable(src, dst);
        }
        results.push_back(result);
    }
    return results;
}

bool judge(map<int, bool>& res, int type) {
    //disjunctive: 有一个true就是true
    if (type == 1) {
        for (auto it = res.begin(); it != res.end(); it++) {
            if (it->second == true) return true;
        }
        return false;
    }
    //conjunctive: 全部true才是true
    else {
        for (auto it = res.begin(); it != res.end(); it++) {
            if (it->second == false) return false;
        }
        return true;
    }
}

bool judge(vector<pair<ToGrail, bool>>& res, int type) {
    //disjunctive: 有一个true就是true
    if (type == 1) {
        for (auto it = res.begin(); it != res.end(); it++) {
            if (it->second == true) return true;
        }
        return false;
    }
    //conjunctive: 全部true才是true
    else {
        for (auto it = res.begin(); it != res.end(); it++) {
            if (it->second == false) return false;
        }
        return true;
    }
}

bool GenerateRandomQuery(HRindex& hrindex, int num, string queryFileAddress) {
    vector<QueryResult> queryRecords;
    int maxLen = 4;
    for (int i = 0; i < num; i++) {
        int begin = rand() % maxLen;
        int end = rand() % maxLen;
        if (begin > end) {
            int tmp = begin;
            begin = end;
            end = tmp;
        }
        int souID = hrindex.originGraph[begin].getRandomNodeID();
        int tarID = hrindex.originGraph[end].getRandomNodeID();
        int type = rand() % 2 + 1;
        int reach = -1;
        QueryResult queryRecord(souID, tarID, begin, end, type, reach);
        queryRecords.push_back(queryRecord);
    }
    ofstream queryFile;
    queryFile.open(queryFileAddress);
    for (auto it = queryRecords.begin(); it != queryRecords.end(); it++) {
        queryFile << it->souID << " " << it->tarID << " " << it->query_b << " " << it->query_e << " " << it->type << endl;
    }
    queryFile.close();
    return true;
}


bool Query(HRindex& hrindex, vector<QueryResult>& queryRecords, string resultFileAddress) {
    auto groundTruth = ReachabilityQueryValidation(hrindex, queryRecords);
    int i = 0;
    ofstream resultFile;
    resultFile.open(resultFileAddress);
    //在文件头写入当前时间
    time_t now = time(0);
    char* dt = ctime(&now);
    resultFile << "The local date and time is: " << dt << endl;
    
    OpSccTable opSccTable = BuildOpSccTable(hrindex.sccTable);
    for (auto queryit = queryRecords.begin(); queryit != queryRecords.end(); queryit++) {
        cout << "Query(" << i << "): " << queryit->souID << "->" << queryit->tarID << " [" << queryit->query_b << "," << queryit->query_e << "] " << queryit->type << endl;
        bool res;
        bool truth = judge(groundTruth[i], queryit->type);
        double tmp = 0;
        vector<ToGrail> toGrail = QueryReachabilityonIG2(hrindex.IG, opSccTable, (*queryit), tmp);
        bool flag = false;
        for (auto toGrailit = toGrail.begin(); toGrailit != toGrail.end(); toGrailit++) {
            if (toGrailit->souID == -1 && toGrailit->tarID == -1) {
                res = true;
                flag = true;
                break;
            }
            if (toGrailit->souID == -2 && toGrailit->tarID == -2) {
                res = false;
                flag = true;
                break;
            }
            if (!flag) {
                vector<pair<ToGrail, bool>> result;
                for (auto it = toGrail.begin(); it != toGrail.end(); it++) {
                    result.push_back(make_pair(*it, hrindex.IG.isReachable(it->souID, it->tarID)));
                }
                res = judge(result, queryit->type);
            }
        }
        resultFile << endl;
        resultFile << i << " st======================" << endl;
        resultFile << "Query: " << queryit->souID << "->" << queryit->tarID << " [" << queryit->query_b << "," << queryit->query_e << "] " << queryit->type << endl;
        resultFile << "GroundTruth: " << endl;
        for (auto it = groundTruth[i].begin(); it != groundTruth[i].end(); it++) {
            resultFile << it->first << " : " << it->second << endl;
        }
        if (res == truth) resultFile << "Correct!" << endl;
        else {
            resultFile << "Wrong! GroundTruth: " << truth << " Result: " << res << endl;
            exit(0);
        }
        i++;
    }
    cout << "Finish!" << endl;
    resultFile.close();
    return true;
}



void compareHRindex(HRindex& gt, HRindex& test, string resultIGPath) {
    int gtNITsize = gt.nodeInfoTable.size();
    int testNITsize = test.nodeInfoTable.size();
    cout << "comparing NIT..." << endl;
    if (gtNITsize != testNITsize) {
        throw "NIT size not equal!";
    }
    for (int i = 0; i < gtNITsize; i++) {
        int gtinsize = gt.nodeInfoTable[i].In.size();
        int testinsize = test.nodeInfoTable[i].In.size();
        int gtoutsize = gt.nodeInfoTable[i].Out.size();
        int testoutsize = test.nodeInfoTable[i].Out.size();
        
        if (gtinsize != testinsize) {
            throw "In size not equal!";
        }
        if (gtoutsize != testoutsize) {
            throw "Out size not equal!";
        }
        for (int j = 0; j < gtinsize; j++) {
            if(!(gt.nodeInfoTable[i].In[j] == test.nodeInfoTable[i].In[j]))
            {
                throw "In not equal!";
            }
        }
        for (int j = 0; j < gtoutsize; j++) {
            if (!(gt.nodeInfoTable[i].Out[j] == test.nodeInfoTable[i].Out[j])) {
                throw "Out not equal!";
            }
        }
    }
    cout << "comparing RefineNIT..." << endl;
    int gtRefineSize = gt.refineNITable.size();
    int testRefineSize = test.refineNITable.size();
    if (gtRefineSize != testRefineSize) {
        throw "Refine size not equal!";
    }
    for (int i = 0; i < gtRefineSize; i++) {
        int gtOutSize = gt.refineNITable[i].Out.size();
        int testOutSize = test.refineNITable[i].Out.size();
        if (gtOutSize != testOutSize) {
            throw "Out size not equal!";
        }
        for (int j = 0; j < gtOutSize; j++) {
            if (gt.refineNITable[i].Out[j].partLab == test.refineNITable[i].Out[j].partLab && gt.refineNITable[i].Out[j].vertexID == test.refineNITable[i].Out[j].vertexID
                && gt.refineNITable[i].Out[j].lifespan == test.refineNITable[i].Out[j].lifespan) {
            }
            else {
                throw "Out not equal!";
            }
        }
    }

    cout << "comparing IGraph..." << endl;
    int gtNodeNum = gt.IG.GetVexNum();
    int testNodeNum = test.IG.GetVexNum();
    string gtIGPath = resultIGPath + "gtIG.json";
    string testIGPath = resultIGPath + "testIG.json";
    gt.IG.StoreFullIndexGraphJSON(gtIGPath);
    test.IG.StoreFullIndexGraphJSON(testIGPath);
    
    if (gtNodeNum != testNodeNum) {
        throw "NodeNum not equal!";
    }
    for (int i = 0; i < gtNodeNum; i++) {
        IGVerNode* gtNode = gt.IG.findNodeByPos(i);
        IGVerNode* testNode = test.IG.findNode(gtNode->souID, gtNode->souLifespan);
        if (testNode == NULL) {
            throw "Node not found";
        }
        if (!(*gtNode == *testNode)) {
            throw "Node not equal!";
        }
    }
    cout << "compare success!" << endl;
    return;
}

#endif /* AEFB69C0_5322_4A8A_9A27_AB27D571511F */
