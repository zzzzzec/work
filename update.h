#ifndef IG_NOOP_5_UPDATE_H
#define IG_NOOP_5_UPDATE_H

#include "common.h"
#include "Lifespan.h"
#include "Graph.h"
#include "SCC_Table.h"
#include "Process_Snapshots.h"
#include "Construct_NITable.h"
#include "IndexGraph.h"
#include "Process_Query.h"
#include "update.h"
#include "HRindex.h"
#include "SCCGraph.h"

/* updatefile structure:
    type: 
        1: add a single node (u)
        2: delete a single node (v)
        3: add a single edge<u,v>
        4: delete a single edge<u,v>
    u: the node id
    v: the node id(in case 1 and 2 this value is -1)
    timestamp: the time when this update happens
*/
//wirte a json for this structure

typedef struct updateRecord_t{
    int type;
    int u;
    int v;
    int timestamp;
}updateRecord;

bool readUpdateRecords(vector<updateRecord> &updateRecordVector, string updateFileAddress){
    ifstream updateFile(updateFileAddress);
    if(!updateFile.is_open()){
        printf("update file open failed");
        return false;
    }
    string line;
    while(getline(updateFile, line)){
        updateRecord ur;
        stringstream ss(line);
        ss >> ur.type >> ur.u >> ur.v >> ur.timestamp;
        updateRecordVector.push_back(ur);
    }
    return true;

}

#endif