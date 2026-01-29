# Integration Test for Treatment Train Model

## Overview

This integration test validates the GoldSim-SWMM Bridge DLL with a real SWMM engine using a complete treatment train model.

## Test Components

### 1. Treatment Train SWMM Model (`treatment_train_model.inp`)

The model includes all required elements for the treatment train:
- **S1**: Subcatchment that receives rainfall and generates runoff
- **ST1**: Bioswale storage node (first treatment stage)
- **ST2**: Detention storage node (second treatment stage)
- **ST3**: Retention storage node (third treatment stage)
- **J2**: Outfall node (final discharge point)
- **C1**: Conduit from ST1 to ST2
- **C2**: Conduit from ST2 to ST3
- **C3**: Conduit from ST3 to J2 (final discharge)

The model uses:
- Dynamic wave routing for accurate hydraulic calculations
- Functional storage curves for the treatment nodes
- A 2-hour simulation period with varying rainfall

### 2. Integration Test Program (`test_integration_treatment_train.cpp`)

The test program validates:
- **Version reporting**: Confirms DLL reports version 3.0 (treatment train)
- **Argument reporting**: Confirms 1 input (rainfall) and 5 outputs
- **Initialization**: Tests SWMM engine startup and element identification
- **Calculate operations**: Runs multiple time steps with varying rainfall
- **Hydraulic outputs**: Verifies all 5 outputs are retrieved correctly:
  - Catchment discharge from S1
  - Bioswale volume from ST1
  - Detention volume from ST2
  - Retention volume from ST3
  - Final discharge from C3
- **Cleanup**: Tests proper SWMM shutdown
- **Multiple realizations**: Tests Monte Carlo support with 3 realizations
- **Treatment train validation**: Confirms all elements are found and accessible

## Running the Integration Test

### Prerequisites

1. Visual Studio 2022 (Community, Professional, or BuildTools)
2. x64 build tools installed
3. GSswmm.dll built and copied to the tests directory
4. swmm5.dll present in the tests directory

### Build and Run

#### Option 1: Using the Batch File

```batch
cd tests
build_and_test_integration.bat
```

The batch file will:
1. Set up the Visual Studio build environment
2. Compile the integration test
3. Copy the treatment train model to model.inp
4. Run the test
5. Report results

#### Option 2: Using Developer Command Prompt

1. Open "x64 Native Tools Command Prompt for VS 2022"
2. Navigate to the tests directory
3. Compile the test:
   ```
   cl /EHsc /I..\include test_integration_treatment_train.cpp
   ```
4. Copy the model:
   ```
   copy treatment_train_model.inp model.inp
   ```
5. Run the test:
   ```
   test_integration_treatment_train.exe
   ```

#### Option 3: Manual Compilation

If you have a different C++ compiler:

```bash
# Using g++ (MinGW)
g++ -I../include test_integration_treatment_train.cpp -o test_integration_treatment_train.exe

# Using clang
clang++ -I../include test_integration_treatment_train.cpp -o test_integration_treatment_train.exe
```

Then copy the model and run:
```
copy treatment_train_model.inp model.inp
test_integration_treatment_train.exe
```

## Expected Results

When the test runs successfully, you should see output like:

```
=== Integration Test: Treatment Train Model ===

Bridge DLL loaded successfully

Test 1: Report Version
[PASS] Version report returns success
[PASS] Version is 3.0 (treatment train)

Test 2: Report Arguments
[PASS] Arguments report returns success
[PASS] 1 input argument (rainfall)
[PASS] 5 output arguments (catchment + 3 volumes + final)

Test 3: Initialize with Treatment Train Model
[PASS] Initialize succeeds with treatment train model

Test 4: Calculate with Rainfall Inputs
  Step 0: Rain=0.0 in/hr, Catchment=0.0 CFS, Bioswale=0.0 cu.ft, ...
  Step 1: Rain=0.5 in/hr, Catchment=0.15 CFS, Bioswale=12.5 cu.ft, ...
  ...
[PASS] All calculate steps succeed
[PASS] Storage volumes increase during rainfall
[PASS] Discharge occurs from catchment or final outlet

Test 5: Cleanup
[PASS] Cleanup succeeds

Test 6: Multiple Realization Support
  Realization 0:
    Realization 0 completed successfully
  Realization 1:
    Realization 1 completed successfully
  Realization 2:
    Realization 2 completed successfully
[PASS] Multiple realizations complete successfully

Test 7: Treatment Train Element Validation
[PASS] Initialize finds all treatment train elements
[PASS] All 5 hydraulic outputs are valid (non-negative)

=== Test Summary ===
Tests Passed: 13
Tests Failed: 0

ALL TESTS PASSED!
```

## Troubleshooting

### "Failed to load GSswmm.dll"

- Ensure GSswmm.dll is in the tests directory
- Rebuild the DLL using Visual Studio or MSBuild
- Check that the DLL is 64-bit (x64)

### "Failed to find SwmmGoldSimBridge function"

- Verify the DLL exports the function correctly
- Use `dumpbin /exports GSswmm.dll` to check exports

### "Input file does not exist"

- Ensure treatment_train_model.inp is copied to model.inp
- Check that the file is in the tests directory

### "Storage node ST1 (bioswale) not found"

- Verify the treatment_train_model.inp file has all required elements
- Check that element names match exactly (case-sensitive)

### Compilation Errors

- Ensure Visual Studio 2022 is installed with C++ tools
- Try opening the Developer Command Prompt directly
- Check that include path points to the correct location

## Validation Criteria

The integration test validates:

1. ✓ All treatment train elements (S1, ST1, ST2, ST3, J2, C3) are found
2. ✓ Full lifecycle (initialize → calculate → cleanup) works correctly
3. ✓ All 5 hydraulic values are retrieved and valid
4. ✓ Storage volumes increase during rainfall events
5. ✓ Discharge occurs from catchment and final outlet
6. ✓ Multiple realizations work without errors
7. ✓ Numerical results match expected treatment train behavior

## Next Steps

After the integration test passes:

1. Review the numerical results to ensure they're physically reasonable
2. Test with different rainfall patterns and intensities
3. Validate against hand calculations or reference solutions
4. Test with various SWMM model configurations
5. Perform long-duration simulations for stability testing
6. Test Monte Carlo scenarios with thousands of realizations
