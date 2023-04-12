#ifndef AEFB69C0_5322_4A8A_9A27_AB27D571511F
#define AEFB69C0_5322_4A8A_9A27_AB27D571511F

#include "HRindex.h"

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
            result[i] = IsReachable(&(hrindex.originGraph[i - 1]), src, dst);
        }
        results.push_back(result);
    }
    return results;
}

bool IsReachable(Graph* graph, int src, int dst) {
    if (src == dst)
        return true;
    map<int,bool> visited;
    stack<int> s;
    s.push(src);
    while (!s.empty()) {
        int u = s.top();
        s.pop();
        int spos = graph->VerPos(u);
        for (ArcNode* v = graph->vertices[u].firstArc; v != NULL; v = v->nextarc) {
            if (v->tarID == dst)
                return true;
            if (!visited[v->tarID]) {
                visited[v->tarID] = true;
                s.push(v->tarID);
            }
        }
    }
    return false;
}

#endif /* AEFB69C0_5322_4A8A_9A27_AB27D571511F */
