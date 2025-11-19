#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <functional>
#include <algorithm>
using namespace std;

struct Edge{
    int to;
    double distKm;
    bool blocked;
};

class Graph{
public:
    Graph(){}

    // add area, return its index (existing -> same index)
    int addArea(const string &areaId){
        auto it = indexOf_.find(areaId);
        if(it!=indexOf_.end()) return it->second;
        int idx = (int)areaIds_.size();
        areaIds_.push_back(areaId);
        indexOf_[areaId] = idx;
        adj_.emplace_back();
        return idx;
    }

    // add undirected road (creates areas if missing)
    void addRoad(const string &from,const string &to,double distKm,bool blocked=false){
        int u = addArea(from);
        int v = addArea(to);
        adj_[u].push_back({v,distKm,blocked});
        adj_[v].push_back({u,distKm,blocked});
    }

    // Dijkstra shortest path ignoring blocked edges
    // returns false if no path (outPath cleared), sets totalDist if true
    bool shortestPath(const string &fromId,const string &toId, vector<string> &outPath, double &totalDist) const{
        outPath.clear();
        totalDist = 0.0;
        auto itf = indexOf_.find(fromId);
        auto itt = indexOf_.find(toId);
        if(itf==indexOf_.end() || itt==indexOf_.end()) return false;
        int src = itf->second;
        int dst = itt->second;
        int n = (int)areaIds_.size();
        if(n==0) return false;

        const double INF = numeric_limits<double>::infinity();
        vector<double> dist(n, INF);
        vector<int> parent(n, -1);
        dist[src] = 0.0;

        using P = pair<double,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        pq.push({0.0, src});

        while(!pq.empty()){
            auto [d,u] = pq.top(); pq.pop();
            if(d>dist[u]) continue;
            if(u==dst) break;
            for(const auto &e : adj_[u]){
                if(e.blocked) continue;
                double nd = d + e.distKm;
                if(nd < dist[e.to]){
                    dist[e.to] = nd;
                    parent[e.to] = u;
                    pq.push({nd, e.to});
                }
            }
        }

        if(dist[dst]==INF) return false;

        // rebuild path
        vector<int> rev;
        for(int cur = dst; cur!=-1; cur = parent[cur]) rev.push_back(cur);
        reverse(rev.begin(), rev.end());
        outPath.reserve(rev.size());
        for(int idx : rev) outPath.push_back(areaIds_[idx]);
        totalDist = dist[dst];
        return true;
    }

    // BFS k-hop neighborhood (returns areaIds within k hops, excluding start)
    vector<string> kHopNeighborhood(const string &areaId,int k) const{
        vector<string> out;
        auto it = indexOf_.find(areaId);
        if(it==indexOf_.end() || k<=0) return out;
        int start = it->second;
        int n = (int)areaIds_.size();
        vector<bool> visited(n,false);
        queue<pair<int,int>> q;
        visited[start] = true;
        q.push({start,0});

        while(!q.empty()){
            auto [u,d] = q.front(); q.pop();
            if(d>=1) out.push_back(areaIds_[u]);
            if(d==k) continue;
            for(const auto &e : adj_[u]){
                if(e.blocked) continue; // treat blocked as unreachable for neighborhood
                if(!visited[e.to]){
                    visited[e.to] = true;
                    q.push({e.to, d+1});
                }
            }
        }
        return out;
    }

    // convenience accessors
    const vector<string>& areaIds() const { return areaIds_; }
    bool hasArea(const string &id) const { return indexOf_.find(id)!=indexOf_.end(); }

private:
    vector<string> areaIds_;
    unordered_map<string,int> indexOf_;
    vector<vector<Edge>> adj_;
};
