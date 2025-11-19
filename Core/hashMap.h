#pragma once
#include <vector>
#include <functional>
#include <utility>
#include <string>
using namespace std;

template<typename K, typename V>
class HashMap{
private:
    vector<vector<pair<K,V>>> buckets;
    size_t count;

public:
    // ---------------- CONSTRUCTOR ----------------
    HashMap(size_t bucketCount = 16){
        buckets.resize(bucketCount);
        count = 0;
    }

    // ---------------- HASH FUNCTION ----------------
    size_t hashKey(const K &key) const{
        return hash<K>()(key) % buckets.size();
    }

    // ---------------- LOAD FACTOR ----------------
    double loadFactor() const{
        return (double)count / buckets.size();
    }

    size_t size() const{
        return count;
    }

    // ---------------- REHASH ----------------
    void rehash(size_t newBucketCount){
        vector<vector<pair<K,V>>> newBuckets(newBucketCount);

        for(auto &bucket : buckets){
            for(auto &kv : bucket){
                size_t idx = hash<K>()(kv.first) % newBucketCount;
                newBuckets[idx].push_back(kv);
            }
        }
        buckets.swap(newBuckets);
    }

    // ---------------- INSERT ----------------
    bool insert(const K &key, const V &value){
        size_t idx = hashKey(key);

        for(auto &kv : buckets[idx]){
            if(kv.first == key){
                kv.second = value;     // overwrite existing
                return false;          // indicates replacement
            }
        }

        buckets[idx].push_back({key, value});
        count++;

        if(loadFactor() > 0.75){
            rehash(buckets.size() * 2);
        }

        return true;   // new insert
    }

    // ---------------- FIND (mutable) ----------------
    V* find(const K &key){
        size_t idx = hashKey(key);
        for(auto &kv : buckets[idx]){
            if(kv.first == key) return &kv.second;
        }
        return nullptr;
    }

    // ---------------- FIND (const) ----------------
    const V* find(const K &key) const{
        size_t idx = hashKey(key);
        for(const auto &kv : buckets[idx]){
            if(kv.first == key) return &kv.second;
        }
        return nullptr;
    }

    // ---------------- CONTAINS ----------------
    bool contains(const K &key) const{
        return find(key) != nullptr;
    }

    // ---------------- ERASE ----------------
    bool erase(const K &key){
        size_t idx = hashKey(key);

        auto &bucket = buckets[idx];
        for(size_t i=0; i<bucket.size(); i++){
            if(bucket[i].first == key){
                bucket.erase(bucket.begin() + i);
                count--;
                return true;
            }
        }
        return false;
    }

    // ---------------- ITERATION ----------------
    void forEach(const function<void(const K&, V&)> &fn){
        for(auto &bucket : buckets){
            for(auto &kv : bucket){
                fn(kv.first, kv.second);
            }
        }
    }
};
