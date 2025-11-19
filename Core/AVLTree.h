#pragma once
#include <algorithm>
#include <vector>
#include "AVLNode.h"
using namespace std;

template<typename Key, typename Value>
class AVLTree{
public:
    AVLNode<Key,Value>* root;

    AVLTree(){ root = nullptr; }

    // ---------- HEIGHT ----------
    int height(AVLNode<Key,Value>* n){
        return n ? n->height : 0;
    }

    // ---------- BALANCE ----------
    int getBalance(AVLNode<Key,Value>* n){
        return n ? height(n->left) - height(n->right) : 0;
    }

    // ---------- ROTATIONS ----------
    AVLNode<Key,Value>* rightRotate(AVLNode<Key,Value>* y){
        auto x = y->left;
        auto T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;
        return x;
    }

    AVLNode<Key,Value>* leftRotate(AVLNode<Key,Value>* x){
        auto y = x->right;
        auto T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;
        return y;
    }

    // ---------- INSERT ----------
    AVLNode<Key,Value>* insertNode(AVLNode<Key,Value>* node, const Key &key, const Value &value){
        if(!node) return new AVLNode<Key,Value>(key, value);

        if(key < node->key) 
            node->left = insertNode(node->left, key, value);
        else if(key > node->key) 
            node->right = insertNode(node->right, key, value);
        else{ 
            node->values.push_back(value);
            return node;
        }

        node->height = 1 + max(height(node->left), height(node->right));
        int bal = getBalance(node);

        if(bal > 1 && key < node->left->key) return rightRotate(node);
        if(bal < -1 && key > node->right->key) return leftRotate(node);

        if(bal > 1 && key > node->left->key){
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if(bal < -1 && key < node->right->key){
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void insert(const Key &key, const Value &value){
        root = insertNode(root, key, value);
    }

    // ---------- REMOVE SINGLE VALUE (remove only this value from node->values) ----------
bool removeValue(const Key &key, const Value &value){
    // find node (iterative)
    AVLNode<Key,Value>* cur = root;
    while(cur){
        if(key < cur->key) cur = cur->left;
        else if(key > cur->key) cur = cur->right;
        else break;
    }
    if(!cur) return false;

    // find value inside values vector
    auto it = std::find(cur->values.begin(), cur->values.end(), value);
    if(it == cur->values.end()) return false;

    cur->values.erase(it);

    // if still has other values, keep node
    if(!cur->values.empty()) return true;

    // otherwise remove the whole node (key), using existing removeNode
    root = removeNode(root, key);
    return true;
}


    // ---------- MIN NODE (for deletion) ----------
    AVLNode<Key,Value>* minNode(AVLNode<Key,Value>* node){
        while(node->left) node = node->left;
        return node;
    }

    // ---------- REMOVE ----------
    AVLNode<Key,Value>* removeNode(AVLNode<Key,Value>* node, const Key &key){
        if(!node) return nullptr;

        if(key < node->key)
            node->left = removeNode(node->left, key);
        else if(key > node->key)
            node->right = removeNode(node->right, key);
        else{
            if(!node->left || !node->right){
                AVLNode<Key,Value>* temp = node->left ? node->left : node->right;
                if(!temp){
                    temp = node;
                    node = nullptr;
                } else {
                    *node = *temp;
                }
                delete temp;
            }
            else{
                AVLNode<Key,Value>* temp = minNode(node->right);
                node->key = temp->key;
                node->values = temp->values;
                node->right = removeNode(node->right, temp->key);
            }
        }

        if(!node) return nullptr;

        node->height = 1 + max(height(node->left), height(node->right));
        int bal = getBalance(node);

        if(bal > 1 && getBalance(node->left) >= 0) return rightRotate(node);
        if(bal > 1 && getBalance(node->left) < 0){
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if(bal < -1 && getBalance(node->right) <= 0) return leftRotate(node);
        if(bal < -1 && getBalance(node->right) > 0){
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    bool remove(const Key &key){
        if(!root) return false;
        root = removeNode(root, key);
        return true;
    }

    // ---------- RANGE QUERY ----------
    void rangeQueryHelper(AVLNode<Key,Value>* node, const Key &low, const Key &high, vector<Value> &out){
        if(!node) return;

        if(low < node->key) rangeQueryHelper(node->left, low, high, out);
        if(node->key >= low && node->key <= high){
            for(const auto &v : node->values)
                out.push_back(v);
        }
        if(high > node->key) rangeQueryHelper(node->right, low, high, out);
    }

    vector<Value> rangeQuery(const Key &low, const Key &high){
        vector<Value> out;
        rangeQueryHelper(root, low, high, out);
        return out;
    }

    // ---------- GET LAST K ELEMENTS (INORDER REVERSE) ----------
    void getLastKHelper(AVLNode<Key,Value>* node, int &k, vector<pair<Key,Value>> &out){
        if(!node || k <= 0) return;

        getLastKHelper(node->right, k, out);

        for(auto &v : node->values){
            if(k <= 0) break;
            out.push_back({node->key, v});
            k--;
        }

        getLastKHelper(node->left, k, out);
    }

    vector<pair<Key,Value>> getLastK(int k){
        vector<pair<Key,Value>> out;
        getLastKHelper(root, k, out);
        return out;
    }
};

