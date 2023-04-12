#include "common.h"
#include "HRindex.h"
using namespace std;

void Query(HRindex& hrindex, vector<QueryResult>& queryRecords) {
    OpSccTable opSccTable = BuildOpSccTable(hrindex.sccTable);
    double tmp = 0;
    vector<ToGrail> toGrail = QueryReachabilityonIG2(hrindex.IG, opSccTable, queryRecords, tmp);
    for(auto it = toGrail.begin(); it != toGrail.end(); it++){
        cout << it->souID << " " << it->tarID << " " << hrindex.IG.isReachable(it->souID, it->tarID) << endl;
    }
}

int main() {

    int timeIntervalLength = 16; 
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
    hrindex.updateFileAddress = updateFileAddress;

    hrindex.buildOriginGraph();
    hrindex.getSCCTable();
    hrindex.stroreSCCTable();
    hrindex.getNITable();
    hrindex.getRefineNITable();
    hrindex.stroreRefineNITable();
    hrindex.buildIndexGraph();
    hrindex.stroreIndexGraph();

    vector<QueryResult> queryRecords;
    QueryResult queryRecord(4,1, 1, 4, 1, 0);
    queryRecords.push_back(queryRecord);
    Query(hrindex, queryRecords);

    hrindex.update();
    Query(hrindex, queryRecords);
    
    return 0;

}