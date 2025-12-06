#pragma once
#include <vector>
using namespace std;

template<typename Key, typename Value>
struct AVLNode{
    Key key;
    vector<Value> values;
    int height;
    AVLNode *left,*right;

    AVLNode(const Key &k,const Value &val){
        key=k;
        values.push_back(val);
        height=1;
        left=right=nullptr;
    }
};
