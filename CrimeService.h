#ifndef CRIME_SERVICE_H
#define CRIME_SERVICE_H

#include "Models.h"
#include "hashMap.h"
#include "AVLTree.h"
#include "Graph.h"
#include <string>
#include <vector>
using namespace std;

class CrimeService {
private:
    HashMap<string, CrimeReport> crimes_;
    HashMap<string, Officer> officers_;
    HashMap<string, Area> areas_;
    AVLTree<CrimeKey, string> crimeIndex_;   // CrimeKey 12 crimeID
    Graph cityGraph_;

public:
    CrimeService();

    // Area operations
    bool addArea(const Area& area);
    Area* findArea(const string& areaId);
    vector<Area> getAllAreas() const; // new

    // Road operations  
    bool addRoad(const Road& road);

    // Crime operations
    bool addCrime(const CrimeReport& report);
    CrimeReport* findCrime(const string& crimeId);
    bool updateCrime(const string& crimeId, const CrimeReport& updated);
    bool deleteCrime(const string& crimeId);

    // Officer operations
    bool addOfficer(const Officer& officer);
    Officer* findOfficer(const string& officerId);
    bool updateOfficer(const string& officerId, const Officer& updated);
    bool deleteOfficer(const string& officerId);
    vector<Officer> getAllOfficers() const; // new

    // Assignment
    bool assignOfficer(const string& crimeId, const string& officerId);
    bool advanceCrimeStage(const string& crimeId, CaseStage newStage);

    // AVL-based queries
    vector<string> getCrimesInTimeRange(long long startEpoch, long long endEpoch);
    vector<string> getRecentCrimes(int k);
    vector<string> getHighSeverityCrimes(int minSeverity);

    // Graph operations
    vector<string> shortestRoute(const string& fromArea, const string& toArea, double& totalDist);
    vector<string> getNearbyAreas(const string& areaId, int maxHops);

    // Utility
    int getTotalCrimes() const { return crimes_.size(); }
    int getTotalOfficers() const { return officers_.size(); }
};

#endif
