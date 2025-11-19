#include "CrimeService.h"
using namespace std;

CrimeService::CrimeService(){}

// ----------------------------- CRIME OPS ------------------------------

bool CrimeService::addCrime(const CrimeReport& report){
    if(!crimes_.insert(report.id, report)) return false; // duplicate ID -> fail
    crimeIndex_.insert(report.epoch, report.id);
    return true;
}

CrimeReport* CrimeService::findCrime(const string& crimeId){
    return crimes_.find(crimeId);
}

bool CrimeService::updateCrime(const string& crimeId, const CrimeReport& updated){
    CrimeReport* old = crimes_.find(crimeId);
    if(!old) return false;

    // If epoch changed, remove only this crimeId from its old key, then insert new
    if(updated.epoch != old->epoch){
        crimeIndex_.removeValue(old->epoch, old->id); // <-- use new removeValue
        crimeIndex_.insert(updated.epoch, updated.id);
    }

    *old = updated;
    return true;
}

bool CrimeService::deleteCrime(const string& crimeId){
    CrimeReport* old = crimes_.find(crimeId);
    if(!old) return false;

    // remove only this id from AVL index
    crimeIndex_.removeValue(old->epoch, old->id);

    return crimes_.erase(crimeId);
}

// ----------------------------- OFFICER OPS ------------------------------

bool CrimeService::addOfficer(const Officer& officer){
    return officers_.insert(officer.id, officer);
}

Officer* CrimeService::findOfficer(const string& officerId){
    return officers_.find(officerId);
}

bool CrimeService::updateOfficer(const string& officerId, const Officer& updated){
    Officer* old = officers_.find(officerId);
    if(!old) return false;
    *old = updated;
    return true;
}

bool CrimeService::deleteOfficer(const string& officerId){
    return officers_.erase(officerId);
}

// ----------------------------- ASSIGNMENT ------------------------------

bool CrimeService::assignOfficer(const string& crimeId, const string& officerId){
    CrimeReport* cr = crimes_.find(crimeId);
    Officer* of = officers_.find(officerId);

    if(!cr || !of) return false;
    if(of->curLoad >= of->maxLoad) return false;
    if(cr->officerId == officerId) return true;

    if(cr->officerId != ""){
        Officer* prev = officers_.find(cr->officerId);
        if(prev) prev->curLoad--;
    }

    cr->officerId = officerId;
    of->curLoad++;
    return true;
}

// ----------------------------- AVL QUERIES ------------------------------

vector<string> CrimeService::getCrimesInTimeRange(long long startEpoch, long long endEpoch){
    // AVLTree::rangeQuery returns vector<Value>
    vector<string> result = crimeIndex_.rangeQuery(startEpoch, endEpoch);
    return result;
}

vector<string> CrimeService::getRecentCrimes(int k){
    auto latest = crimeIndex_.getLastK(k); // returns vector<pair<long long,string>>
    vector<string> out;
    out.reserve(latest.size());
    for(auto &p : latest) out.push_back(p.second);
    return out;
}

