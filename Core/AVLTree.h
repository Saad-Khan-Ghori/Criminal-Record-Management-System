#pragma once
#include <algorithm>
#include "AVLNode.h"
using namespace std;

template<typename Key,typename Value>
class AVLTree{
public:
    AVLNode<Key,Value>* root;

    AVLTree(){root=nullptr;}

    int height(AVLNode<Key,Value>* n){
        return n?n->height:0;
    }

    int getBalance(AVLNode<Key,Value>* n){
        return n?height(n->left)-height(n->right):0;
    }

    AVLNode<Key,Value>* rightRotate(AVLNode<Key,Value>* y){
        auto x=y->left;
        auto T2=x->right;
        x->right=y;
        y->left=T2;
        y->height=max(height(y->left),height(y->right))+1;
        x->height=max(height(x->left),height(x->right))+1;
        return x;
    }

    AVLNode<Key,Value>* leftRotate(AVLNode<Key,Value>* x){
        auto y=x->right;
        auto T2=y->left;
        y->left=x;
        x->right=T2;
        x->height=max(height(x->left),height(x->right))+1;
        y->height=max(height(y->left),height(y->right))+1;
        return y;
    }

    AVLNode<Key,Value>* insert(AVLNode<Key,Value>* node,const Key &key,const Value &value){
        if(!node) return new AVLNode<Key,Value>(key,value);

        if(key<node->key) node->left=insert(node->left,key,value);
        else if(key>node->key) node->right=insert(node->right,key,value);
        else{ 
            node->values.push_back(value);
            return node;
        }

        node->height=1+max(height(node->left),height(node->right));
        int bal=getBalance(node);

        if(bal>1&&key<node->left->key) return rightRotate(node);
        if(bal<-1&&key>node->right->key) return leftRotate(node);
        if(bal>1&&key>node->left->key){
            node->left=leftRotate(node->left);
            return rightRotate(node);
        }
        if(bal<-1&&key<node->right->key){
            node->right=rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    void insert(const Key &key,const Value &value){
        root=insert(root,key,value);
    }
};
