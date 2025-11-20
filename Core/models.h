#pragma once
#include <string>
#include <vector>
using namespace std;

enum class CaseStage { Reported, Assigned, Investigation, Closed };

struct CrimeReport {
    string id;        // "C1001"
    string type;      // "Theft", "Assault", ...
    int severity = 1;      // 1..5
    long long epoch = 0;   // timestamp
    string areaId;    // "A7"
    string notes;
    string officerId; // "O12" or empty
    CaseStage stage = CaseStage::Reported;
};

struct Officer {
    string id;        // "O12"
    string name;
    string role;      // "Investigator", "Forensics"...
    string areaId;    // base area
    int maxLoad = 8;
    int curLoad = 0;
};

struct Area {
    string id;        // "A1"
    string name;      // "Central Precinct"
};

struct Road {
    string from;      // "A1"
    string to;        // "A7"
    double distKm = 0.0;
    bool blocked = false;
};

// CrimeKey for AVL tree indexing (time + severity)
struct CrimeKey {
    long long epoch;
    int severity;

    bool operator<(const CrimeKey& other) const {
        if (epoch != other.epoch)
            return epoch < other.epoch;  // Smaller time first
        return severity > other.severity; // Higher severity first for same time
    }

    bool operator>(const CrimeKey& other) const {
        if (epoch != other.epoch)
            return epoch > other.epoch;  // Larger time first  
        return severity < other.severity; // Lower severity first for same time
    }

    bool operator==(const CrimeKey& other) const {
        return epoch == other.epoch && severity == other.severity;
    }

    bool operator<=(const CrimeKey& other) const {
        return !(other < *this);
    }

    bool operator>=(const CrimeKey& other) const {
        return !(*this < other);
    }

    bool operator!=(const CrimeKey& other) const {
        return !(*this == other);
    }
};
