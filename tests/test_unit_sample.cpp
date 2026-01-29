//-----------------------------------------------------------------------------
//   test_unit_sample.cpp
//
//   Sample unit tests using Google Test framework with SWMM mock
//   Demonstrates testing lifecycle management with mocked SWMM API
//-----------------------------------------------------------------------------

#include "gtest_minimal.h"
#include "swmm_mock.h"
#include <windows.h>

// Function pointer type for the bridge function
typedef void (*BridgeFunctionType)(int, int*, double*, double*);
typedef void (*SetSubcatchmentIndexType)(int);

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

// Global DLL handle and function pointers
static HMODULE g_hDll = nullptr;
static BridgeFunctionType g_SwmmGoldSimBridge = nullptr;
static SetSubcatchmentIndexType g_SetSubcatchmentIndex = nullptr;

// Test fixture setup/teardown
class BridgeTestFixture {
public:
    static void SetUpTestSuite() {
        // Load the DLL once for all tests
        g_hDll = LoadLibraryA("GSswmm.dll");
        if (g_hDll) {
            g_SwmmGoldSimBridge = (BridgeFunctionType)GetProcAddress(g_hDll, "SwmmGoldSimBridge");
            g_SetSubcatchmentIndex = (SetSubcatchmentIndexType)GetProcAddress(g_hDll, "SetSubcatchmentIndex");
        }
    }
    
    static void TearDownTestSuite() {
        if (g_hDll) {
            FreeLibrary(g_hDll);
            g_hDll = nullptr;
        }
    }
    
    void SetUp() {
        // Reset mock state before each test
        SwmmMock_Reset();
        SwmmMock_SetSuccessMode();
        SwmmMock_SetGetCountReturn(1); // Default to 1 subcatchment
    }
};

//-----------------------------------------------------------------------------
// Version and Arguments Tests
//-----------------------------------------------------------------------------

TEST(VersionTests, ReturnsCorrectVersion) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_REP_VERSION, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(outargs[0], 4.00);
}

TEST(ArgumentsTests, ReturnsCorrectArgumentCounts) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(outargs[0], 2.0);  // 2 inputs
    EXPECT_EQ(outargs[1], 7.0);  // 7 outputs
}

//-----------------------------------------------------------------------------
// Lifecycle Tests
//-----------------------------------------------------------------------------

TEST(LifecycleTests, InitializeCallsSwmmOpenAndStart) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(SwmmMock_GetOpenCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetStartCallCount(), 1);
}

TEST(LifecycleTests, CleanupCallsSwmmEndAndClose) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize first
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    ASSERT_EQ(status, XF_SUCCESS);
    
    // Reset call counts
    SwmmMock_Reset();
    SwmmMock_SetSuccessMode();
    
    // Now cleanup
    g_SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(SwmmMock_GetEndCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetCloseCallCount(), 1);
}

TEST(LifecycleTests, CleanupWhenNotRunningSucceeds) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(SwmmMock_GetEndCallCount(), 0);
    EXPECT_EQ(SwmmMock_GetCloseCallCount(), 0);
}

//-----------------------------------------------------------------------------
// Error Handling Tests
//-----------------------------------------------------------------------------

TEST(ErrorHandlingTests, OpenFailureReturnsError) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    SwmmMock_SetOpenFailure(1, "Mock open error");
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_FAILURE_WITH_MSG);
    EXPECT_EQ(SwmmMock_GetOpenCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetStartCallCount(), 0); // Should not call start after open fails
}

TEST(ErrorHandlingTests, StartFailureCleansUpAndReturnsError) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    SwmmMock_SetStartFailure(1, "Mock start error");
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_FAILURE_WITH_MSG);
    EXPECT_EQ(SwmmMock_GetOpenCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetStartCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetCloseCallCount(), 1); // Should cleanup after start fails
}

//-----------------------------------------------------------------------------
// Calculate Tests
//-----------------------------------------------------------------------------

TEST(CalculateTests, CalculateBeforeInitializeReturnsFailure) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    g_SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_FAILURE);
}

TEST(CalculateTests, CalculateSetsRainfallAndGetsRunoff) {
    BridgeTestFixture fixture;
    fixture.SetUp();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Initialize first
    g_SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    ASSERT_EQ(status, XF_SUCCESS);
    
    // Set up mock to return runoff value
    SwmmMock_SetGetValueReturn(5.5);
    
    // Calculate with rainfall input
    inargs[0] = 2.5; // rainfall intensity
    g_SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
    
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(SwmmMock_GetSetValueCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetLastSetValueValue(), 2.5);
    EXPECT_EQ(SwmmMock_GetStepCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetValueCallCount(), 1);
    EXPECT_EQ(outargs[0], 5.5);
}

//-----------------------------------------------------------------------------
// Main Test Runner
//-----------------------------------------------------------------------------

int main() {
    std::cout << "=== GoldSim-SWMM Bridge Unit Tests ===" << std::endl;
    std::cout << std::endl;
    
    // Set up test suite
    BridgeTestFixture::SetUpTestSuite();
    
    if (!g_SwmmGoldSimBridge) {
        std::cerr << "ERROR: Failed to load GSswmm.dll or get function pointer" << std::endl;
        std::cerr << "Make sure the DLL is built and in the tests directory" << std::endl;
        return 1;
    }
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    // Tear down test suite
    BridgeTestFixture::TearDownTestSuite();
    
    return result;
}
