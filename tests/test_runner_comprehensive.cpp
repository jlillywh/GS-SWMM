/**
 * Comprehensive Test Runner for GoldSim-SWMM Bridge
 * 
 * This test runner uses the SWMM test model generator to create
 * all necessary test files on-the-fly, eliminating the need for
 * pre-existing model files.
 */

#include <windows.h>
#include <iostream>
#include <string>
#include "swmm_test_models.h"

// Function pointer type for the bridge
typedef void (*BridgeFunc)(int, int*, double*, double*);

// Method IDs
#define XF_INITIALIZE 0
#define XF_CALCULATE 1
#define XF_REP_VERSION 2
#define XF_REP_ARGUMENTS 3
#define XF_CLEANUP 99

// Status codes
#define XF_SUCCESS 0
#define XF_FAILURE 1
#define XF_FAILURE_WITH_MSG -1

int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

void PrintTestHeader(const std::string& test_name) {
    std::cout << "\n=== " << test_name << " ===" << std::endl;
}

void PrintTestResult(const std::string& test_name, bool passed, const std::string& message = "") {
    tests_run++;
    if (passed) {
        tests_passed++;
        std::cout << "  [PASS] " << test_name;
        if (!message.empty()) std::cout << ": " << message;
        std::cout << std::endl;
    } else {
        tests_failed++;
        std::cout << "  [FAIL] " << test_name;
        if (!message.empty()) std::cout << ": " << message;
        std::cout << std::endl;
    }
}

void PrintInfo(const std::string& message) {
    std::cout << "  [INFO] " << message << std::endl;
}

bool TestVersionAndArguments(BridgeFunc bridge) {
    PrintTestHeader("Version and Arguments Test");
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Test XF_REP_VERSION
    bridge(XF_REP_VERSION, &status, inargs, outargs);
    bool version_ok = (status == XF_SUCCESS && outargs[0] == 4.0);
    PrintTestResult("XF_REP_VERSION returns 4.0", version_ok);
    
    // Test XF_REP_ARGUMENTS
    bridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    bool args_ok = (status == XF_SUCCESS && outargs[0] == 2.0 && outargs[1] == 7.0);
    PrintTestResult("XF_REP_ARGUMENTS returns (2, 7)", args_ok);
    
    return version_ok && args_ok;
}

bool TestLifecycleWithTreatmentTrain(BridgeFunc bridge) {
    PrintTestHeader("Lifecycle Test with Treatment Train");
    
    // Create test model
    SwmmTestModels::TestFixture fixture("lifecycle_test");
    if (!fixture.CreateTreatmentTrain()) {
        PrintTestResult("Create treatment train model", false, "Failed to create model file");
        return false;
    }
    PrintTestResult("Create treatment train model", true);
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Test cleanup when not running
    bridge(XF_CLEANUP, &status, inargs, outargs);
    PrintTestResult("XF_CLEANUP when not running", status == XF_SUCCESS);
    
    // Test initialize
    bridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS) {
        if (status == XF_FAILURE_WITH_MSG) {
            char* error_msg = (char*)(*(ULONG_PTR*)outargs);
            PrintTestResult("XF_INITIALIZE", false, std::string("Error: ") + error_msg);
        } else {
            PrintTestResult("XF_INITIALIZE", false, "Failed with status " + std::to_string(status));
        }
        return false;
    }
    PrintTestResult("XF_INITIALIZE", true);
    
    // Test calculate
    inargs[0] = 0.5;  // rainfall
    inargs[1] = 0.0;  // evaporation
    bridge(XF_CALCULATE, &status, inargs, outargs);
    bool calc_ok = (status == XF_SUCCESS);
    PrintTestResult("XF_CALCULATE", calc_ok);
    
    if (calc_ok) {
        PrintInfo("Catchment discharge: " + std::to_string(outargs[0]));
        PrintInfo("Bioswale volume: " + std::to_string(outargs[1]));
        PrintInfo("Detention volume: " + std::to_string(outargs[2]));
        PrintInfo("Retention volume: " + std::to_string(outargs[3]));
        PrintInfo("Final discharge: " + std::to_string(outargs[4]));
    }
    
    // Test cleanup
    bridge(XF_CLEANUP, &status, inargs, outargs);
    PrintTestResult("XF_CLEANUP", status == XF_SUCCESS);
    
    return calc_ok;
}

bool TestCalculateBeforeInitialize(BridgeFunc bridge) {
    PrintTestHeader("Calculate Before Initialize Test");
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Ensure clean state
    bridge(XF_CLEANUP, &status, inargs, outargs);
    
    // Try to calculate without initializing
    inargs[0] = 0.5;
    bridge(XF_CALCULATE, &status, inargs, outargs);
    
    bool test_ok = (status == XF_FAILURE);
    PrintTestResult("Calculate before initialize fails correctly", test_ok);
    
    return test_ok;
}

bool TestMissingElements(BridgeFunc bridge) {
    PrintTestHeader("Missing Element Validation Test");
    
    bool all_passed = true;
    std::string elements[] = {"ST1", "ST2", "ST3", "C3", "J2"};
    
    for (const auto& element : elements) {
        SwmmTestModels::TestFixture fixture("missing_" + element);
        if (!fixture.CreateMissingElement(element)) {
            PrintTestResult("Create model missing " + element, false);
            all_passed = false;
            continue;
        }
        
        int status;
        double inargs[10] = {0};
        double outargs[10] = {0};
        
        bridge(XF_INITIALIZE, &status, inargs, outargs);
        
        bool test_ok = (status == XF_FAILURE || status == XF_FAILURE_WITH_MSG);
        PrintTestResult("Initialize fails when missing " + element, test_ok);
        
        if (status == XF_FAILURE_WITH_MSG) {
            char* error_msg = (char*)(*(ULONG_PTR*)outargs);
            PrintInfo("Error message: " + std::string(error_msg));
        }
        
        bridge(XF_CLEANUP, &status, inargs, outargs);
        
        all_passed = all_passed && test_ok;
    }
    
    return all_passed;
}

