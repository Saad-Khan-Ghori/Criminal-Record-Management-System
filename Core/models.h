#pragma once
#include <string>
#include <vector>

enum class CaseStage { Reported, Assigned, Investigation, Closed };

struct CrimeReport {
    std::string id;        // "C1001"
    std::string type;      // "Theft", "Assault", ...
    int severity = 1;      // 1..5
    long long epoch = 0;   // timestamp
    std::string areaId;    // "A7"
    std::string notes;
    std::string officerId; // "O12" or empty
    CaseStage stage = CaseStage::Reported;
};

struct Officer {
    std::string id;        // "O12"
    std::string name;
    std::string role;      // "Investigator", "Forensics"...
    std::string areaId;    // base area
    int maxLoad = 8;
    int curLoad = 0;
};

struct Area {
    std::string id;        // "A1"
    std::string name;      // "Central Precinct"
};

struct Road {
    std::string from;      // "A1"
    std::string to;        // "A7"
    double distKm = 0.0;
    bool blocked = false;
};
