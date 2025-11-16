#pragma once
#include <string>
#include <vector>
#include "CrimeKey.h"
#include "CrimeReport.h"
#include "Officer.h"
#include "Area.h"
#include "Road.h"
#include "Graph.h"
#include "AVLTree.h"
#include "Queue.h"

class CrimeService{
public:
    // ----- Data Loading / Setup -----
    bool addArea(const Area& a);
    bool addRoad(const Road& r);
    bool addOfficer(const Officer& o);

    // ----- Crime Operations -----
    bool addCrime(const CrimeReport& c);
    const CrimeReport* getCrime(const std::string& id) const;

    // ----- Assignment & Stages -----
    bool assignOfficer(const std::string& crimeId,const std::string& officerId);
    bool advanceStage(const std::string& crimeId,CaseStage nextStage);

    // ----- Queries -----
    std::vector<CrimeReport> rangeCrimes(long long fromEpoch,long long toEpoch) const;
    std::vector<std::string> activeQueuePeek(size_t k=20) const;

    // ----- Routing -----
    std::vector<std::string> shortestRoute(const std::string& areaFrom,const std::string& areaTo,double& totalDist) const;

private:
    // main storage
    HashMap<std::string,CrimeReport> crimes_;
    HashMap<std::string,Officer> officers_;

    // AVL ordered index by time/severity
    AVLTree<CrimeKey,std::string> crimesByTime_;

    // city graph
    Graph city_;

    // active processing queue
    Queue<std::string> activeQueue_;
};
