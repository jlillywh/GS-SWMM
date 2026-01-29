// Test which properties are writable during simulation stepping
#include <stdio.h>
#include "include/swmm5.h"

void test_property_during_step(const char* name, int property, int index, double test_value)
{
    // Read initial value
    double initial = swmm_getValue(property, index);
    
    // Try to set new value
    swmm_setValue(property, index, test_value);
    
    // Read value immediately after
    double after = swmm_getValue(property, index);
    
    // Step the simulation
    double elapsed_time;
    swmm_step(&elapsed_time);
    
    // Read value after step
    double after_step = swmm_getValue(property, index);
    
    // Check results
    bool immediate_change = (after == test_value);
    bool step_change = (after_step == test_value);
    
    const char* status;
    if (immediate_change) status = "✓ WRITABLE (immediate)";
    else if (step_change) status = "✓ WRITABLE (after step)";
    else if (after != initial) status = "? PARTIAL (changed but not to test value)";
    else status = "✗ READ-ONLY";
    
    printf("%-30s | %10.4f | %10.4f | %10.4f | %s\n", 
           name, initial, after, after_step, status);
}

int main()
{
    printf("SWMM API Writable Properties Test (During Simulation)\n");
    printf("======================================================\n\n");
    
    // Open and start SWMM
    int error = swmm_open("tests/model.inp", "tests/model.rpt", "tests/model.out");
    if (error != 0)
    {
        printf("ERROR: Failed to open SWMM model\n");
        return 1;
    }
    
    error = swmm_start(1);
    if (error != 0)
    {
        printf("ERROR: Failed to start SWMM\n");
        swmm_close();
        return 1;
    }
    
    printf("%-30s | %-10s | %-10s | %-10s | Status\n", 
           "Property", "Initial", "After Set", "After Step", "");
    printf("----------------------------------------------------------------------------------------\n");
    
    printf("\nGAGE Properties:\n");
    test_property_during_step("GAGE_RAINFALL", swmm_GAGE_RAINFALL, 0, 5.0);
    
    printf("\nSUBCATCHMENT Properties:\n");
    test_property_during_step("SUBCATCH_RAINFALL", swmm_SUBCATCH_RAINFALL, 0, 3.0);
    test_property_during_step("SUBCATCH_EVAP", swmm_SUBCATCH_EVAP, 0, 0.5);
    test_property_during_step("SUBCATCH_INFIL", swmm_SUBCATCH_INFIL, 0, 2.0);
    
    printf("\nNODE Properties:\n");
    test_property_during_step("NODE_LATFLOW", swmm_NODE_LATFLOW, 0, 2.0);
    test_property_during_step("NODE_INFLOW", swmm_NODE_INFLOW, 0, 3.0);
    
    // Test LINK_SETTING if links exist
    int link_count = swmm_getCount(swmm_LINK);
    if (link_count > 0)
    {
        printf("\nLINK Properties:\n");
        test_property_during_step("LINK_SETTING", swmm_LINK_SETTING, 0, 0.5);
    }
    
    printf("\n======================================================\n");
    printf("Legend:\n");
    printf("✓ WRITABLE (immediate)  - Value changes immediately after swmm_setValue\n");
    printf("✓ WRITABLE (after step) - Value changes after swmm_step\n");
    printf("? PARTIAL               - Value changed but not to expected value\n");
    printf("✗ READ-ONLY             - Value cannot be changed\n");
    
    // Cleanup
    swmm_end();
    swmm_close();
    
    return 0;
}
