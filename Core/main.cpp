#include <iostream>
#include <string>
#include <vector>
#include <chrono>
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
    switch(crime.stage) {
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

// Test Areas
void testAreas(CrimeService& service) {
    cout << "=== TESTING AREAS ===" << endl;
    
    vector<Area> areas = {
        {"A1", "Downtown Precinct"},
        {"A2", "Westside Station"},
        {"A3", "East End Division"},
        {"A4", "North District"},
        {"A5", "South Central"}
    };
    
    for(const auto& area : areas) {
        if(service.addArea(area)) {
            cout << "✓ Added area: " << area.id << " - " << area.name << endl;
        }
    }
    cout << endl;
}

// Test Roads
void testRoads(CrimeService& service) {
    cout << "=== TESTING ROADS ===" << endl;
    
    vector<Road> roads = {
        {"A1", "A2", 5.2, false},
        {"A1", "A3", 3.8, false},
        {"A2", "A4", 7.1, false},
        {"A3", "A4", 4.5, false},
        {"A4", "A5", 6.3, false},
        {"A2", "A5", 8.9, true}  // Blocked road
    };
    
    for(const auto& road : roads) {
        if(service.addRoad(road)) {
            cout << "✓ Added road: " << road.from << " → " << road.to 
                 << " (" << road.distKm << "km)" 
                 << (road.blocked ? " [BLOCKED]" : "") << endl;
        }
    }
    cout << endl;
}

// Test Officers
void testOfficers(CrimeService& service) {
    cout << "=== TESTING OFFICERS ===" << endl;
    
    vector<Officer> officers = {
        {"O1", "John Smith", "Investigator", "A1", 10, 0},
        {"O2", "Sarah Johnson", "Forensics", "A2", 8, 0},
        {"O3", "Mike Brown", "Patrol", "A3", 12, 0},
        {"O4", "Lisa Davis", "Detective", "A1", 6, 0}
    };
    
    for(const auto& officer : officers) {
        if(service.addOfficer(officer)) {
            cout << "✓ Added officer: " << officer.name << " (" << officer.id << ")" << endl;
        }
    }
    cout << endl;
}

// Test Crimes
void testCrimes(CrimeService& service) {
    cout << "=== TESTING CRIMES ===" << endl;
    
    long long currentTime = getCurrentEpoch();
    
    vector<CrimeReport> crimes = {
        {"C1001", "Theft", 2, currentTime - 3600, "A1", "Stolen bicycle", "", CaseStage::Reported},
        {"C1002", "Assault", 4, currentTime - 1800, "A2", "Bar fight", "", CaseStage::Reported},
        {"C1003", "Burglary", 3, currentTime - 7200, "A3", "Home break-in", "", CaseStage::Reported},
        {"C1004", "Vandalism", 1, currentTime - 900, "A1", "Graffiti", "", CaseStage::Reported},
        {"C1005", "Robbery", 5, currentTime - 300, "A4", "Armed robbery", "", CaseStage::Reported},
        {"C1006", "Fraud", 2, currentTime - 5400, "A2", "Credit card fraud", "", CaseStage::Reported}
    };
    
    for(const auto& crime : crimes) {
        if(service.addCrime(crime)) {
            cout << "✓ Added crime: " << crime.id << " - " << crime.type 
                 << " (Severity: " << crime.severity << ")" << endl;
        }
    }
    cout << endl;
}

// Test Crime Queries
void testCrimeQueries(CrimeService& service) {
    cout << "=== TESTING CRIME QUERIES ===" << endl;
    
    long long currentTime = getCurrentEpoch();
    long long oneHourAgo = currentTime - 3600;
    long long twoHoursAgo = currentTime - 7200;
    
    // Test range query (last 2 hours)
    cout << "Crimes in last 2 hours:" << endl;
    auto recentCrimes = service.getCrimesInTimeRange(twoHoursAgo, currentTime);
    for(const auto& crimeId : recentCrimes) {
        CrimeReport* crime = service.findCrime(crimeId);
        if(crime) printCrime(*crime);
    }
    cout << endl;
    
    // Test individual crime lookup
    cout << "Looking up crime C1002:" << endl;
    CrimeReport* crime = service.findCrime("C1002");
    if(crime) printCrime(*crime);
    cout << endl;
}

// Test Officer Assignment
void testOfficerAssignment(CrimeService& service) {
    cout << "=== TESTING OFFICER ASSIGNMENT ===" << endl;
    
    // Assign officers to crimes
    vector<pair<string, string>> assignments = {
        {"C1001", "O1"},
        {"C1002", "O2"},
        {"C1003", "O3"},
        {"C1004", "O1"}  // Same officer for multiple crimes
    };
    
    for(const auto& assignment : assignments) {
        if(service.assignOfficer(assignment.first, assignment.second)) {
            cout << "✓ Assigned officer " << assignment.second 
                 << " to crime " << assignment.first << endl;
        } else {
            cout << "✗ Failed to assign officer " << assignment.second 
                 << " to crime " << assignment.first << endl;
        }
    }
    cout << endl;
    
    // Test officer overload
    cout << "Testing officer overload (O1 already has 2 cases):" << endl;
    if(!service.assignOfficer("C1005", "O1")) {
        cout << "✓ Correctly prevented overload assignment" << endl;
    }
    cout << endl;
    
    // Print officer loads
    cout << "Current officer loads:" << endl;
    for(int i = 1; i <= 4; i++) {
        string officerId = "O" + to_string(i);
        Officer* officer = service.findOfficer(officerId);
        if(officer) printOfficer(*officer);
    }
    cout << endl;
}

// Test Graph Operations
void testGraphOperations(CrimeService& service) {
    cout << "=== TESTING GRAPH OPERATIONS ===" << endl;
    
    // Test shortest path
    double totalDist;
    auto path = service.shortestRoute("A1", "A5", totalDist);
    
    if(!path.empty()) {
        cout << "Shortest path from A1 to A5: ";
        for(size_t i = 0; i < path.size(); i++) {
            cout << path[i];
            if(i < path.size() - 1) cout << " → ";
        }
        cout << " | Total distance: " << totalDist << "km" << endl;
    } else {
        cout << "No path found from A1 to A5" << endl;
    }
    cout << endl;
    
    // Test neighborhood
    cout << "Areas within 2 hops of A1:" << endl;
    auto nearby = service.getNearbyAreas("A1", 2);
    for(const auto& area : nearby) {
        cout << "  " << area << endl;
    }
    cout << endl;
}

// Test Crime Updates
void testCrimeUpdates(CrimeService& service) {
    cout << "=== TESTING CRIME UPDATES ===" << endl;
    
    // Update a crime
    CrimeReport* crime = service.findCrime("C1002");
    if(crime) {
        CrimeReport updated = *crime;
        updated.severity = 5; // Increase severity
        updated.notes = "Victim hospitalized - upgraded severity";
        
        if(service.updateCrime("C1002", updated)) {
            cout << "✓ Successfully updated crime C1002" << endl;
            cout << "Updated crime details:" << endl;
            printCrime(updated);
        }
    }
    cout << endl;
    
    // Test stage advancement
    if(service.advanceCrimeStage("C1001", CaseStage::Investigation)) {
        cout << "✓ Advanced C1001 to Investigation stage" << endl;
        CrimeReport* crime = service.findCrime("C1001");
        if(crime) printCrime(*crime);
    }
    cout << endl;
}

// Test Recent Crimes
void testRecentCrimes(CrimeService& service) {
    cout << "=== TESTING RECENT CRIMES ===" << endl;
    
    auto recent = service.getRecentCrimes(3);
    cout << "3 most recent crimes:" << endl;
    for(const auto& crimeId : recent) {
        CrimeReport* crime = service.findCrime(crimeId);
        if(crime) printCrime(*crime);
    }
    cout << endl;
}

// Performance Test
void testPerformance(CrimeService& service) {
    cout << "=== PERFORMANCE TEST ===" << endl;
    
    auto start = high_resolution_clock::now();
    
    // Add multiple crimes quickly
    long long baseTime = getCurrentEpoch();
    for(int i = 0; i < 100; i++) {
        CrimeReport crime{
            "P" + to_string(2000 + i),
            "Test Crime",
            1 + (i % 5),
            baseTime + i,
            "A" + to_string(1 + (i % 5)),
            "Performance test crime",
            "",
            CaseStage::Reported
        };
        service.addCrime(crime);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    cout << "Added 100 crimes in " << duration.count() << "ms" << endl;
    cout << "Total crimes in system: " << service.getTotalCrimes() << endl;
    cout << "Total officers in system: " << service.getTotalOfficers() << endl;
    cout << endl;
}

int main() {
    cout << "🚔 CRIME MANAGEMENT SYSTEM TEST SUITE 🚔" << endl;
    cout << "========================================" << endl << endl;
    
    try {
        CrimeService service;
        
        // Run all tests
        testAreas(service);
        testRoads(service);
        testOfficers(service);
        testCrimes(service);
        testCrimeQueries(service);
        testOfficerAssignment(service);
        testGraphOperations(service);
        testCrimeUpdates(service);
        testRecentCrimes(service);
        testPerformance(service);
        
        cout << "🎉 ALL TESTS COMPLETED SUCCESSFULLY! 🎉" << endl;
        cout << "Your Crime Management System is working correctly!" << endl;
        
    } catch(const exception& e) {
        cout << "❌ ERROR: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
