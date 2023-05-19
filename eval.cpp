#include "common.h"
#include "HRindex.h"
#include "validation.h"
//#define TESTFILE "sx-mathoverflow"
//#define TESTFILE "wiki-talk-temporal"
//#define FILESIZE "100k"
//#define TESTFILE "sx-stackoverflow"
//#define FILESIZE "150k"
//#define TESTFILE "sx-askubuntu"
//#define FILESIZE "100k"
#define TESTFILE "CollegeMsg"
//#define TESTFILE_ADDHEAD "./Dataset/" TESTFILE "/" TESTFILE  "-" FILESIZE
#define TESTFILE_ADDHEAD "./Dataset/" TESTFILE "/" TESTFILE
#define UPDATEFILEPATH "./Dataset/"  TESTFILE  "/update-del.txt"
using namespace std;


int main() {

    //第一行输出日期
    logFile = fstream("./log.txt", ios::out);
    time_t now = time(0);
    char* dt = ctime(&now);
    LOG << "The local date and time is: " << dt << endl;
    LOG << "starting" << endl;
    
    int timeIntervalLength = 16;

    //string testGraphDatafileAddHead = "./test/data/graph";
    string testGraphDatafileAddHead = TESTFILE_ADDHEAD;
    string testStoreOriginGraphDir = "./test/originGraph/";
    string testStoreSCCGraphDir = "./test/SCCGraph/";
    string testStoreIGDir = "./test/IG/";
    string testResultPath = "./test/result.txt";
    
    string queryFilePath = "./QueryFile/testQuery.txt";

    cout << "Starting..." << endl;
    
    HRindex test(
        timeIntervalLength,
        testGraphDatafileAddHead,
        queryFilePath,
        testResultPath
    );

    test.buildOriginGraph();
    test.getSCCTable();
    test.buildSCCGraph();
    test.getNITable();
    test.getRefineNITable();
    test.buildIndexGraph();
    test.printStatistics();
    test.updateFromFile(UPDATEFILEPATH, 3);

    LOG << "test construction finished" << endl;
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