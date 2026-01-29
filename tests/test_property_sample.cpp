//-----------------------------------------------------------------------------
//   test_property_sample.cpp
//
//   Sample property-based tests using RapidCheck framework
//   Demonstrates property testing with SWMM mock
//-----------------------------------------------------------------------------

#include "rapidcheck_minimal.h"
#include "swmm_mock.h"
#include <windows.h>

// Function pointer type for the bridge function
typedef void (*BridgeFunctionType)(int, int*, double*, double*);

// GoldSim Method IDs
#define XF_INITIALIZE       0
#define XF_CALCULATE        1
#define XF_REP_VERSION      2
#define XF_REP_ARGUMENTS    3
#define XF_CLEANUP          99

// GoldSim Status Codes
#define XF_SUCCESS              0
#define XF_FAILURE              1
#define XF_FAILURE_WITH_MSG    -1

// Global DLL handle and function pointer
static HMODULE g_hDll = nullptr;
static BridgeFunctionType g_SwmmGoldSimBridge = nullptr;

// Setup function
void SetupPropertyTests() {
    g_hDll = LoadLibraryA("GSswmm.dll");
    if (g_hDll) {
        g_SwmmGoldSimBridge = (BridgeFunctionType)GetProcAddress(g_hDll, "SwmmGoldSimBridge");
    }
}

// Teardown function
void TeardownPropertyTests() {
    if (g_hDll) {
        FreeLibrary(g_hDll);
        g_hDll = nullptr;
    }
}

//-----------------------------------------------------------------------------
// Property 1: Successful method calls return XF_SUCCESS
// Feature: goldsim-swmm-bridge
// Validates: Requirements 1.4
//-----------------------------------------------------------------------------

RC_GTEST_PROP(BridgeProperties, SuccessfulCallsReturnSuccess,
              "goldsim-swmm-bridge",
              "Property 1: Successful method calls return XF_SUCCESS")
{
    // Reset mock to success mode
    SwmmMock_Reset();
    SwmmMock_SetSuccessMode();
    SwmmMock_SetGetCountReturn(1);
    
    // Generate random valid method ID
    auto methodIDGen = rc::gen::element(XF_REP_VERSION, XF_REP_ARGUMENTS, XF_INITIALIZE, XF_CLEANUP);
    int methodID = methodIDGen.generate();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(methodID, &status, inargs, outargs);
    
    // Verify status is XF_SUCCESS (0)
    RC_ASSERT(status == XF_SUCCESS);
    
    return true;
}

//-----------------------------------------------------------------------------
// Property 2: Failed method calls return appropriate error codes
// Feature: goldsim-swmm-bridge
// Validates: Requirements 1.5
//-----------------------------------------------------------------------------

RC_GTEST_PROP(BridgeProperties, FailedCallsReturnErrorCodes,
              "goldsim-swmm-bridge",
              "Property 2: Failed method calls return appropriate error codes")
{
    // Reset mock and configure for failure
    SwmmMock_Reset();
    SwmmMock_SetOpenFailure(1, "Mock error");
    SwmmMock_SetGetCountReturn(1);
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Try to initialize (will fail due to mock)
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    
    // Verify status is error code (>0 and <99, or -1 for message)
    RC_ASSERT(status == XF_FAILURE_WITH_MSG || (status > 0 && status < 99));
    
    return true;
}

//-----------------------------------------------------------------------------
// Property 3: SWMM API errors propagate to status
// Feature: goldsim-swmm-bridge
// Validates: Requirements 8.1
//-----------------------------------------------------------------------------

RC_GTEST_PROP(BridgeProperties, SwmmErrorsPropagateToStatus,
              "goldsim-swmm-bridge",
              "Property 3: SWMM API errors propagate to status")
{
    // Reset mock
    SwmmMock_Reset();
    SwmmMock_SetGetCountReturn(1);
    
    // Generate random error code (1-10)
    auto errorCodeGen = rc::gen::inRange(1, 10);
    int errorCode = errorCodeGen.generate();
    
    // Pick a random SWMM function to fail
    auto funcGen = rc::gen::element(0, 1, 2); // 0=open, 1=start, 2=step
    int funcToFail = funcGen.generate();
    
    if (funcToFail == 0) {
        SwmmMock_SetOpenFailure(errorCode, "Mock error");
    } else if (funcToFail == 1) {
        SwmmMock_SetStartFailure(errorCode, "Mock error");
    } else {
        SwmmMock_SetStepFailure(errorCode, "Mock error");
    }
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Try to initialize
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    
    // Verify status indicates failure
    RC_ASSERT(status != XF_SUCCESS);
    
    return true;
}

//-----------------------------------------------------------------------------
// Property 4: Subcatchment index consistency
// Feature: goldsim-swmm-bridge
// Validates: Requirements 4.3, 5.3, 7.2
//-----------------------------------------------------------------------------

RC_GTEST_PROP(BridgeProperties, SubcatchmentIndexConsistency,
              "goldsim-swmm-bridge",
              "Property 4: Subcatchment index consistency")
{
    // Reset mock
    SwmmMock_Reset();
    SwmmMock_SetSuccessMode();
    SwmmMock_SetGetCountReturn(5); // 5 subcatchments
    SwmmMock_SetGetValueReturn(3.5);
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    RC_ASSERT(status == XF_SUCCESS);
    
    // Calculate (will call setValue and getValue)
    inargs[0] = 2.0;
    g_SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
    RC_ASSERT(status == XF_SUCCESS);
    
    // Verify same index used for both setValue and getValue
    int setValueIndex = SwmmMock_GetLastSetValueIndex();
    int getValueIndex = SwmmMock_GetLastGetValueIndex();
    RC_ASSERT(setValueIndex == getValueIndex);
    
    return true;
}

//-----------------------------------------------------------------------------
// Property 7: State invariant maintenance
// Feature: goldsim-swmm-bridge
// Validates: Requirements 9.1
//-----------------------------------------------------------------------------

RC_GTEST_PROP(BridgeProperties, StateInvariantMaintenance,
              "goldsim-swmm-bridge",
              "Property 7: State invariant maintenance")
{
    // Reset mock
    SwmmMock_Reset();
    SwmmMock_SetSuccessMode();
    SwmmMock_SetGetCountReturn(1);
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Generate random sequence of method calls
    auto seqLenGen = rc::gen::inRange(1, 5);
    int seqLen = seqLenGen.generate();
    
    for (int i = 0; i < seqLen; i++) {
        auto methodGen = rc::gen::element(XF_INITIALIZE, XF_CLEANUP);
        int methodID = methodGen.generate();
        
        g_SwmmGoldSimBridge(methodID, &status, inargs, outargs);
        
        // After initialize, SWMM should be running (open and start called)
        if (methodID == XF_INITIALIZE && status == XF_SUCCESS) {
            RC_ASSERT(SwmmMock_GetOpenCallCount() > 0);
            RC_ASSERT(SwmmMock_GetStartCallCount() > 0);
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------
// Main Test Runner
//-----------------------------------------------------------------------------

int main() {
    std::cout << "=== GoldSim-SWMM Bridge Property-Based Tests ===" << std::endl;
    std::cout << std::endl;
    
    // Set up
    SetupPropertyTests();
    
    if (!g_SwmmGoldSimBridge) {
        std::cerr << "ERROR: Failed to load GSswmm.dll or get function pointer" << std::endl;
        std::cerr << "Make sure the DLL is built and in the tests directory" << std::endl;
        return 1;
    }
    
    // Run all property tests
    int result = RUN_ALL_PROPERTY_TESTS();
    
    // Tear down
    TeardownPropertyTests();
    
    return result;
}
