#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include "CrimeService.h"

using namespace std;
using namespace std::chrono;

// Helper function to get current epoch time
long long getCurrentEpoch() {
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

// Helper function to print crime details
void printCrime(const CrimeReport& crime) {
    cout << "  ID: " << crime.id
        << " | Type: " << crime.type
        << " | Severity: " << crime.severity
        << " | Area: " << crime.areaId
        << " | Officer: " << (crime.officerId.empty() ? "None" : crime.officerId)
        << " | Stage: ";
    switch (crime.stage) {
    case CaseStage::Reported: cout << "Reported"; break;
    case CaseStage::Assigned: cout << "Assigned"; break;
    case CaseStage::Investigation: cout << "Investigation"; break;
    case CaseStage::Closed: cout << "Closed"; break;
    }
    cout << " | Time: " << crime.epoch << endl;
}

// Helper function to print officer details
void printOfficer(const Officer& officer) {
    cout << "  ID: " << officer.id
        << " | Name: " << officer.name
        << " | Role: " << officer.role
        << " | Area: " << officer.areaId
        << " | Load: " << officer.curLoad << "/" << officer.maxLoad << endl;
}

// Forward declarations needed because some helper functions call these before their definitions
void assignOfficerInteractive(CrimeService& svc, const string& preSelectedCrimeId = "");

// Helper function to select from a list by number
template<typename T>
bool selectFromList(const vector<T>& items, const string& prompt, int& selectedIndex, 
                   function<void(int, const T&)> printItem) {
    if (items.empty()) {
        cout << "No items available.\n";
        return false;
    }

    cout << prompt << "\n";
    for (size_t i = 0; i < items.size(); ++i) {
        cout << "  " << (i + 1) << ". ";
        printItem(i, items[i]);
    }
    cout << "  0. Cancel\n";
    cout << "Select option: ";

    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) return false;
        if (choice < 1 || choice > static_cast<int>(items.size())) {
            cout << "Invalid selection.\n";
            return false;
        }
        selectedIndex = choice - 1;
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Helper to select crime by number
bool selectCrime(CrimeService& svc, string& crimeId) {
    auto crimeIds = svc.getRecentCrimes(1000);
    if (crimeIds.empty()) {
        cout << "No crimes available.\n";
        return false;
    }
    
    cout << "Select crime:\n";
    vector<CrimeReport*> crimes;
    for (size_t i = 0; i < crimeIds.size(); ++i) {
        CrimeReport* c = svc.findCrime(crimeIds[i]);
        if (c) {
            crimes.push_back(c);
            cout << "  " << (crimes.size()) << ". ";
            printCrime(*c);
        }
    }
    if (crimes.empty()) {
        cout << "No crimes found.\n";
        return false;
    }
    cout << "  0. Cancel\n";
    cout << "Select option: ";
    
    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) return false;
        if (choice < 1 || choice > static_cast<int>(crimes.size())) {
            cout << "Invalid selection.\n";
            return false;
        }
        crimeId = crimes[choice - 1]->id;
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Helper to select officer by number
bool selectOfficer(CrimeService& svc, string& officerId, const string& filterArea = "", bool onlyAvailable = false) {
    vector<Officer> allOfficers = svc.getAllOfficers();
    vector<Officer> filtered;
    
    for (const auto& o : allOfficers) {
        if (!filterArea.empty() && o.areaId != filterArea) continue;
        if (onlyAvailable && o.curLoad >= o.maxLoad) continue;
        filtered.push_back(o);
    }
    
    if (filtered.empty()) {
        cout << "No officers available";
        if (!filterArea.empty()) cout << " in area " << filterArea;
        if (onlyAvailable) cout << " with available capacity";
        cout << ".\n";
        return false;
    }
    
    cout << "Select officer:\n";
    for (size_t i = 0; i < filtered.size(); ++i) {
        cout << "  " << (i + 1) << ". ";
        printOfficer(filtered[i]);
    }
    cout << "  0. Cancel\n";
    cout << "Select option: ";
    
    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) return false;
        if (choice < 1 || choice > static_cast<int>(filtered.size())) {
            cout << "Invalid selection.\n";
            return false;
        }
        officerId = filtered[choice - 1].id;
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Helper to select area by number
bool selectArea(CrimeService& svc, string& areaId) {
    vector<Area> areas = svc.getAllAreas();
    if (areas.empty()) {
        cout << "No areas available.\n";
        return false;
    }
    
    cout << "Select area:\n";
    for (size_t i = 0; i < areas.size(); ++i) {
        cout << "  " << (i + 1) << ". " << areas[i].id << " - " << areas[i].name << "\n";
    }
    cout << "  0. Cancel\n";
    cout << "Select option: ";
    
    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) return false;
        if (choice < 1 || choice > static_cast<int>(areas.size())) {
            cout << "Invalid selection.\n";
            return false;
        }
        areaId = areas[choice - 1].id;
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Helper to select crime type
bool selectCrimeType(string& type) {
    vector<string> types = {"Theft", "Assault", "Burglary", "Vandalism", "Fraud", 
                           "Drug Offense", "Traffic Violation", "Other"};
    cout << "Select crime type:\n";
    for (size_t i = 0; i < types.size(); ++i) {
        cout << "  " << (i + 1) << ". " << types[i] << "\n";
    }
    cout << "  0. Enter custom type\n";
    cout << "Select option: ";
    
    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) {
            cout << "Enter custom crime type: ";
            if (!getline(cin, type) || type.empty()) {
                cout << "Invalid type.\n";
                return false;
            }
            return true;
        }
        if (choice < 1 || choice > static_cast<int>(types.size())) {
            cout << "Invalid selection.\n";
            return false;
        }
        type = types[choice - 1];
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Helper to select officer role
bool selectOfficerRole(string& role) {
    vector<string> roles = {"Investigator", "Patrol Officer", "Detective", "Forensics", 
                            "Supervisor", "Specialist", "Other"};
    cout << "Select officer role:\n";
    for (size_t i = 0; i < roles.size(); ++i) {
        cout << "  " << (i + 1) << ". " << roles[i] << "\n";
    }
    cout << "  0. Enter custom role\n";
    cout << "Select option: ";
    
    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) {
            cout << "Enter custom role: ";
            if (!getline(cin, role) || role.empty()) {
                cout << "Invalid role.\n";
                return false;
            }
            return true;
        }
        if (choice < 1 || choice > static_cast<int>(roles.size())) {
            cout << "Invalid selection.\n";
            return false;
        }
        role = roles[choice - 1];
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Helper to select case stage
bool selectCaseStage(CaseStage& stage) {
    cout << "Select case stage:\n";
    cout << "  1. Reported\n";
    cout << "  2. Assigned\n";
    cout << "  3. Investigation\n";
    cout << "  4. Closed\n";
    cout << "  0. Cancel\n";
    cout << "Select option: ";
    
    string line;
    if (!getline(cin, line)) return false;
    try {
        int choice = stoi(line);
        if (choice == 0) return false;
        if (choice < 1 || choice > 4) {
            cout << "Invalid selection.\n";
            return false;
        }
        stage = static_cast<CaseStage>(choice - 1);
        return true;
    } catch(...) {
        cout << "Invalid input.\n";
        return false;
    }
}

// Read integer with validation and no extra input allowed
bool readIntValidated(int &out) {
    string line;
    if(!std::getline(cin, line)) return false;
    try {
        size_t idx;
        int val = stoi(line, &idx);
        if (idx != line.size()) return false;
        out = val;
        return true;
    } catch(...) { return false; }
}

// Read long long validated
bool readLongLongValidated(long long &out) {
    string line;
    if(!getline(cin, line)) return false;
    try {
        size_t idx;
        long long val = stoll(line, &idx);
        if (idx != line.size()) return false;
        out = val;
        return true;
    } catch(...) { return false; }
}

void addAreaInteractive(CrimeService& svc) {
    Area a;
    cout << "Enter area id: ";
    if(!getline(cin, a.id) || a.id.empty()) { cout << "Invalid id\n"; return; }
    cout << "Enter area name (or press Enter for empty): ";
    if(!getline(cin, a.name)) a.name = "";
    if (svc.addArea(a)) cout << "Added area " << a.id << "\n";
    else cout << "Failed to add area (maybe duplicate)\n";
}

void addRoadInteractive(CrimeService& svc) {
    Road r;
    if (!selectArea(svc, r.from)) return;
    cout << "To area:\n";
    if (!selectArea(svc, r.to)) return;
    cout << "Distance (km): ";
    string dline; if(!getline(cin, dline)){ cout<<"Invalid\n"; return; }
    try { r.distKm = stod(dline); } catch(...) { cout<<"Invalid distance\n"; return; }
    cout << "Blocked? (y/n): ";
    string bline; if(!getline(cin, bline)) return; char blocked = bline.empty()? 'n' : bline[0];
    r.blocked = (blocked=='y' || blocked=='Y');
    if (svc.addRoad(r)) cout << "Added road from " << r.from << " to " << r.to << "\n";
}

void addOfficerInteractive(CrimeService& svc) {
    Officer o;
    cout << "Officer id: "; if(!getline(cin, o.id) || o.id.empty()){ cout<<"Invalid\n"; return; }
    cout << "Name: "; getline(cin, o.name);
    if (!selectOfficerRole(o.role)) return;
    cout << "Base area:\n";
    if (!selectArea(svc, o.areaId)) return;
    cout << "Max load (number, default 8): ";
    string mline; if(!getline(cin, mline) || mline.empty()) { o.maxLoad = 8; }
    else {
        try { o.maxLoad = stoi(mline); } catch(...) { cout<<"Invalid number, using default 8\n"; o.maxLoad = 8; }
    }
    o.curLoad = 0;
    if (svc.addOfficer(o)) cout << "Added officer " << o.id << "\n";
    else cout << "Failed to add officer (maybe duplicate)\n";
}

void addCrimeInteractive(CrimeService& svc) {
    CrimeReport c;
    cout << "Crime id: "; if(!getline(cin, c.id) || c.id.empty()){ cout<<"Invalid\n"; return; }
    if (!selectCrimeType(c.type)) return;
    cout << "Severity (1-5, where 5 is most severe): "; 
    string sline; if(!getline(cin, sline)){ cout<<"Invalid\n"; return; }
    try { 
        c.severity = stoi(sline); 
        if(c.severity<1 || c.severity>5) { 
            cout<<"Severity out of range, using 3 (medium)\n"; 
            c.severity = 3; 
        } 
    } catch(...) { 
        cout<<"Invalid, using default 3 (medium)\n"; 
        c.severity = 3; 
    }
    c.epoch = getCurrentEpoch();
    cout << "Area:\n";
    if (!selectArea(svc, c.areaId)) return;
    cout << "Notes (or press Enter for none): "; getline(cin, c.notes);
    c.officerId = "";
    c.stage = CaseStage::Reported;
    if (svc.addCrime(c)) {
        cout << "Added crime " << c.id << "\n";
        // Offer to assign officer immediately
        cout << "\nWould you like to assign an officer now? (y/n): ";
        string assign;
        if (getline(cin, assign) && (assign == "y" || assign == "Y")) {
            assignOfficerInteractive(svc, c.id);
        }
    } else {
        cout << "Failed to add crime (maybe duplicate id)\n";
    }
}

void listCrimes(CrimeService& svc) {
    cout << "All crimes (IDs) in system:\n";
    auto ids = svc.getRecentCrimes(1000);
    for (const auto &id : ids) {
        CrimeReport* c = svc.findCrime(id);
        if (c) printCrime(*c);
    }
}

void listOfficers(CrimeService& svc) {
    cout << "All officers:\n";
    vector<Officer> officers = svc.getAllOfficers();
    for (const auto &o : officers) printOfficer(o);
}

void listAreas(CrimeService& svc) {
    cout << "All areas:\n";
    vector<Area> areas = svc.getAllAreas();
    for (const auto &a : areas) cout << "  " << a.id << " | " << a.name << "\n";
}

void assignOfficerInteractive(CrimeService& svc, const string& preSelectedCrimeId) {
    string crimeId = preSelectedCrimeId;
    if (crimeId.empty()) {
        if (!selectCrime(svc, crimeId)) return;
    }
    
    CrimeReport* crime = svc.findCrime(crimeId);
    if (!crime) {
        cout << "Crime not found!\n";
        return;
    }
    
    // Show crime info
    cout << "\nAssigning officer to crime:\n";
    printCrime(*crime);
    cout << "\n";
    
    // Smart filtering: prefer officers in same area with available capacity
    string officerId;
    cout << "Available officers (filtered by area and capacity):\n";
    if (!selectOfficer(svc, officerId, crime->areaId, true)) {
        // If no officers in same area, show all available
        cout << "\nNo officers available in area " << crime->areaId 
             << ". Showing all available officers:\n";
        if (!selectOfficer(svc, officerId, "", true)) {
            cout << "No available officers found.\n";
            return;
        }
    }
    
    if (svc.assignOfficer(crimeId, officerId)) {
        cout << "Assignment successful!\n";
    } else {
        cout << "Assignment failed\n";
    }
}

void advanceStageInteractive(CrimeService& svc) {
    string cid;
    if (!selectCrime(svc, cid)) return;
    
    CrimeReport* crime = svc.findCrime(cid);
    if (!crime) {
        cout << "Crime not found!\n";
        return;
    }
    
    cout << "\nCurrent stage: ";
    switch (crime->stage) {
        case CaseStage::Reported: cout << "Reported"; break;
        case CaseStage::Assigned: cout << "Assigned"; break;
        case CaseStage::Investigation: cout << "Investigation"; break;
        case CaseStage::Closed: cout << "Closed"; break;
    }
    cout << "\n\n";
    
    CaseStage newStage;
    if (!selectCaseStage(newStage)) return;
    
    if (svc.advanceCrimeStage(cid, newStage)) {
        cout << "Stage updated successfully!\n";
    } else {
        cout << "Failed to update stage\n";
    }
}

void recentCrimesInteractive(CrimeService& svc) {
    cout << "How many recent crimes to show? ";
    string sl; if(!getline(cin, sl)){ cout<<"Invalid\n"; return; }
    int k;
    try { k = stoi(sl); } catch(...) { cout<<"Invalid\n"; return; }
    auto list = svc.getRecentCrimes(k);
    for (const auto &id : list) {
        CrimeReport* c = svc.findCrime(id);
        if (c) printCrime(*c);
    }
}

void crimesInRangeInteractive(CrimeService& svc) {
    cout << "Select time range:\n";
    cout << "  1. Last hour\n";
    cout << "  2. Last 24 hours\n";
    cout << "  3. Last week (7 days)\n";
    cout << "  4. Custom range\n";
    cout << "  0. Cancel\n";
    cout << "Select option: ";
    
    string choice;
    if (!getline(cin, choice)) { cout << "Invalid\n"; return; }
    
    long long s, e;
    long long now = getCurrentEpoch();
    
    try {
        int opt = stoi(choice);
        switch(opt) {
            case 1: 
                s = now - 3600; 
                e = now; 
                break;
            case 2: 
                s = now - 86400; 
                e = now; 
                break;
            case 3: 
                s = now - 604800; 
                e = now; 
                break;
            case 4:
                cout << "Enter start epoch (or 0 for now-3600): "; 
                if(!readLongLongValidated(s)){ cout<<"Invalid\n"; return; }
                cout << "Enter end epoch (or 0 for now): "; 
                if(!readLongLongValidated(e)){ cout<<"Invalid\n"; return; }
                if (s==0) s = now - 3600;
                if (e==0) e = now;
                break;
            case 0: return;
            default: cout << "Invalid option\n"; return;
        }
    } catch(...) {
        cout << "Invalid input\n";
        return;
    }
    
    auto ids = svc.getCrimesInTimeRange(s, e);
    if (ids.empty()) {
        cout << "No crimes found in the specified time range.\n";
    } else {
        cout << "\nFound " << ids.size() << " crime(s) in time range:\n";
        for (const auto &id : ids) {
            CrimeReport* c = svc.findCrime(id);
            if (c) printCrime(*c);
        }
    }
}

void shortestRouteInteractive(CrimeService& svc) {
    string from, to;
    cout << "From area:\n";
    if (!selectArea(svc, from)) return;
    cout << "To area:\n";
    if (!selectArea(svc, to)) return;
    
    double total;
    auto path = svc.shortestRoute(from, to, total);
    if (path.empty()) {
        cout << "No path found between " << from << " and " << to << "\n";
    } else {
        cout << "\nShortest route:\n";
        cout << "Path: ";
        for (size_t i=0;i<path.size();++i){ 
            Area* area = svc.findArea(path[i]);
            if (area) cout << area->name << " (" << path[i] << ")";
            else cout << path[i];
            if (i+1<path.size()) cout << " -> "; 
        }
        cout << "\nTotal distance: " << total << " km\n";
    }
}

// Additional helper functions for new features
void viewCrimeDetails(CrimeService& svc) {
    string crimeId;
    if (!selectCrime(svc, crimeId)) return;
    
    CrimeReport* crime = svc.findCrime(crimeId);
    if (!crime) {
        cout << "Crime not found!\n";
        return;
    }
    
    cout << "\n=== CRIME DETAILS ===\n";
    cout << "ID: " << crime->id << "\n";
    cout << "Type: " << crime->type << "\n";
    cout << "Severity: " << crime->severity << "/5\n";
    cout << "Area: " << crime->areaId << "\n";
    cout << "Officer: " << (crime->officerId.empty() ? "Not assigned" : crime->officerId) << "\n";
    cout << "Stage: ";
    switch (crime->stage) {
        case CaseStage::Reported: cout << "Reported"; break;
        case CaseStage::Assigned: cout << "Assigned"; break;
        case CaseStage::Investigation: cout << "Investigation"; break;
        case CaseStage::Closed: cout << "Closed"; break;
    }
    cout << "\n";
    cout << "Time: " << crime->epoch << "\n";
    cout << "Notes: " << (crime->notes.empty() ? "None" : crime->notes) << "\n";
    cout << "===================\n";
    
    // Quick actions
    if (crime->stage != CaseStage::Closed) {
        cout << "\nQuick Actions:\n";
        if (crime->officerId.empty()) {
            cout << "  Assign officer? (y/n): ";
            string assign;
            if (getline(cin, assign) && (assign == "y" || assign == "Y")) {
                assignOfficerInteractive(svc, crimeId);
            }
        }
        cout << "  Update stage? (y/n): ";
        string update;
        if (getline(cin, update) && (update == "y" || update == "Y")) {
            advanceStageInteractive(svc);
        }
    }
}

void viewOfficerDetails(CrimeService& svc) {
    string officerId;
    if (!selectOfficer(svc, officerId)) return;
    
    Officer* officer = svc.findOfficer(officerId);
    if (!officer) {
        cout << "Officer not found!\n";
        return;
    }
    
    cout << "\n=== OFFICER DETAILS ===\n";
    cout << "ID: " << officer->id << "\n";
    cout << "Name: " << officer->name << "\n";
    cout << "Role: " << officer->role << "\n";
    cout << "Base Area: " << officer->areaId << "\n";
    cout << "Workload: " << officer->curLoad << "/" << officer->maxLoad;
    if (officer->curLoad >= officer->maxLoad) cout << " (FULL)";
    else if (officer->curLoad == 0) cout << " (AVAILABLE)";
    cout << "\n";
    
    // Show assigned crimes
    vector<string> allCrimes = svc.getRecentCrimes(1000);
    int assignedCount = 0;
    cout << "\nAssigned Crimes:\n";
    for (const auto& crimeId : allCrimes) {
        CrimeReport* crime = svc.findCrime(crimeId);
        if (crime && crime->officerId == officerId) {
            assignedCount++;
            cout << "  - " << crime->id << " (" << crime->type << ", Severity: " << crime->severity << ")\n";
        }
    }
    if (assignedCount == 0) cout << "  None\n";
    cout << "======================\n";
}

void viewAreaDetails(CrimeService& svc) {
    string areaId;
    if (!selectArea(svc, areaId)) return;
    
    Area* area = svc.findArea(areaId);
    if (!area) {
        cout << "Area not found!\n";
        return;
    }
    
    cout << "\n=== AREA DETAILS ===\n";
    cout << "ID: " << area->id << "\n";
    cout << "Name: " << area->name << "\n";
    
    // Count crimes in this area
    vector<string> allCrimes = svc.getRecentCrimes(1000);
    int crimeCount = 0;
    for (const auto& crimeId : allCrimes) {
        CrimeReport* crime = svc.findCrime(crimeId);
        if (crime && crime->areaId == areaId) {
            crimeCount++;
        }
    }
    cout << "Crimes in area: " << crimeCount << "\n";
    cout << "===================\n";
}

void updateCrimeInteractive(CrimeService& svc) {
    string crimeId;
    if (!selectCrime(svc, crimeId)) return;
    
    CrimeReport* crime = svc.findCrime(crimeId);
    if (!crime) {
        cout << "Crime not found!\n";
        return;
    }
    
    CrimeReport updated = *crime;
    
    cout << "\nCurrent type: " << crime->type << "\n";
    cout << "Update type? (y/n): ";
    string updateType;
    if (getline(cin, updateType) && (updateType == "y" || updateType == "Y")) {
        if (!selectCrimeType(updated.type)) return;
    }
    
    cout << "\nCurrent severity: " << crime->severity << "\n";
    cout << "New severity (1-5, or press Enter to keep): ";
    string sline;
    if(getline(cin, sline) && !sline.empty()) {
        try {
            int sev = stoi(sline);
            if (sev >= 1 && sev <= 5) updated.severity = sev;
        } catch(...) {}
    }
    
    cout << "\nCurrent notes: " << (crime->notes.empty() ? "None" : crime->notes) << "\n";
    cout << "New notes (or press Enter to keep): ";
    string newNotes;
    getline(cin, newNotes);
    if (!newNotes.empty()) updated.notes = newNotes;
    
    if (svc.updateCrime(crimeId, updated)) {
        cout << "Crime updated successfully!\n";
    } else {
        cout << "Failed to update crime\n";
    }
}

void deleteCrimeInteractive(CrimeService& svc) {
    string crimeId;
    if (!selectCrime(svc, crimeId)) return;
    
    CrimeReport* crime = svc.findCrime(crimeId);
    if (!crime) {
        cout << "Crime not found!\n";
        return;
    }
    
    cout << "\nCrime to delete:\n";
    printCrime(*crime);
    cout << "\nAre you sure you want to delete this crime? (y/n): ";
    string confirm;
    getline(cin, confirm);
    if (confirm == "y" || confirm == "Y") {
        if (svc.deleteCrime(crimeId)) {
            cout << "Crime deleted successfully!\n";
        } else {
            cout << "Failed to delete crime\n";
        }
    } else {
        cout << "Deletion cancelled\n";
    }
}

void updateOfficerInteractive(CrimeService& svc) {
    string officerId;
    if (!selectOfficer(svc, officerId)) return;
    
    Officer* officer = svc.findOfficer(officerId);
    if (!officer) {
        cout << "Officer not found!\n";
        return;
    }
    
    Officer updated = *officer;
    
    cout << "\nCurrent name: " << officer->name << "\n";
    cout << "New name (or press Enter to keep): ";
    string newName;
    getline(cin, newName);
    if (!newName.empty()) updated.name = newName;
    
    cout << "\nCurrent role: " << officer->role << "\n";
    cout << "Update role? (y/n): ";
    string updateRole;
    if (getline(cin, updateRole) && (updateRole == "y" || updateRole == "Y")) {
        if (!selectOfficerRole(updated.role)) return;
    }
    
    cout << "\nCurrent max load: " << officer->maxLoad << "\n";
    cout << "New max load (or press Enter to keep): ";
    string mline;
    if(getline(cin, mline) && !mline.empty()) {
        try {
            int maxLoad = stoi(mline);
            if (maxLoad > 0) updated.maxLoad = maxLoad;
        } catch(...) {}
    }
    
    if (svc.updateOfficer(officerId, updated)) {
        cout << "Officer updated successfully!\n";
    } else {
        cout << "Failed to update officer\n";
    }
}

void deleteOfficerInteractive(CrimeService& svc) {
    string officerId;
    if (!selectOfficer(svc, officerId)) return;
    
    Officer* officer = svc.findOfficer(officerId);
    if (!officer) {
        cout << "Officer not found!\n";
        return;
    }
    
    // Check for assigned crimes
    vector<string> allCrimes = svc.getRecentCrimes(1000);
    int assignedCount = 0;
    for (const auto& crimeId : allCrimes) {
        CrimeReport* crime = svc.findCrime(crimeId);
        if (crime && crime->officerId == officerId) {
            assignedCount++;
        }
    }
    
    cout << "\nOfficer to delete:\n";
    printOfficer(*officer);
    if (assignedCount > 0) {
        cout << "\nWARNING: This officer has " << assignedCount << " assigned crime(s).\n";
    }
    
    cout << "\nAre you sure you want to delete this officer? (y/n): ";
    string confirm;
    getline(cin, confirm);
    if (confirm == "y" || confirm == "Y") {
        if (svc.deleteOfficer(officerId)) {
            cout << "Officer deleted successfully!\n";
        } else {
            cout << "Failed to delete officer\n";
        }
    } else {
        cout << "Deletion cancelled\n";
    }
}

void viewCrimesByStage(CrimeService& svc) {
    CaseStage stage;
    if (!selectCaseStage(stage)) return;
    
    vector<string> allCrimes = svc.getRecentCrimes(1000);
    int count = 0;
    
    cout << "\nCrimes in stage: ";
    switch(stage) {
        case CaseStage::Reported: cout << "Reported\n"; break;
        case CaseStage::Assigned: cout << "Assigned\n"; break;
        case CaseStage::Investigation: cout << "Investigation\n"; break;
        case CaseStage::Closed: cout << "Closed\n"; break;
    }
    cout << "==================\n";
    
    for (const auto& crimeId : allCrimes) {
        CrimeReport* crime = svc.findCrime(crimeId);
        if (crime && crime->stage == stage) {
            count++;
            printCrime(*crime);
        }
    }
    
    if (count == 0) cout << "No crimes found in this stage.\n";
    else cout << "\nTotal: " << count << " crime(s)\n";
}

void findNearbyAreasInteractive(CrimeService& svc) {
    string areaId;
    if (!selectArea(svc, areaId)) return;
    
    cout << "Enter max hops (distance, 1-10): ";
    string hline;
    if(!getline(cin, hline)) { cout << "Invalid\n"; return; }
    int maxHops;
    try { 
        maxHops = stoi(hline); 
        if (maxHops < 1) maxHops = 1;
        if (maxHops > 10) maxHops = 10;
    } catch(...) { 
        cout << "Invalid, using default 2\n"; 
        maxHops = 2; 
    }
    
    vector<string> nearby = svc.getNearbyAreas(areaId, maxHops);
    if (nearby.empty()) {
        cout << "No nearby areas found within " << maxHops << " hops\n";
    } else {
        cout << "\nNearby areas (within " << maxHops << " hops):\n";
        for (const auto& aid : nearby) {
            Area* area = svc.findArea(aid);
            if (area) {
                cout << "  - " << area->id << " | " << area->name << "\n";
            }
        }
    }
}

void showSystemStatistics(CrimeService& svc) {
    cout << "\n=== SYSTEM STATISTICS ===\n";
    cout << "Total Crimes: " << svc.getTotalCrimes() << "\n";
    cout << "Total Officers: " << svc.getTotalOfficers() << "\n";
    
    vector<Area> areas = svc.getAllAreas();
    cout << "Total Areas: " << areas.size() << "\n\n";
    
    // Crimes by stage
    vector<string> allCrimes = svc.getRecentCrimes(1000);
    int reported = 0, assigned = 0, investigation = 0, closed = 0;
    
    for (const auto& crimeId : allCrimes) {
        CrimeReport* crime = svc.findCrime(crimeId);
        if (crime) {
            switch(crime->stage) {
                case CaseStage::Reported: reported++; break;
                case CaseStage::Assigned: assigned++; break;
                case CaseStage::Investigation: investigation++; break;
                case CaseStage::Closed: closed++; break;
            }
        }
    }
    
    cout << "Crimes by Stage:\n";
    cout << "  Reported: " << reported << "\n";
    cout << "  Assigned: " << assigned << "\n";
    cout << "  Investigation: " << investigation << "\n";
    cout << "  Closed: " << closed << "\n\n";
    
    // Officer workload
    vector<Officer> officers = svc.getAllOfficers();
    int overloaded = 0, available = 0;
    for (const auto& o : officers) {
        if (o.curLoad >= o.maxLoad) overloaded++;
        else if (o.curLoad == 0) available++;
    }
    
    cout << "Officer Status:\n";
    cout << "  Available (0 cases): " << available << "\n";
    cout << "  Overloaded (at capacity): " << overloaded << "\n";
    cout << "========================\n";
}

void waitForEnter() {
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

// Menu display functions (ASCII-only)
void showMainMenu() {
    cout << "\n========================================\n";
    cout << "   CRIME RECORD MANAGEMENT SYSTEM (CRMS)\n";
    cout << "========================================\n";
    cout << "MAIN MENU:\n";
    cout << "1. Crime Management\n";
    cout << "2. Officer Management\n";
    cout << "3. Area Management\n";
    cout << "4. Routes & Navigation\n";
    cout << "5. Assignments & Case Management\n";
    cout << "6. Reports & Analytics\n";
    cout << "7. System Information\n";
    cout << "0. Exit\n";
    cout << "\nSelect option: ";
}

void showCrimeMenu() {
    cout << "\n----------------------------------------\n";
    cout << "             CRIME MANAGEMENT\n";
    cout << "----------------------------------------\n";
    cout << "1. Add New Crime\n";
    cout << "2. View Crime Details\n";
    cout << "3. List All Crimes\n";
    cout << "4. Update Crime Information\n";
    cout << "5. Delete Crime\n";
    cout << "6. View Crimes by Stage\n";
    cout << "0. Back to Main Menu\n";
    cout << "\nSelect option: ";
}

void showOfficerMenu() {
    cout << "\n----------------------------------------\n";
    cout << "            OFFICER MANAGEMENT\n";
    cout << "----------------------------------------\n";
    cout << "1. Add New Officer\n";
    cout << "2. View Officer Details\n";
    cout << "3. List All Officers\n";
    cout << "4. Update Officer Information\n";
    cout << "5. Delete Officer\n";
    cout << "0. Back to Main Menu\n";
    cout << "\nSelect option: ";
}

void showAreaMenu() {
    cout << "\n----------------------------------------\n";
    cout << "             AREA MANAGEMENT\n";
    cout << "----------------------------------------\n";
    cout << "1. Add New Area\n";
    cout << "2. View Area Details\n";
    cout << "3. List All Areas\n";
    cout << "0. Back to Main Menu\n";
    cout << "\nSelect option: ";
}

void showRouteMenu() {
    cout << "\n----------------------------------------\n";
    cout << "          ROUTES & NAVIGATION\n";
    cout << "----------------------------------------\n";
    cout << "1. Add Road/Connection\n";
    cout << "2. Find Shortest Route\n";
    cout << "3. Find Nearby Areas\n";
    cout << "0. Back to Main Menu\n";
    cout << "\nSelect option: ";
}

void showAssignmentMenu() {
    cout << "\n----------------------------------------\n";
    cout << "      ASSIGNMENTS & CASE MANAGEMENT\n";
    cout << "----------------------------------------\n";
    cout << "1. Assign Officer to Crime\n";
    cout << "2. Advance Crime Stage\n";
    cout << "3. View Crimes by Stage\n";
    cout << "0. Back to Main Menu\n";
    cout << "\nSelect option: ";
}

void showReportsMenu() {
    cout << "\n----------------------------------------\n";
    cout << "          REPORTS & ANALYTICS\n";
    cout << "----------------------------------------\n";
    cout << "1. Recent Crimes Report\n";
    cout << "2. Crimes in Time Range\n";
    cout << "3. System Statistics\n";
    cout << "0. Back to Main Menu\n";
    cout << "\nSelect option: ";
}

// Menu handler functions
bool handleCrimeMenu(CrimeService& svc) {
    string line;
    int opt;
    
    while(true) {
        showCrimeMenu();
        if(!getline(cin, line)) return false;
        try { opt = stoi(line); } catch(...) { cout << "Invalid input\n"; waitForEnter(); continue; }
        
        switch(opt) {
            case 1: addCrimeInteractive(svc); waitForEnter(); break;
            case 2: viewCrimeDetails(svc); waitForEnter(); break;
            case 3: listCrimes(svc); waitForEnter(); break;
            case 4: updateCrimeInteractive(svc); waitForEnter(); break;
            case 5: deleteCrimeInteractive(svc); waitForEnter(); break;
            case 6: viewCrimesByStage(svc); waitForEnter(); break;
            case 0: return true; // Back to main menu
            default: cout << "Invalid option\n"; waitForEnter(); break;
        }
    }
}

bool handleOfficerMenu(CrimeService& svc) {
    string line;
    int opt;
    
    while(true) {
        showOfficerMenu();
        if(!getline(cin, line)) return false;
        try { opt = stoi(line); } catch(...) { cout << "Invalid input\n"; waitForEnter(); continue; }
        
        switch(opt) {
            case 1: addOfficerInteractive(svc); waitForEnter(); break;
            case 2: viewOfficerDetails(svc); waitForEnter(); break;
            case 3: listOfficers(svc); waitForEnter(); break;
            case 4: updateOfficerInteractive(svc); waitForEnter(); break;
            case 5: deleteOfficerInteractive(svc); waitForEnter(); break;
            case 0: return true; // Back to main menu
            default: cout << "Invalid option\n"; waitForEnter(); break;
        }
    }
}

bool handleAreaMenu(CrimeService& svc) {
    string line;
    int opt;
    
    while(true) {
        showAreaMenu();
        if(!getline(cin, line)) return false;
        try { opt = stoi(line); } catch(...) { cout << "Invalid input\n"; waitForEnter(); continue; }
        
        switch(opt) {
            case 1: addAreaInteractive(svc); waitForEnter(); break;
            case 2: viewAreaDetails(svc); waitForEnter(); break;
            case 3: listAreas(svc); waitForEnter(); break;
            case 0: return true; // Back to main menu
            default: cout << "Invalid option\n"; waitForEnter(); break;
        }
    }
}

bool handleRouteMenu(CrimeService& svc) {
    string line;
    int opt;
    
    while(true) {
        showRouteMenu();
        if(!getline(cin, line)) return false;
        try { opt = stoi(line); } catch(...) { cout << "Invalid input\n"; waitForEnter(); continue; }
        
        switch(opt) {
            case 1: addRoadInteractive(svc); waitForEnter(); break;
            case 2: shortestRouteInteractive(svc); waitForEnter(); break;
            case 3: findNearbyAreasInteractive(svc); waitForEnter(); break;
            case 0: return true; // Back to main menu
            default: cout << "Invalid option\n"; waitForEnter(); break;
        }
    }
}

bool handleAssignmentMenu(CrimeService& svc) {
    string line;
    int opt;
    
    while(true) {
        showAssignmentMenu();
        if(!getline(cin, line)) return false;
        try { opt = stoi(line); } catch(...) { cout << "Invalid input\n"; waitForEnter(); continue; }
        
        switch(opt) {
            case 1: assignOfficerInteractive(svc); waitForEnter(); break;
            case 2: advanceStageInteractive(svc); waitForEnter(); break;
            case 3: viewCrimesByStage(svc); waitForEnter(); break;
            case 0: return true; // Back to main menu
            default: cout << "Invalid option\n"; waitForEnter(); break;
        }
    }
}

bool handleReportsMenu(CrimeService& svc) {
    string line;
    int opt;
    
    while(true) {
        showReportsMenu();
        if(!getline(cin, line)) return false;
        try { opt = stoi(line); } catch(...) { cout << "Invalid input\n"; waitForEnter(); continue; }
        
        switch(opt) {
            case 1: recentCrimesInteractive(svc); waitForEnter(); break;
            case 2: crimesInRangeInteractive(svc); waitForEnter(); break;
            case 3: showSystemStatistics(svc); waitForEnter(); break;
            case 0: return true; // Back to main menu
            default: cout << "Invalid option\n"; waitForEnter(); break;
        }
    }
}

int main(){
    CrimeService svc;
    string line;
    
    cout << "\n";
    cout << "========================================\n";
    cout << " Welcome to Crime Record Management System\n";
    cout << "========================================\n";
    
    while(true){
        showMainMenu();
        
        if(!getline(cin, line)) break;
        int opt;
        try { opt = stoi(line); } catch(...) { 
            cout << "Invalid input. Please enter a number.\n"; 
            waitForEnter();
            continue; 
        }

        switch(opt){
            case 1: 
                if (!handleCrimeMenu(svc)) return 0;
                break;
            case 2: 
                if (!handleOfficerMenu(svc)) return 0;
                break;
            case 3: 
                if (!handleAreaMenu(svc)) return 0;
                break;
            case 4: 
                if (!handleRouteMenu(svc)) return 0;
                break;
            case 5: 
                if (!handleAssignmentMenu(svc)) return 0;
                break;
            case 6: 
                if (!handleReportsMenu(svc)) return 0;
                break;
            case 7: 
                showSystemStatistics(svc);
                waitForEnter();
                break;
            case 0: 
                cout << "\nThank you for using CRMS. Goodbye!\n";
                return 0;
            default: 
                cout << "Invalid option. Please select from the menu.\n";
                waitForEnter();
                break;
        }
    }
    return 0;
}
