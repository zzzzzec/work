#include "common.h"
#include "HRindex.h"
#include "validation.h"
using namespace std;

/*
    原节点ID，SCCID，timeStamp都是从0开始
*/
bool judge(map<int,bool>& res, int type) {
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

int main() {

    int timeIntervalLength = 4; 
    string graphDatafileAddHead = "./GraphData_DBLP/graph";
    //string graphDatafileAddHead = "./testdata/graph";
    //string graphDatafileAddHead = "./Dataset/sx-mathoverflow/sx-mathoverflow";
    string storeIndexGraphAddress = "./Result_IG2Grail/test_IG2GRail.txt";
    string storeFull_IG_Address = "./Result_Full_IG/test_FULL_IG.txt";
    string storeIGJSONPath = "./Result_Full_IG/IG.json";
    string storeOriginGraphPath = "./ResultOriginGraph/";
    string storeSccTableAddress = "./Result_SccTable/test_SccTable.txt";
    string storeRefineNITableAddress = "./Result_RefineNITable/test_RefineNITable.txt";
    string queryFileAddress = "./QueryFile/testQuery.txt";
    string resultFileAddress = "./Result_Query2Grail/testResult.txt";
    string resultFileAddress1 = "./Result_Query2Grail/testResult1.txt";
    string updateFileAddress = "./UpdateFile/testUpdate.txt";
    string logFileAddress = "./log.txt";
    cout << "Starting..." << endl;
    
    HRindex hrindex(
        timeIntervalLength,
        graphDatafileAddHead,
        queryFileAddress,
        resultFileAddress,
        
        storeIndexGraphAddress,
        storeFull_IG_Address,
        storeSccTableAddress,
        storeRefineNITableAddress,
        logFileAddress
    );

    hrindex.buildOriginGraph();
    hrindex.getSCCTable();
    hrindex.buildSCCGraph();
    hrindex.getNITable();
    hrindex.getRefineNITable();
    hrindex.buildIndexGraph();
    hrindex.printStatistics();
 
    cout << "Finish building index graph!" << endl;
    /*
    vector<QueryResult> queryRecords;
    QueryResult queryRecord(4,1, 1, 4, 1, 0);
    queryRecords.push_back(queryRecord);
    Query(hrindex, queryRecords);
    */
    //Query(hrindex, queryRecords);
    //GenerateRandomQuery(hrindex, 1000, queryFileAddress);
    //cout << "start query..." << endl;
    //vector<QueryResult> queryRecords = ReadQuery(queryFileAddress);
    //cout << "query loaded! size :" << queryRecords.size() << endl;
    //Query(hrindex, queryRecords, resultFileAddress);
    vector<updateRecord> updateRecords;
    updateRecords.push_back(updateRecord(UPDATE_TYPE_ADD_EDGE, 7, 10, 1));
    hrindex.updateFromRecords(updateRecords);
    //hrindex.updateFromFile(updateFileAddress);
    
    for (int i = 0; i < hrindex.timeIntervalLength; i++) {
        hrindex.originGraph[i].gsort();
    }
    sort(hrindex.nodeInfoTable.begin(), hrindex.nodeInfoTable.end(), compareRecordItem);
    sort(hrindex.refineNITable.begin(), hrindex.refineNITable.end(), compareRefineRecordItem);
    hrindex.IG.StoreFullIndexGraphJSON(storeIGJSONPath);
    hrindex.storeOriginGraph(storeOriginGraphPath);
    //Query(hrindex, queryRecords, resultFileAddress);
    return 0;
}