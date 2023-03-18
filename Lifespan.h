//
// Created by MoCuishle on 2019/11/29.
//

#ifndef IG_NOOP_5_LIFESPAN_H
#define IG_NOOP_5_LIFESPAN_H

#include <bitset>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
//#define MNS 5000
#define MNS 20

using namespace std;

//typedef bitset<MNS> Lifespan;

bitset<MNS> LifespanJoin(bitset<MNS> &l1, bitset<MNS> &l2);

bitset<MNS> LifespanUnion(bitset<MNS> &l1, bitset<MNS> &l2);

bitset<MNS> LifespanBuild(bitset<MNS> &b, int i, int j);

bool LifespanisSub(bitset<MNS> &L, bitset<MNS> &l);

bool LifespanisProSub(bitset<MNS> &L, bitset<MNS> &l);

int GetFirst1Pos(bitset<MNS> &b);

bitset<MNS> LifespanDifference(bitset<MNS> &L, bitset<MNS> &l);

vector<int> GetLifespanTruePos(bitset<MNS> &l);

int LifespanTrue_MaxPos(bitset<MNS> &l);

int LifespanTrue_MinPos(bitset<MNS> &l);

vector<string> split(const string &s, const string &seperator);

bitset<MNS> StringToLifespan(string str);
//-----------------------------------------------
bitset<MNS> LifespanJoin(bitset<MNS> &l1, bitset<MNS> &l2) {
    bitset<MNS> result;

    result = l1 & l2;
    return result;
}

bitset<MNS> LifespanUnion(bitset<MNS> &l1, bitset<MNS> &l2) {
    bitset<MNS> result;

    result = l1 | l2;
    return result;

}

bitset<MNS> LifespanBuild(bitset<MNS> &b, int i, int j) {
    for (int k = i; k <= j; ++k) {
        b.set(k);
    }
    return b;
}

bool LifespanisSub(bitset<MNS> &L, bitset<MNS> &l) {
    bitset<MNS> j = LifespanJoin(L, l);
    if (l == j) {
        return true;
    } else {
        return false;
    }
}

int GetFirst1Pos(bitset<MNS> &b) {
    for (int i = 0; i < MNS; ++i) {
        if (b.test(i)) {
            return i;
        }
    }

    return -1;
}

bool LifespanisProSub(bitset<MNS> &L, bitset<MNS> &l) {
    if (LifespanisSub(L, l) && L.count() != l.count()) {
        return true;
    } else {
        return false;
    }
}

bitset<MNS> LifespanDifference(bitset<MNS> &L, bitset<MNS> &l) {
    bitset<MNS> res;

    res = L;
    for (int i = 0; i < MNS; ++i) {
        if (res.test(i) && l.test(i)) {
            res.reset(i);
        }
    }
    return res;
}

vector<int> GetLifespanTruePos(bitset<MNS> &l) {
    vector<int> posVector;

    for (int i = 0; i < MNS; ++i) {
        if (l.test(i)) {
            posVector.push_back(i);
        }
    }

    return posVector;
}

int LifespanTrue_MaxPos(bitset<MNS> &l) {

    for (int i = MNS - 1; i >= 0; --i) {
        if (l.test(i)) {
            return i;
        }
    }

    return -1;
}

int LifespanTrue_MinPos(bitset<MNS> &l) {
    for (int i = 0; i < MNS; ++i) {
        if (l.test(i)){
            return i;
        }
    }
    return -1;
}

vector<string> split(const string &s, const string &seperator) {
    vector<string> result;
    typedef string::size_type string_size;
    string_size i = 0;

    while (i != s.size()) {
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        while (i != s.size() && flag == 0) {
            flag = 1;
            for (string_size x = 0; x < seperator.size(); ++x) {
                if (s[i] == seperator[x]) {
                    ++i;
                    flag = 0;
                    break;
                }
            }
        }

        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while (j != s.size() && flag == 0) {
            for (string_size x = 0; x < seperator.size(); ++x) {
                if (s[j] == seperator[x]) {
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) {
                ++j;
            }
        }
        if (i != j) {
            result.push_back(s.substr(i, j - i));
            i = j;
        }
    }
    return result;
}

bitset<MNS> StringToLifespan(string str) {
    bitset<MNS> lifespan;

    reverse(str.begin(),str.end());

    for (int i = 0; i < str.size(); ++i) {
        if (str[i]=='1'){
            lifespan.set(i);
        }
    }

    return lifespan;
}

#endif //IG_NOOP_5_LIFESPAN_H
