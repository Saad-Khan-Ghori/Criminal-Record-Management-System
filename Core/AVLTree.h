#pragma once
#include <algorithm>
#include <vector>
#include <functional>
#include "AVLNode.h"
using namespace std;

template<typename Key, typename Value>
class AVLTree{
private:
    // ---------- MIN NODE (for deletion) ----------
    AVLNode<Key,Value>* minNode(AVLNode<Key,Value>* node){
        while(node && node->left) node = node->left;
        return node;
    }

    // ---------- REMOVE NODE BY KEY (private) ----------
    AVLNode<Key,Value>* removeNode(AVLNode<Key,Value>* node, const Key &key){
        if(!node) return nullptr;

        if(key < node->key)
            node->left = removeNode(node->left, key);
        else if(key > node->key)
            node->right = removeNode(node->right, key);
        else{
            // Node found - handle deletion
            if(!node->left || !node->right){
                AVLNode<Key,Value>* temp = node->left ? node->left : node->right;
                if(!temp){
                    // No children
                    delete node;
                    return nullptr;
                } else {
                    // One child
                    *node = *temp; // Copy contents
                    delete temp;
                }
            } else {
                // Two children
                AVLNode<Key,Value>* temp = minNode(node->right);
                node->key = temp->key;
                node->values = temp->values;
                node->right = removeNode(node->right, temp->key);
            }
        }

        if(!node) return nullptr;

        // Update height and balance
        node->height = 1 + max(height(node->left), height(node->right));
        int bal = getBalance(node);

        // Perform rotations if needed
        if(bal > 1 && getBalance(node->left) >= 0) 
            return rightRotate(node);
        if(bal > 1 && getBalance(node->left) < 0){
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if(bal < -1 && getBalance(node->right) <= 0) 
            return leftRotate(node);
        if(bal < -1 && getBalance(node->right) > 0){
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

public:
    AVLNode<Key,Value>* root;

    AVLTree(){ root = nullptr; }
    ~AVLTree(){ clear(root); }
    
    void clear(AVLNode<Key,Value>* node){
        if(node){
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

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
            // Key exists - add value to existing node
            node->values.push_back(value);
            return node;
        }

        node->height = 1 + max(height(node->left), height(node->right));
        int bal = getBalance(node);

        // Left Left Case
        if(bal > 1 && key < node->left->key) 
            return rightRotate(node);
        
        // Right Right Case  
        if(bal < -1 && key > node->right->key) 
            return leftRotate(node);
        
        // Left Right Case
        if(bal > 1 && key > node->left->key){
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        
        // Right Left Case
        if(bal < -1 && key < node->right->key){
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void insert(const Key &key, const Value &value){
        root = insertNode(root, key, value);
    }

    // ---------- REMOVE SINGLE VALUE ----------
    bool removeValue(const Key &key, const Value &value){
        // Find the node first
        AVLNode<Key,Value>* current = root;
        AVLNode<Key,Value>* parent = nullptr;
        
        while(current){
            if(key < current->key){
                parent = current;
                current = current->left;
            } else if(key > current->key){
                parent = current;
                current = current->right;
            } else {
                break; // Found node
            }
        }
        
        if(!current) return false; // Node not found

        // Remove the specific value from node's values
        auto it = find(current->values.begin(), current->values.end(), value);
        if(it == current->values.end()) return false; // Value not found
        
        current->values.erase(it);
        
        // If node still has values, we're done
        if(!current->values.empty()) return true;
        
        // Node is empty - remove entire node
        root = removeNode(root, key);
        return true;
    }

    // ---------- REMOVE ENTIRE NODE ----------
    bool remove(const Key &key){
        if(!root) return false;
        root = removeNode(root, key);
        return true;
    }

    // ---------- RANGE QUERY ----------
    void rangeQueryHelper(AVLNode<Key,Value>* node, const Key &low, const Key &high, vector<Value> &out){
        if(!node) return;

        if(low < node->key) 
            rangeQueryHelper(node->left, low, high, out);
        
        if(node->key >= low && node->key <= high){
            for(const auto &v : node->values)
                out.push_back(v);
        }
        
        if(high > node->key) 
            rangeQueryHelper(node->right, low, high, out);
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
    
    // ---------- SEARCH ----------
    vector<Value>* find(const Key &key){
        AVLNode<Key,Value>* current = root;
        while(current){
            if(key < current->key)
                current = current->left;
            else if(key > current->key)
                current = current->right;
            else
                return &(current->values);
        }
        return nullptr;
    }
};
