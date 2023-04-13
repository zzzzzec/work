#include "common.h"
#include "HRindex.h"
#include "validation.h"
using namespace std;

/*
    原节点ID，SCCID，timeStamp都是从0开始
*/


bool Query(HRindex& hrindex, vector<QueryResult>& queryRecords) {
    auto groundTruth = ReachabilityQueryValidation(hrindex, queryRecords);
    int i = 0;
    OpSccTable opSccTable = BuildOpSccTable(hrindex.sccTable);
    for (auto queryit = queryRecords.begin(); queryit != queryRecords.end(); queryit++) {
        bool res;
        bool truth;
        //disjunctive: 有一个true就是true
        if (queryit->type == 1) {
            for (auto groundTruthit = groundTruth[i].begin(); groundTruthit != groundTruth[i].end(); groundTruthit++) {
                if (groundTruthit->second == true) {
                    truth = true;
                    break;
                }
            }
            truth = false;
        }
        //conjunctive: 全部true才是true
        else {
            for (auto groundTruthit = groundTruth[i].begin(); groundTruthit != groundTruth[i].end(); groundTruthit++) {
                if (groundTruthit->second == false) {
                    truth = false;
                    break;
                }
                truth = true;
            }
        }
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
        }
        if (!flag) {
            vector<pair<ToGrail, bool>> result;
            for (auto it = toGrail.begin(); it != toGrail.end(); it++) {
                result.push_back(make_pair(*it, hrindex.IG.isReachable(it->souID, it->tarID)));
            }
            if (queryit->type == 1) {
                for (auto it = result.begin(); it != result.end(); it++) {
                    if (it->second == true) {
                        res = true;
                        break;
                    }
                }

            }
            else {
                for (auto it = result.begin(); it != result.end(); it++) {
                    if (it->second == false) {
                        res = false;
                        break;
                    } 
                }

            }
        }
        cout << endl;
        cout << i << " st======================" << endl;
        cout << "Query: " << queryit->souID << "->" << queryit->tarID << " [" << queryit->query_b << "," << queryit->query_e << "] " << queryit->type << endl;
        cout << "GroundTruth: " << endl;
        for (auto it = groundTruth[i].begin(); it != groundTruth[i].end(); it++) {
            cout << it->first << " : " << it->second << endl;
        }
        if (res == truth) cout << "Correct!" << endl;
        else {
            cout << "Wrong! GroundTruth: " << truth << " Result: " << res << endl;
            int a = 1;
            a++;
        }
        i++;
    }
    cout << "Finish!" << endl;
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
    string storeIndexGraphAddress = "./Result_IG2Grail/test_IG2GRail.txt";
    string storeFull_IG_Address = "./Result_Full_IG/test_FULL_IG.txt";
    string storeSccTableAddress = "./Result_SccTable/test_SccTable.txt";
    string storeRefineNITableAddress = "./Result_RefineNITable/test_RefineNITable.txt";
    string recordConstructTime = "./Result_ConstructSccTableTime/test_BuildSccTable.txt";
    string queryFileAddress = "./QueryFile/testQuery.txt";
    string resultFileAddress = "./Result_Query2Grail/testResult.txt";
    string updateFileAddress = "./UpdateFile/testUpdate.txt";

    cout << "Starting..." << endl;
    
    HRindex hrindex;
    hrindex.timeIntervalLength = timeIntervalLength;
    hrindex.graphDatafileAddHead = graphDatafileAddHead;
    hrindex.storeIndexGraphAddress = storeIndexGraphAddress;
    hrindex.storeFull_IG_Address = storeFull_IG_Address;
    hrindex.storeSccTableAddress = storeSccTableAddress;
    hrindex.storeRefineNITableAddress = storeRefineNITableAddress;
    hrindex.recordConstructTime = recordConstructTime;
    hrindex.queryFileAddress = queryFileAddress;
    hrindex.resultFileAddress = resultFileAddress;

    hrindex.buildOriginGraph();
    hrindex.getSCCTable();
    hrindex.buildSCCGraph();
    hrindex.getNITable();
    hrindex.getRefineNITable();
    hrindex.buildIndexGraph();

    /*
    vector<QueryResult> queryRecords;
    QueryResult queryRecord(4,1, 1, 4, 1, 0);
    queryRecords.push_back(queryRecord);
    Query(hrindex, queryRecords);
    */
    //hrindex.update();
    //Query(hrindex, queryRecords);
    //GenerateRandomQuery(hrindex, 1000, queryFileAddress);
    vector<QueryResult> queryRecords = ReadQuery(queryFileAddress);
    Query(hrindex, queryRecords);
    return 0;

}