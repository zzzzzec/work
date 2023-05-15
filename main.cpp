#include "common.h"
#include "HRindex.h"
#include "validation.h"
using namespace std;
/*
    原节点ID，SCCID，timeStamp都是从0开始
*/

int main() {

    //第一行输出日期
    logFile = fstream("./log.txt", ios::out);
    time_t now = time(0);
    char* dt = ctime(&now);
    LOG << "The local date and time is: " << dt << endl;
    LOG << "starting" << endl;
    
    int timeIntervalLength = 4;
    string gtGraphDatefileAddHead = "./gt/data/graph";
    string gtStoreOriginGraphDir = "./gt/originGraph/";
    string gtStoreSCCGraphDir = "./gt/SCCGraph/";
    string gtStoreIGDir = "./gt/IG/";
    string gtResultPath = "./gt/result.txt";

    string testGraphDatafileAddHead = "./test/data/graph";
    string testStoreOriginGraphDir = "./test/originGraph/";
    string testStoreSCCGraphDir = "./test/SCCGraph/";
    string testStoreIGDir = "./test/IG/";
    string testResultPath = "./test/result.txt";
    
    string queryFilePath = "./QueryFile/testQuery.txt";
    string updateFilePath = "./UpdateFile/testUpdate.txt";

    cout << "Starting..." << endl;

    vector<updateRecord> updateRecords;
    updateRecords.push_back(updateRecord(UPDATE_TYPE_DELETE_EDGE,8, 5, 1));
    //updateRecords.push_back(updateRecord(UPDATE_TYPE_ADD_EDGE, 6, 4, 0));
    //updateRecords.push_back(updateRecord(UPDATE_TYPE_ADD_EDGE, 6, 4, 1));
    //updateRecords.push_back(updateRecord(UPDATE_TYPE_ADD_EDGE, 7, 11, 2));

    HRindex gt(
        timeIntervalLength,
        gtGraphDatefileAddHead,
        queryFilePath,
        gtResultPath
    );
    gt.buildOriginGraph();
    gt.getSCCTable();
    gt.buildSCCGraph();
    gt.getNITable();
    gt.getRefineNITable();
    gt.buildIndexGraph();
    gt.printStatistics();

    LOG << "gt construction finished" << endl;
    
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
    test.updateFromRecords(updateRecords, 3);

    LOG << "test construction finished" << endl;
    //remap
    map<int, int> remap;
    for (auto it : gt.sccTable) {
        auto nodeGroup = it.nodeGroup;
        auto findres = find_if(test.sccTable.begin(), test.sccTable.end(),
            [&](const SCCTableItem& item) {return it.nodeGroup == item.nodeGroup; });
        if (findres == test.sccTable.end()) throw "SCC not found!";
        remap[it.sccID_Life.scc_id] = findres->sccID_Life.scc_id;
    }
    for (auto &it : gt.sccTable) {
        it.SCCID = remap[it.SCCID];
    }
    for (auto& it : gt.sccGraphs) {
        for (auto& it2 : it.vertices) {
            it2.SCCID = remap[it2.SCCID];
            for(auto edge = it2.firstArc; edge != NULL; edge = edge->next) {
                edge->dstID = remap[edge->dstID];
            }
        }
    }
    for (auto& it : gt.nodeInfoTable) {
        it.node = remap[it.node];
        for (auto & itin : it.In) {
            itin.vertexID = remap[itin.vertexID];
        }
        for (auto & itout : it.Out) {
            itout.vertexID = remap[itout.vertexID];
        }
    }
    for (auto& it : gt.refineNITable) {
        it.node = remap[it.node];
        for (auto & outit : it.Out) {
            outit.vertexID = remap[outit.vertexID];
        }
    }
    gt.IG.IDremap(remap);
    gt.sortAll();
    gt.storeOriginGraph(gtStoreOriginGraphDir);
    gt.storeSCCGraph(gtStoreSCCGraphDir);
    gt.storeIG(gtStoreIGDir);

    test.sortAll();
    test.storeOriginGraph(testStoreOriginGraphDir);
    test.storeSCCGraph(testStoreSCCGraphDir);
    test.storeIG(testStoreIGDir);
    
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