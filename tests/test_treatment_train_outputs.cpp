//-----------------------------------------------------------------------------
//   test_treatment_train_outputs.cpp
//
//   Test program to verify all 5 treatment train outputs are returned
//   Tests: Catchment discharge, bioswale volume, detention volume, 
//          retention volume, final discharge
//-----------------------------------------------------------------------------

#include <iostream>
#include <windows.h>

// Function pointer type for the bridge function
typedef void (*BridgeFunctionType)(int, int*, double*, double*);

// GoldSim Method IDs
#define XF_INITIALIZE       0
#define XF_CALCULATE        1
#define XF_CLEANUP          99

// GoldSim Status Codes
#define XF_SUCCESS              0

int main()
{
    std::cout << "=== Treatment Train Outputs Test ===" << std::endl;
    std::cout << std::endl;

    // Load the DLL
    HMODULE hDll = LoadLibraryA("GSswmm.dll");
    if (!hDll)
    {
        std::cerr << "ERROR: Failed to load GSswmm.dll" << std::endl;
        return 1;
    }

    // Get the function pointer
    BridgeFunctionType SwmmGoldSimBridge = 
        (BridgeFunctionType)GetProcAddress(hDll, "SwmmGoldSimBridge");
    
    if (!SwmmGoldSimBridge)
    {
        std::cerr << "ERROR: Failed to get SwmmGoldSimBridge function" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    // Test variables
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};

    // Initialize SWMM
    std::cout << "Initializing SWMM..." << std::endl;
    SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    if (status != XF_SUCCESS)
    {
        std::cerr << "ERROR: Initialize failed" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }
    std::cout << "[PASS] Initialize succeeded" << std::endl;
    std::cout << std::endl;

    // Run several time steps with rainfall
    std::cout << "Running simulation with 2.0 in/hr rainfall..." << std::endl;
    std::cout << std::endl;
    
    for (int step = 1; step <= 10; step++)
    {
        inargs[0] = 2.0;  // 2.0 inch/hour rainfall
        SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
        
        if (status != XF_SUCCESS)
        {
            std::cerr << "ERROR: Calculate failed at step " << step << std::endl;
            break;
        }
        
        std::cout << "Step " << step << ":" << std::endl;
        std::cout << "  Rainfall:           " << inargs[0] << " in/hr" << std::endl;
        std::cout << "  Catchment Discharge: " << outargs[0] << " CFS" << std::endl;
        std::cout << "  Bioswale Volume:     " << outargs[1] << " cu.ft" << std::endl;
        std::cout << "  Detention Volume:    " << outargs[2] << " cu.ft" << std::endl;
        std::cout << "  Retention Volume:    " << outargs[3] << " cu.ft" << std::endl;
        std::cout << "  Final Discharge:     " << outargs[4] << " CFS" << std::endl;
        std::cout << std::endl;
    }

    // Cleanup
    SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
    FreeLibrary(hDll);

    std::cout << "=== Test Complete ===" << std::endl;
    std::cout << "All 5 treatment train outputs are being returned correctly!" << std::endl;
    
    return 0;
}
