// Test to determine which SWMM properties are writable via swmm_setValue
#include <stdio.h>
#include "include/swmm5.h"

void test_property(const char* name, int property, int index, double test_value)
{
    // Read initial value
    double initial = swmm_getValue(property, index);
    
    // Try to set new value
    swmm_setValue(property, index, test_value);
    
    // Read value again
    double after = swmm_getValue(property, index);
    
    // Check if it changed
    bool changed = (after != initial);
    bool matches_test = (after == test_value);
    
    printf("%-30s | Initial: %10.4f | After: %10.4f | %s\n", 
           name, initial, after,
           matches_test ? "✓ WRITABLE" : (changed ? "? CHANGED" : "✗ READ-ONLY"));
}

int main()
{
    printf("SWMM API Writable Properties Test\n");
    printf("==================================\n\n");
    
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
    
    printf("Testing GAGE Properties:\n");
    printf("%-30s | %-12s | %-12s | Status\n", "Property", "Initial", "After Set", "");
    printf("--------------------------------------------------------------------------------\n");
    test_property("GAGE_RAINFALL", swmm_GAGE_RAINFALL, 0, 5.0);
    
    printf("\nTesting SUBCATCHMENT Properties:\n");
    printf("%-30s | %-12s | %-12s | Status\n", "Property", "Initial", "After Set", "");
    printf("--------------------------------------------------------------------------------\n");
    test_property("SUBCATCH_AREA", swmm_SUBCATCH_AREA, 0, 20.0);
    test_property("SUBCATCH_RAINGAGE", swmm_SUBCATCH_RAINGAGE, 0, 1.0);
    test_property("SUBCATCH_RAINFALL", swmm_SUBCATCH_RAINFALL, 0, 3.0);
    test_property("SUBCATCH_EVAP", swmm_SUBCATCH_EVAP, 0, 0.5);
    test_property("SUBCATCH_INFIL", swmm_SUBCATCH_INFIL, 0, 2.0);
    test_property("SUBCATCH_RUNOFF", swmm_SUBCATCH_RUNOFF, 0, 10.0);
    
    printf("\nTesting NODE Properties:\n");
    printf("%-30s | %-12s | %-12s | Status\n", "Property", "Initial", "After Set", "");
    printf("--------------------------------------------------------------------------------\n");
    test_property("NODE_ELEV", swmm_NODE_ELEV, 0, 100.0);
    test_property("NODE_MAXDEPTH", swmm_NODE_MAXDEPTH, 0, 20.0);
    test_property("NODE_DEPTH", swmm_NODE_DEPTH, 0, 5.0);
    test_property("NODE_HEAD", swmm_NODE_HEAD, 0, 105.0);
    test_property("NODE_VOLUME", swmm_NODE_VOLUME, 0, 1000.0);
    test_property("NODE_LATFLOW", swmm_NODE_LATFLOW, 0, 2.0);
    test_property("NODE_INFLOW", swmm_NODE_INFLOW, 0, 3.0);
    
    printf("\nTesting LINK Properties:\n");
    printf("%-30s | %-12s | %-12s | Status\n", "Property", "Initial", "After Set", "");
    printf("--------------------------------------------------------------------------------\n");
    // Note: Your model might not have links, so these might fail
    int link_count = swmm_getCount(swmm_LINK);
    if (link_count > 0)
    {
        test_property("LINK_SETTING", swmm_LINK_SETTING, 0, 0.5);
        test_property("LINK_FLOW", swmm_LINK_FLOW, 0, 10.0);
        test_property("LINK_DEPTH", swmm_LINK_DEPTH, 0, 2.0);
    }
    else
    {
        printf("(No links in model - skipping link tests)\n");
    }
    
    printf("\n==================================\n");
    printf("Summary:\n");
    printf("✓ WRITABLE   - Property can be set dynamically\n");
    printf("✗ READ-ONLY  - Property cannot be changed via API\n");
    printf("? CHANGED    - Value changed but not to test value (unusual)\n");
    
    // Cleanup
    swmm_end();
    swmm_close();
    
    return 0;
}
