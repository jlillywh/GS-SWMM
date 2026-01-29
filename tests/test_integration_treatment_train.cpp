//-----------------------------------------------------------------------------
//   test_integration_treatment_train.cpp
//
//   Integration test for GoldSim-SWMM Bridge with real SWMM engine
//   Tests full lifecycle with treatment train model (S1, ST1, ST2, ST3, J2, C3)
//-----------------------------------------------------------------------------

#include <iostream>
#include <cmath>
#include <windows.h>

// Function pointer type for the bridge DLL function
typedef void (*BridgeFunctionType)(int, int*, double*, double*);

// GoldSim method IDs
#define XF_INITIALIZE       0
#define XF_CALCULATE        1
#define XF_REP_VERSION      2
#define XF_REP_ARGUMENTS    3
#define XF_CLEANUP          99

// GoldSim status codes
#define XF_SUCCESS              0
#define XF_FAILURE              1
#define XF_FAILURE_WITH_MSG    -1

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

// Helper function to check test results
void CHECK(bool condition, const char* test_name)
{
    if (condition)
    {
        std::cout << "[PASS] " << test_name << std::endl;
        tests_passed++;
    }
    else
    {
        std::cout << "[FAIL] " << test_name << std::endl;
        tests_failed++;
    }
}

// Helper function to check approximate equality for floating point
bool APPROX_EQUAL(double a, double b, double tolerance = 0.001)
{
    return std::abs(a - b) < tolerance;
}

