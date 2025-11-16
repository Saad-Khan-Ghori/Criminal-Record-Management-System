#include <iostream>
#include <vector>
#include <functional>
using namespace std;

template <typename T>
class Node{
public:
    T data;
    Node* link;
    Node(): link(NULL) {}
    Node(const T& d): data(d), link(NULL) {}
};

template <typename T>
class Queue{
public:
    Node<T>* head;   // front
    Node<T>* tail;   // rear (tail->link = head for circular)
    int count;

    Queue(){ head = tail = NULL; count = 0; }

    bool isEmpty(){ return head == NULL; }

    void enqueue(const T& d){
        Node<T>* temp = new Node<T>(d);

        if(isEmpty()){
            head = tail = temp;
            tail->link = head;        // circular link
        }
        else{
            tail->link = temp;
            tail = temp;
            tail->link = head;        // maintain circular link
        }
        count++;
    }

    void dequeue(){
        if(isEmpty()){
            cout << "Queue is empty\n";
            return;
        }

        if(head == tail){
            delete head;
            head = tail = NULL;
        }
        else{
            Node<T>* temp = head;
            head = head->link;
            tail->link = head;    // maintain circular
            delete temp;
        }
        count--;
    }

    T& front(){
        if(isEmpty()){
            cout << "Queue is empty\n";
            exit(1);
        }
        return head->data;
    }

    void clear(){
        while(!isEmpty()) dequeue();
    }

    bool contains(const T& value) const{
        if(isEmpty()) return false;

        Node<T>* temp = head;
        do{
            if(temp->data == value) return true;
            temp = temp->link;
        } while(temp != head);

        return false;
    }

    bool removeIf(const function<bool(const T&)>& pred){
        if(isEmpty()) return false;

        Node<T>* curr = head;
        Node<T>* prev = tail;

        do{
            if(pred(curr->data)){
                if(curr == head){
                    dequeue();
                }
                else{
                    prev->link = curr->link;
                    if(curr == tail) tail = prev;
                    delete curr;
                    count--;
                }
                return true;
            }
            prev = curr;
            curr = curr->link;
        } while(curr != head);

        return false;
    }

    vector<T> toVector() const{
        vector<T> v;
        if(isEmpty()) return v;

        Node<T>* temp = head;
        do{
            v.push_back(temp->data);
            temp = temp->link;
        } while(temp != head);

        return v;
    }
};
