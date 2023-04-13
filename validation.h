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

#endif /* AEFB69C0_5322_4A8A_9A27_AB27D571511F */
