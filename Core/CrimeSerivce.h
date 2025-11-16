#ifndef CRIME_SERVICE_H
#define CRIME_SERVICE_H

#include "Models.h"
#include "HashMap.h"
#include "AVLTree.h"
#include <string>
#include <vector>
using namespace std;

class CrimeService {
    HashMap<string, CrimeReport> crimes_;
    HashMap<string, Officer> officers_;
    AVLTree<long long, string> crimeIndex_;   // epoch → crimeID (for sorting by time)

public:
    CrimeService();

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

    // Assignment
    bool assignOfficer(const string& crimeId, const string& officerId);

    // AVL-based queries
    vector<string> getCrimesInTimeRange(long long startEpoch, long long endEpoch);
    vector<string> getRecentCrimes(int k);
};

#endif
