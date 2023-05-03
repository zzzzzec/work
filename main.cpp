#include "common.h"
#include "HRindex.h"
#include "validation.h"
using namespace std;
/*
    原节点ID，SCCID，timeStamp都是从0开始
*/

int main() {

    int timeIntervalLength = 4;
    string graphDatafileAddHead = "./GraphData_DBLP/graph";
    //string graphDatafileAddHead = "./testdata/graph";
    //string graphDatafileAddHead = "./Dataset/sx-mathoverflow/sx-mathoverflow";
    string storeIndexGraphAddress = "./Result_IG2Grail/test_IG2GRail.txt";
    string storeFull_IG_Address = "./Result_Full_IG/test_FULL_IG.txt";
    string storeIGJSONPath = "./Result_Full_IG/IG.json";
    string storeOriginGraphPath = "./ResultOriginGraph/";
    string storeSCCGraphPath = "./resultSCCGraph/";
    string storeSccTableAddress = "./Result_SccTable/test_SccTable.txt";
    string storeRefineNITableAddress = "./Result_RefineNITable/test_RefineNITable.txt";
    string queryFileAddress = "./QueryFile/testQuery.txt";
    string resultFileAddress = "./Result_Query2Grail/testResult.txt";
    string resultFileAddress1 = "./Result_Query2Grail/testResult1.txt";
    string updateFileAddress = "./UpdateFile/testUpdate.txt";
    string logFileAddress = "./log.txt";
    cout << "Starting..." << endl;

    vector<updateRecord> updateRecords;
    updateRecords.push_back(updateRecord(UPDATE_TYPE_ADD_EDGE, 7, 10, 1));

    HRindex gt(
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
    HRindex test(
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
    gt.buildOriginGraph();
    gt.getSCCTable();
    gt.buildSCCGraph();
    gt.getNITable();
    gt.getRefineNITable();
    gt.buildIndexGraph();
    gt.printStatistics();
    gt.updateFromRecords(updateRecords, 1);
    gt.sortAll();

    test.buildOriginGraph();
    test.getSCCTable();
    test.buildSCCGraph();
    test.getNITable();
    test.getRefineNITable();
    test.buildIndexGraph();
    test.printStatistics();
    test.updateFromRecords(updateRecords, 2);
    test.sortAll();
    cout << "Finish building index graph!" << endl;
    string compareIGPath = "./compare/";
    compareHRindex(gt, test, compareIGPath);
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
    
    //hrindex.IG.StoreFullIndexGraphJSON(storeIGJSONPath);
    //hrindex.storeOriginGraph(storeOriginGraphPath);
    //hrindex.storeSCCGraph(storeSCCGraphPath);
    //Query(hrindex, queryRecords, resultFileAddress);
    return 0;
}