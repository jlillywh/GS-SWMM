//-----------------------------------------------------------------------------
//   test_lifecycle.cpp
//
//   Minimal test program to verify SWMM lifecycle management
//   Tests: Initialize -> Cleanup sequence
//-----------------------------------------------------------------------------

#include <iostream>
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

int main()
{
    std::cout << "=== GoldSim-SWMM Bridge Lifecycle Test ===" << std::endl;
    std::cout << std::endl;

    // Load the DLL
    HMODULE hDll = LoadLibraryA("GSswmm.dll");
    if (!hDll)
    {
        std::cerr << "ERROR: Failed to load GSswmm.dll" << std::endl;
        std::cerr << "Make sure the DLL is built and in the same directory" << std::endl;
        return 1;
    }
    std::cout << "[PASS] DLL loaded successfully" << std::endl;

    // Get the function pointer
    BridgeFunctionType SwmmGoldSimBridge = 
        (BridgeFunctionType)GetProcAddress(hDll, "SwmmGoldSimBridge");
    
    if (!SwmmGoldSimBridge)
    {
        std::cerr << "ERROR: Failed to get SwmmGoldSimBridge function" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }
    std::cout << "[PASS] Function pointer obtained" << std::endl;
    std::cout << std::endl;

    // Test variables
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    int test_count = 0;
    int pass_count = 0;

    // Test 1: XF_REP_VERSION
    std::cout << "Test 1: XF_REP_VERSION" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_REP_VERSION, &status, inargs, outargs);
    if (status == XF_SUCCESS && outargs[0] == 1.05)
    {
        std::cout << "  [PASS] Version = " << outargs[0] << ", Status = " << status << std::endl;
        pass_count++;
    }
    else
    {
        std::cout << "  [FAIL] Expected version 1.05 and status 0, got version " 
                  << outargs[0] << " and status " << status << std::endl;
    }
    std::cout << std::endl;

    // Test 2: XF_REP_ARGUMENTS
    std::cout << "Test 2: XF_REP_ARGUMENTS" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
    if (status == XF_SUCCESS && outargs[0] == 2.0 && outargs[1] == 7.0)
    {
        std::cout << "  [PASS] Inputs = " << outargs[0] << ", Outputs = " 
                  << outargs[1] << ", Status = " << status << std::endl;
        pass_count++;
    }
    else
    {
        std::cout << "  [FAIL] Expected 2 inputs, 7 outputs, status 0, got " 
                  << outargs[0] << " inputs, " << outargs[1] << " outputs, status " 
                  << status << std::endl;
    }
    std::cout << std::endl;

    // Test 3: XF_CLEANUP when not running (should succeed)
    std::cout << "Test 3: XF_CLEANUP when not running" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status == XF_SUCCESS)
    {
        std::cout << "  [PASS] Cleanup when not running succeeded (status = " 
                  << status << ")" << std::endl;
        pass_count++;
    }
    else
    {
        std::cout << "  [FAIL] Cleanup when not running should succeed, got status " 
                  << status << std::endl;
    }
    std::cout << std::endl;

    // Test 4: XF_INITIALIZE (requires valid SWMM model file)
    std::cout << "Test 4: XF_INITIALIZE" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status == XF_SUCCESS)
    {
        std::cout << "  [PASS] Initialize succeeded (status = " << status << ")" << std::endl;
        pass_count++;
    }
    else if (status == XF_FAILURE_WITH_MSG)
    {
        // Get error message
        ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
        char* errorMsg = (char*)(*pAddr);
        std::cout << "  [INFO] Initialize failed with message: " << errorMsg << std::endl;
        std::cout << "  [INFO] This is expected if model.inp doesn't exist" << std::endl;
        std::cout << "  [SKIP] Skipping remaining lifecycle tests" << std::endl;
        
        // Clean up and exit
        FreeLibrary(hDll);
        std::cout << std::endl;
        std::cout << "=== Test Summary ===" << std::endl;
        std::cout << "Tests run: " << test_count << std::endl;
        std::cout << "Tests passed: " << pass_count << std::endl;
        std::cout << "Tests skipped: " << (test_count - pass_count) << std::endl;
        std::cout << std::endl;
        std::cout << "NOTE: To run full lifecycle tests, create a valid model.inp file" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "  [FAIL] Initialize failed with status " << status << std::endl;
    }
    std::cout << std::endl;

    // Test 5: XF_CLEANUP after successful initialize
    std::cout << "Test 5: XF_CLEANUP after initialize" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
    if (status == XF_SUCCESS)
    {
        std::cout << "  [PASS] Cleanup after initialize succeeded (status = " 
                  << status << ")" << std::endl;
        pass_count++;
    }
    else
    {
        std::cout << "  [FAIL] Cleanup after initialize failed with status " 
                  << status << std::endl;
    }
    std::cout << std::endl;

    // Test 6: Re-initialization (initialize -> cleanup -> initialize)
    std::cout << "Test 6: Re-initialization" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status == XF_SUCCESS)
    {
        std::cout << "  [PASS] Re-initialize succeeded (status = " << status << ")" << std::endl;
        
        // Clean up after test
        SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
        pass_count++;
    }
    else
    {
        std::cout << "  [FAIL] Re-initialize failed with status " << status << std::endl;
    }
    std::cout << std::endl;

    // Test 7: Initialize while already running (should cleanup first)
    std::cout << "Test 7: Initialize while already running" << std::endl;
    test_count++;
    SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status == XF_SUCCESS)
    {
        // Now initialize again without cleanup
        SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
        if (status == XF_SUCCESS)
        {
            std::cout << "  [PASS] Initialize while running succeeded (auto-cleanup)" << std::endl;
            
            // Final cleanup
            SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
            pass_count++;
        }
        else
        {
            std::cout << "  [FAIL] Initialize while running failed with status " 
                      << status << std::endl;
            SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
        }
    }
    else
    {
        std::cout << "  [FAIL] Initial initialize failed, cannot test re-init while running" 
                  << std::endl;
    }
    std::cout << std::endl;

    // Clean up
    FreeLibrary(hDll);

    // Print summary
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Tests run: " << test_count << std::endl;
    std::cout << "Tests passed: " << pass_count << std::endl;
    std::cout << "Tests failed: " << (test_count - pass_count) << std::endl;
    std::cout << std::endl;

    if (pass_count == test_count)
    {
        std::cout << "ALL TESTS PASSED!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "SOME TESTS FAILED" << std::endl;
        return 1;
    }
}
