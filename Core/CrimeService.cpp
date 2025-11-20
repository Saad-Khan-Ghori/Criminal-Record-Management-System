#include <iostream>
#include "CrimeService.h"
#include <algorithm>
using namespace std;


CrimeService::CrimeService() {}

// ----------------------------- AREA OPS ------------------------------
bool CrimeService::addArea(const Area& area) {
    return areas_.insert(area.id, area);
}

Area* CrimeService::findArea(const string& areaId) {
    return areas_.find(areaId);
}

// ----------------------------- ROAD OPS ------------------------------
bool CrimeService::addRoad(const Road& road) {
    cityGraph_.addRoad(road.from, road.to, road.distKm, road.blocked);
    return true;
}

// ----------------------------- CRIME OPS ------------------------------
bool CrimeService::addCrime(const CrimeReport& report) {
    if (!crimes_.insert(report.id, report))
        return false; // duplicate ID

    // Add to AVL index with proper CrimeKey
    CrimeKey key{ report.epoch, report.severity };
    crimeIndex_.insert(key, report.id);

    return true;
}

CrimeReport* CrimeService::findCrime(const string& crimeId) {
    return crimes_.find(crimeId);
}

bool CrimeService::updateCrime(const string& crimeId, const CrimeReport& updated) {
    CrimeReport* old = crimes_.find(crimeId);
    if (!old) return false;

    // Remove from AVL index with old key
    CrimeKey oldKey{ old->epoch, old->severity };
    crimeIndex_.removeValue(oldKey, crimeId);

    // Update crime data
    *old = updated;

    // Re-insert with new key
    CrimeKey newKey{ updated.epoch, updated.severity };
    crimeIndex_.insert(newKey, crimeId);

    return true;
}

bool CrimeService::deleteCrime(const string& crimeId) {
    CrimeReport* crime = crimes_.find(crimeId);
    if (!crime) return false;

    // Remove from AVL index
    CrimeKey key{ crime->epoch, crime->severity };
    crimeIndex_.removeValue(key, crimeId);

    return crimes_.erase(crimeId);
}

// ----------------------------- OFFICER OPS ------------------------------
bool CrimeService::addOfficer(const Officer& officer) {
    return officers_.insert(officer.id, officer);
}

Officer* CrimeService::findOfficer(const string& officerId) {
    return officers_.find(officerId);
}

bool CrimeService::updateOfficer(const string& officerId, const Officer& updated) {
    Officer* old = officers_.find(officerId);
    if (!old) return false;
    *old = updated;
    return true;
}

bool CrimeService::deleteOfficer(const string& officerId) {
    return officers_.erase(officerId);
}

// ----------------------------- ASSIGNMENT ------------------------------
bool CrimeService::assignOfficer(const string& crimeId, const string& officerId) {
    CrimeReport* crime = crimes_.find(crimeId);
    Officer* officer = officers_.find(officerId);

    if (!crime || !officer) return false;

    // Check officer capacity - FIXED: Use proper comparison
    if (officer->curLoad >= officer->maxLoad) {
        cout << "Officer " << officerId << " is at full capacity ("
            << officer->curLoad << "/" << officer->maxLoad << ")" << endl;
        return false;
    }

    // If crime already assigned to ANOTHER officer, decrease their load
    if (!crime->officerId.empty() && crime->officerId != officerId) {
        Officer* prevOfficer = officers_.find(crime->officerId);
        if (prevOfficer) {
            prevOfficer->curLoad--;
            cout << "Decreased load for previous officer " << prevOfficer->id
                << " to " << prevOfficer->curLoad << endl;
        }
    }

    // Only increase load if this is a NEW assignment
    if (crime->officerId != officerId) {
        officer->curLoad++;
    }

    // Assign officer
    crime->officerId = officerId;
    crime->stage = CaseStage::Assigned;

    cout << "Assigned officer " << officerId << " to crime " << crimeId
        << " (new load: " << officer->curLoad << "/" << officer->maxLoad << ")" << endl;

    return true;
}

bool CrimeService::advanceCrimeStage(const string& crimeId, CaseStage newStage) {
    CrimeReport* crime = crimes_.find(crimeId);
    if (!crime) return false;

    crime->stage = newStage;
    return true;
}

// ----------------------------- AVL QUERIES ------------------------------
vector<string> CrimeService::getCrimesInTimeRange(long long startEpoch, long long endEpoch) {
    CrimeKey lowKey{ startEpoch, 1 };   // Min severity for lower bound
    CrimeKey highKey{ endEpoch, 5 };    // Max severity for upper bound

    vector<string> crimeIds = crimeIndex_.rangeQuery(lowKey, highKey);

    // Filter to ensure we only get crimes in the exact time range
    vector<string> result;
    for (const auto& crimeId : crimeIds) {
        CrimeReport* crime = findCrime(crimeId);
        if (crime && crime->epoch >= startEpoch && crime->epoch <= endEpoch) {
            result.push_back(crimeId);
        }
    }

    return result;
}

vector<string> CrimeService::getRecentCrimes(int k) {
    vector<pair<CrimeKey, string>> latest = crimeIndex_.getLastK(k);
    vector<string> result;

    // FIXED: Use explicit type instead of auto to avoid confusion
    for (const pair<CrimeKey, string>& crimePair : latest) {
        result.push_back(crimePair.second);
    }
    return result;
}

vector<string> CrimeService::getHighSeverityCrimes(int minSeverity) {
    // This would need additional indexing by severity
    // For now, we'll return empty - you can implement with another AVL tree
    return vector<string>();
}

// ----------------------------- GRAPH OPERATIONS ------------------------------
vector<string> CrimeService::shortestRoute(const string& fromArea, const string& toArea, double& totalDist) {
    vector<string> path;
    bool found = cityGraph_.shortestPath(fromArea, toArea, path, totalDist);
    return found ? path : vector<string>();
}

vector<string> CrimeService::getNearbyAreas(const string& areaId, int maxHops) {
    return cityGraph_.kHopNeighborhood(areaId, maxHops);
}
