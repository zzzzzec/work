#ifndef IG_NOOP_5_UPDATE_H
#define IG_NOOP_5_UPDATE_H

#include "common.h"
#include "HRindex.h"

/* updatefile structure:
    type: 1,2,3,4
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
    //construct function
    updateRecord_t() {}
    updateRecord_t(int type, int u, int v, int timestamp):type(type), u(u), v(v), timestamp(timestamp) {}
}updateRecord;

bool readUpdateRecords(vector<updateRecord>& updateRecordVector, string updateFileAddress) {
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