bool TestSubcatchmentIndexValidation(BridgeFunc bridge) {
    PrintTestHeader("Subcatchment Index Validation Test");
    
    // Create model with 3 subcatchments
    SwmmTestModels::TestFixture fixture("subcatch_test");
    if (!fixture.CreateWithSubcatchments(3)) {
        PrintTestResult("Create model with 3 subcatchments", false);
        return false;
    }
    PrintTestResult("Create model with 3 subcatchments", true);
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Note: Current implementation uses hardcoded index 0
    // This test validates that the model loads correctly
    bridge(XF_INITIALIZE, &status, inargs, outargs);
    
    if (status != XF_SUCCESS) {
        if (status == XF_FAILURE_WITH_MSG) {
            char* error_msg = (char*)(*(ULONG_PTR*)outargs);
            PrintInfo("Initialize failed: " + std::string(error_msg));
        }
        PrintTestResult("Initialize with valid subcatchment", false);
        return false;
    }
    
    PrintTestResult("Initialize with valid subcatchment", true);
    
    bridge(XF_CLEANUP, &status, inargs, outargs);
    
    return true;
}

bool TestMultipleRealizations(BridgeFunc bridge) {
    PrintTestHeader("Multiple Realizations Test");
    
    SwmmTestModels::TestFixture fixture("realization_test");
    if (!fixture.CreateTreatmentTrain()) {
        PrintTestResult("Create treatment train model", false);
        return false;
    }
    
    bool all_passed = true;
    
    for (int i = 0; i < 3; i++) {
        PrintInfo("Realization " + std::to_string(i + 1));
        
        int status;
        double inargs[10] = {0};
        double outargs[10] = {0};
        
        // Initialize
        bridge(XF_INITIALIZE, &status, inargs, outargs);
        if (status != XF_SUCCESS) {
            PrintTestResult("Initialize realization " + std::to_string(i + 1), false);
            all_passed = false;
            continue;
        }
        
        // Run a few steps
        for (int step = 0; step < 5; step++) {
            inargs[0] = 0.5 + (i * 0.1);  // Different rainfall per realization
            inargs[1] = 0.0;
            bridge(XF_CALCULATE, &status, inargs, outargs);
            if (status != XF_SUCCESS && status != 0) {
                PrintTestResult("Calculate step " + std::to_string(step), false);
                all_passed = false;
                break;
            }
        }
        
        // Cleanup
        bridge(XF_CLEANUP, &status, inargs, outargs);
        if (status != XF_SUCCESS) {
            PrintTestResult("Cleanup realization " + std::to_string(i + 1), false);
            all_passed = false;
        }
    }
    
    PrintTestResult("Multiple realizations", all_passed);
    return all_passed;
}

bool TestErrorHandling(BridgeFunc bridge) {
    PrintTestHeader("Error Handling Test");
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Ensure clean state
    bridge(XF_CLEANUP, &status, inargs, outargs);
    
    // Try to initialize with non-existent file
    bridge(XF_INITIALIZE, &status, inargs, outargs);
    
    bool test_ok = (status == XF_FAILURE || status == XF_FAILURE_WITH_MSG);
    PrintTestResult("Initialize with missing file fails", test_ok);
    
    if (status == XF_FAILURE_WITH_MSG) {
        char* error_msg = (char*)(*(ULONG_PTR*)outargs);
        PrintInfo("Error message: " + std::string(error_msg));
        PrintTestResult("Error message provided", true);
    }
    
    return test_ok;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Comprehensive GoldSim-SWMM Bridge Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Load DLL
    HMODULE dll = LoadLibrary("GSswmm.dll");
    if (!dll) {
        std::cout << "[FAIL] Could not load GSswmm.dll" << std::endl;
        return 1;
    }
    std::cout << "[PASS] DLL loaded successfully" << std::endl;
    
    // Get function pointer
    BridgeFunc bridge = (BridgeFunc)GetProcAddress(dll, "SwmmGoldSimBridge");
    if (!bridge) {
        std::cout << "[FAIL] Could not find SwmmGoldSimBridge function" << std::endl;
        FreeLibrary(dll);
        return 1;
    }
    std::cout << "[PASS] Bridge function found" << std::endl;
    
    // Run all tests
    TestVersionAndArguments(bridge);
    TestCalculateBeforeInitialize(bridge);
    TestErrorHandling(bridge);
    TestLifecycleWithTreatmentTrain(bridge);
    TestMissingElements(bridge);
    TestSubcatchmentIndexValidation(bridge);
    TestMultipleRealizations(bridge);
    
    // Print summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Tests run: " << tests_run << std::endl;
    std::cout << "Tests passed: " << tests_passed << std::endl;
    std::cout << "Tests failed: " << tests_failed << std::endl;
    std::cout << std::endl;
    
    FreeLibrary(dll);
    
    if (tests_failed == 0) {
        std::cout << "ALL TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "SOME TESTS FAILED!" << std::endl;
        return 1;
    }
}
