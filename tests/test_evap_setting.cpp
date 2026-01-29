// Test to demonstrate that swmm_setValue for evaporation doesn't work
#include <stdio.h>
#include "include/swmm5.h"

int main()
{
    printf("Testing SWMM Evaporation Setting\n");
    printf("=================================\n\n");
    
    // Open SWMM model
    int error = swmm_open("tests/model.inp", "tests/model.rpt", "tests/model.out");
    if (error != 0)
    {
        printf("ERROR: Failed to open SWMM model (error %d)\n", error);
        return 1;
    }
    printf("✓ SWMM model opened successfully\n");
    
    // Start simulation
    error = swmm_start(1);
    if (error != 0)
    {
        printf("ERROR: Failed to start SWMM (error %d)\n", error);
        swmm_close();
        return 1;
    }
    printf("✓ SWMM simulation started\n\n");
    
    // Read initial evaporation value
    double initial_evap = swmm_getValue(swmm_SUBCATCH_EVAP, 0);
    printf("Initial evaporation rate: %.6f in/day\n", initial_evap);
    
    // Try to set evaporation to a different value
    double new_evap = 10.0;  // Try to set to 10 in/day
    printf("\nAttempting to set evaporation to %.6f in/day...\n", new_evap);
    swmm_setValue(swmm_SUBCATCH_EVAP, 0, new_evap);
    printf("✓ swmm_setValue() called (no error - it returns void)\n");
    
    // Read evaporation value again
    double after_set_evap = swmm_getValue(swmm_SUBCATCH_EVAP, 0);
    printf("\nEvaporation rate after swmm_setValue: %.6f in/day\n", after_set_evap);
    
    // Check if it changed
    printf("\n");
    if (after_set_evap == new_evap)
    {
        printf("✓ SUCCESS: Evaporation was set to %.6f\n", new_evap);
    }
    else if (after_set_evap == initial_evap)
    {
        printf("✗ FAILED: Evaporation unchanged at %.6f (setValue was ignored)\n", initial_evap);
    }
    else
    {
        printf("? UNEXPECTED: Evaporation is %.6f (neither initial nor new value)\n", after_set_evap);
    }
    
    // Cleanup
    swmm_end();
    swmm_close();
    
    printf("\n=================================\n");
    printf("This demonstrates that swmm_setValue for evaporation is ignored by SWMM.\n");
    printf("The value comes from the [EVAPORATION] section in the .inp file.\n");
    
    return 0;
}