int main()
{
    std::cout << "=== Integration Test: Treatment Train Model ===" << std::endl;
    std::cout << std::endl;
    
    // Load the bridge DLL
    HMODULE hDll = LoadLibraryA("GSswmm.dll");
    if (!hDll)
    {
        std::cout << "[ERROR] Failed to load GSswmm.dll" << std::endl;
        return 1;
    }
    
    // Get the bridge function
    BridgeFunctionType SwmmGoldSimBridge = 
        (BridgeFunctionType)GetProcAddress(hDll, "SwmmGoldSimBridge");
    
    if (!SwmmGoldSimBridge)
    {
        std::cout << "[ERROR] Failed to find SwmmGoldSimBridge function" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }
    
    std::cout << "Bridge DLL loaded successfully" << std::endl;
    std::cout << std::endl;
    
    // Test 1: Report Version
    std::cout << "Test 1: Report Version" << std::endl;
    {
        int status;
        double inargs[1] = {0};
        double outargs[5] = {0};
        
        SwmmGoldSimBridge(XF_REP_VERSION, &status, inargs, outargs);
        
        CHECK(status == XF_SUCCESS, "Version report returns success");
        CHECK(APPROX_EQUAL(outargs[0], 3.0), "Version is 3.0 (treatment train)");
    }
    std::cout << std::endl;
    
    // Test 2: Report Arguments
    std::cout << "Test 2: Report Arguments" << std::endl;
    {
        int status;
        double inargs[2] = {0};
        double outargs[5] = {0};
        
        SwmmGoldSimBridge(XF_REP_ARGUMENTS, &status, inargs, outargs);
        
        CHECK(status == XF_SUCCESS, "Arguments report returns success");
        CHECK(APPROX_EQUAL(outargs[0], 2.0), "2 input arguments (ETime, rainfall)");
        CHECK(APPROX_EQUAL(outargs[1], 5.0), "5 output arguments (catchment + 3 volumes + final)");
    }
    std::cout << std::endl;
    
    // Test 3: Initialize with treatment train model
    std::cout << "Test 3: Initialize with Treatment Train Model" << std::endl;
    {
        int status;
        double inargs[1] = {0};
        double outargs[5] = {0};
        
        SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
        
        if (status != XF_SUCCESS)
        {
            if (status == XF_FAILURE_WITH_MSG)
            {
                char* error_msg = (char*)(*(ULONG_PTR*)outargs);
                std::cout << "Error message: " << error_msg << std::endl;
            }
        }
        
        CHECK(status == XF_SUCCESS, "Initialize succeeds with treatment train model");
    }
    std::cout << std::endl;
    
    // Test 4: Calculate with various rainfall inputs
    std::cout << "Test 4: Calculate with Rainfall Inputs" << std::endl;
    {
        // Run several time steps with different rainfall values
        double rainfall_values[] = {0.0, 0.5, 1.0, 1.5, 1.0, 0.5, 0.0};
        int num_steps = sizeof(rainfall_values) / sizeof(rainfall_values[0]);
        
        bool all_steps_succeeded = true;
        bool volumes_increase = false;
        bool discharge_occurs = false;
        
        double prev_bioswale_vol = 0.0;
        double prev_detention_vol = 0.0;
        double prev_retention_vol = 0.0;
        
        for (int i = 0; i < num_steps; i++)
        {
            int status;
            double inargs[2];
            double outargs[5] = {0};
            
            // inargs[0] = ETime in seconds (1 minute time steps)
            // inargs[1] = rainfall intensity
            inargs[0] = i * 60.0;  // 60 seconds per step
            inargs[1] = rainfall_values[i];
            
            SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
            
            if (status != XF_SUCCESS)
            {
                all_steps_succeeded = false;
                std::cout << "  Step " << i << " failed with status " << status << std::endl;
                break;
            }
            
            double catchment_discharge = outargs[0];
            double bioswale_volume = outargs[1];
            double detention_volume = outargs[2];
            double retention_volume = outargs[3];
            double final_discharge = outargs[4];
            
            std::cout << "  Step " << i << ": ETime=" << inargs[0] 
                      << "s, Rain=" << rainfall_values[i] 
                      << " in/hr, Catchment=" << catchment_discharge 
                      << " CFS, Bioswale=" << bioswale_volume 
                      << " cu.ft, Detention=" << detention_volume 
                      << " cu.ft, Retention=" << retention_volume 
                      << " cu.ft, Final=" << final_discharge << " CFS" << std::endl;
            
            // Check if volumes increase during rainfall
            if (rainfall_values[i] > 0.0)
            {
                if (bioswale_volume > prev_bioswale_vol || 
                    detention_volume > prev_detention_vol ||
                    retention_volume > prev_retention_vol)
                {
                    volumes_increase = true;
                }
            }
            
            // Check if discharge occurs
            if (catchment_discharge > 0.01 || final_discharge > 0.01)
            {
                discharge_occurs = true;
            }
            
            prev_bioswale_vol = bioswale_volume;
            prev_detention_vol = detention_volume;
            prev_retention_vol = retention_volume;
        }
        
        CHECK(all_steps_succeeded, "All calculate steps succeed");
        CHECK(volumes_increase, "Storage volumes increase during rainfall");
        CHECK(discharge_occurs, "Discharge occurs from catchment or final outlet");
    }
    std::cout << std::endl;
    
    // Test 5: Cleanup
    std::cout << "Test 5: Cleanup" << std::endl;
    {
        int status;
        double inargs[1] = {0};
        double outargs[5] = {0};
        
        SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
        
        CHECK(status == XF_SUCCESS, "Cleanup succeeds");
    }
    std::cout << std::endl;
    
    // Test 6: Multiple Realization Support
    std::cout << "Test 6: Multiple Realization Support" << std::endl;
    {
        bool all_realizations_succeeded = true;
        
        for (int realization = 0; realization < 3; realization++)
        {
            std::cout << "  Realization " << realization << ":" << std::endl;
            
            // Initialize
            int status;
            double inargs[2] = {0};
            double outargs[5] = {0};
            
            SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
            if (status != XF_SUCCESS)
            {
                std::cout << "    Initialize failed" << std::endl;
                all_realizations_succeeded = false;
                break;
            }
            
            // Run a few steps
            for (int step = 0; step < 3; step++)
            {
                inargs[0] = step * 60.0;  // ETime in seconds
                inargs[1] = 1.0;  // 1 in/hr rainfall
                SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
                
                if (status != XF_SUCCESS)
                {
                    std::cout << "    Calculate step " << step << " failed" << std::endl;
                    all_realizations_succeeded = false;
                    break;
                }
            }
            
            // Cleanup
            SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
            if (status != XF_SUCCESS)
            {
                std::cout << "    Cleanup failed" << std::endl;
                all_realizations_succeeded = false;
                break;
            }
            
            std::cout << "    Realization " << realization << " completed successfully" << std::endl;
        }
        
        CHECK(all_realizations_succeeded, "Multiple realizations complete successfully");
    }
    std::cout << std::endl;
    
    // Test 7: Treatment Train Element Validation
    std::cout << "Test 7: Treatment Train Element Validation" << std::endl;
    {
        // Initialize again to test element identification
        int status;
        double inargs[2] = {0};
        double outargs[5] = {0};
        
        SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
        CHECK(status == XF_SUCCESS, "Initialize finds all treatment train elements");
        
        // Run one step to verify all outputs are accessible
        inargs[0] = 0.0;  // ETime = 0 seconds
        inargs[1] = 1.0;  // 1 in/hr rainfall
        SwmmGoldSimBridge(XF_CALCULATE, &status, inargs, outargs);
        
        bool all_outputs_valid = true;
        if (status == XF_SUCCESS)
        {
            // Check that all outputs are non-negative (valid values)
            for (int i = 0; i < 5; i++)
            {
                if (outargs[i] < 0.0)
                {
                    all_outputs_valid = false;
                    std::cout << "  Output " << i << " is negative: " << outargs[i] << std::endl;
                }
            }
        }
        else
        {
            all_outputs_valid = false;
        }
        
        CHECK(all_outputs_valid, "All 5 hydraulic outputs are valid (non-negative)");
        
        // Cleanup
        SwmmGoldSimBridge(XF_CLEANUP, &status, inargs, outargs);
    }
    std::cout << std::endl;
    
    // Cleanup and unload DLL
    FreeLibrary(hDll);
    
    // Print summary
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Tests Passed: " << tests_passed << std::endl;
    std::cout << "Tests Failed: " << tests_failed << std::endl;
    std::cout << std::endl;
    
    if (tests_failed == 0)
    {
        std::cout << "ALL TESTS PASSED!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "SOME TESTS FAILED!" << std::endl;
        return 1;
    }
}
