#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <windows.h>
#include <direct.h>


#include "Lifespan.h"
#include "Graph.h"
#include "SCC_Table.h"
#include "Process_Snapshots.h"
#include "Construct_NITable.h"
#include "IndexGraph.h"
#include "Process_Query.h"

using namespace std;

int main() {

    int timeIntervalLength = 16;

    string graphDatafileAddHead = "./GraphData_DBLP/graph";
    /*
    string storeIndexGraphAddress = "../Result_IG2Grail/IGraph_DBLP.txt";
    string storeFull_IG_Address = "../Result_Full_IG/IG_WIKI-Talk-20.txt";
    string storeSccTableAddress = "../Result_SccTable/SccTable_WIKI-Talk-20.txt";
    string storeRefineNITableAddress = "../Result_RefineNITable/RefineNITable_DBLP.txt";
    string recordConstructTime = "../Result_ConstructSccTableTime/BuildSccTable_DBLP.txt";
    */

    string storeIndexGraphAddress = "./Result_IG2Grail/test_IG2GRail.txt";
    string storeFull_IG_Address = "./Result_Full_IG/test_FULL_IG.txt";
    string storeSccTableAddress = "./Result_SccTable/test_SccTable.txt";
    string storeRefineNITableAddress = "./Result_RefineNITable/test_RefineNITable.txt";
    string recordConstructTime = "./Result_ConstructSccTableTime/test_BuildSccTable.txt";

    /*
    string queryFileAddress = "./QueryFile/query_Wiki-Talk_con8.txt";
    string resultFileAddress = "./Result_Query2Grail/query_WT8.txt";
    */
    string queryFileAddress = "./QueryFile/testQuery.txt";
    string resultFileAddress = "./Result_Query2Grail/testResult.txt"; 
    cout << "Starting..." << endl;

    //==============================================================================
    vector<vector<int>> evolvingGraphSequence;

    //Get Scc-Table
    SccTable sccTable;

    double buildSccTableTime;
    //evolvingGraphSequence: 一共timeIntervalLength个时间戳，每个时间戳对应一个SCC图，两行表示一条边
    sccTable = GetSCCTable(timeIntervalLength, graphDatafileAddHead, evolvingGraphSequence, buildSccTableTime);
    StoreSccTable(storeSccTableAddress, sccTable);


    printf("----------Scc-Table has been built------------\n");

    //----------------构建NITable----------------

    clock_t buildNIT_startTime, buildNIT_endTime;
    buildNIT_startTime = clock();

    NodeInfoTable nodeInfoTable;
    for (int i = 1; i <= timeIntervalLength; ++i) {
        int timestamp;
        timestamp = i;

        printf("\t Getting NITable at the %dth / %d snapshot...\n", timestamp, timeIntervalLength);

        nodeInfoTable = GetNITable(nodeInfoTable, evolvingGraphSequence, timestamp);
    }
    //nodeInfoTable:  一共有SCCnumber个项目，每个item里包含SCC的ID，指向这个SCC的节点（in）这个SCC节点指向的其他SCC节点（out）
    buildNIT_endTime = clock();

    double buildNIT_Time = (double) (buildNIT_endTime - buildNIT_startTime);

    printf("---------------------NI-Table Construction Completed.------------------------");

    //----------------细化NITable----------------
    RefineNITable refineNITable;

    clock_t buildReNIT_startTime, buildReNIT_endTime;
    buildReNIT_startTime = clock();
    /*
        细化后的NITable: 有SCCnumber个项目，每个item里包含SCC的ID，以及这个SCC所指向的SCC节点，还有标记。
        如果是A -> B , A 没有入边，因此标记为2
        如果是C -> A -> B, A有入边，因此标记为1
    */
    refineNITable = GetRefineNITable(nodeInfoTable);

    buildReNIT_endTime = clock();

    double buildReNIT_Time = (double) (buildReNIT_endTime - buildReNIT_startTime);

    buildSccTableTime = buildSccTableTime + buildReNIT_Time + buildNIT_Time;

    //StoreBuildSccTableTime(recordConstructTime, buildSccTableTime);

    StoreRefineNITable(storeRefineNITableAddress, refineNITable);

    printf("----------------------Refine NI-Table has been built-------------------------\n");

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    cout << "------------------------Begin Index-Graph Building---------------------------" << endl;
    cout << endl;

    clock_t build_startTime, build_endTime;
    build_startTime = clock();

    IGraph IG = BuildIndexGraph(refineNITable);

    //IG.OptimizeIntervalVertex();

    build_endTime = clock();

    double buildIGTime = (double) (build_endTime - build_startTime);

    StoreConstructTime(recordConstructTime, buildSccTableTime, buildIGTime);
    //存储索引图
    IG.StoreFullIndexGraph(storeFull_IG_Address);
    IG.WriteEdgeFormIG2(storeIndexGraphAddress);

    printf("----------------------All Ready-------------------------\n");

    //======================================= Query Part =======================================

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //---------------------------若无IG---------------------------
/*

    SccTable sccTable = ReadSccTable(storeSccTableAddress);
    RefineNITable refineNITable = ReadRefineNITable(storeRefineNITableAddress);
    clock_t build_startTime, build_endTime;
    build_startTime = clock();

    IGraph IG = BuildIndexGraph(refineNITable);

    IG.OptimizeIntervalVertex();

    build_endTime = clock();

    double buildIGTime = (double) (build_endTime - build_startTime);

    StoreConstructTime(recordConstructTime, 0, buildIGTime);

    //存储索引图
    IG.StoreFullIndexGraph(storeFull_IG_Address);
    IG.WriteEdgeFormIG2(storeIndexGraphAddress);

    printf("----------------------IG Ready-------------------------\n");
*/

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /*
    //--------------------------------读取数据--------------------------------

    SccTable sccTable = ReadSccTable(storeSccTableAddress);
    //RefineNITable refineNITable = ReadRefineNITable(storeRefineNITableAddress);
    IGraph IG = ReadIndexGraph(storeFull_IG_Address);

    //--------------------------------开始查询--------------------------------
    */
    //opSccTable: 表项数量为原始图的节点数，包含了自己的生存周期内的所属SCC的情况（哪个时间段属于哪个SCC）
    OpSccTable opSccTable = BuildOpSccTable(sccTable);
    //queryFile结构: souID, tarID, query_begin, query_end, type(1/2), reachability(output)
    vector<QueryResult> queryRecords = ReadQuery(queryFileAddress);

    double queryTime = 0;

    clock_t query_s, query_e;
    query_s = clock();

    vector<ToGrail> toGrail = QueryReachabilityonIG2(IG, opSccTable, queryRecords, queryTime);

    query_e = clock();
    double queryTime2 = (double) (query_e - query_s);

    StoreQueryRecords2Grail(resultFileAddress, toGrail, queryTime2);

    cout << "-----------------" << endl;
    cout << setprecision(8) << "Query Time is: " << queryTime / CLOCKS_PER_SEC << " s / 1000 query records. " << endl;
    cout << setprecision(8) << "Query Time2 is: " << queryTime2 / CLOCKS_PER_SEC << " s / 1000 query records. " << endl;

    cout << "-----------------" << endl;
    cout << "The size of SCC-Table is: " << sccTable.size() << endl;
    cout << "The number of vertices in IndexGraph is: " << IG.GetVexNum() << endl;
    cout << "The number of edges in IndexGraph is: " << IG.GetEdgeNum() << endl;

    return 0;

}