# SWMM Test Management System

## Overview

This test management system eliminates the need to manually create and maintain SWMM model files for testing. Instead, tests dynamically generate the exact SWMM models they need using the `swmm_test_models.h` header.

## Benefits

1. **No Pre-existing Files Required**: Tests create their own model files on-the-fly
2. **Automatic Cleanup**: Test fixtures automatically delete temporary files
3. **Consistent Models**: All tests use the same model generation logic
4. **Easy Maintenance**: Update model structure in one place
5. **Isolated Tests**: Each test gets its own model files with unique names

## Components

### 1. swmm_test_models.h

The core header file providing:

- **CreateTreatmentTrainModel()**: Generates a complete treatment train model with all required elements (S1, ST1, ST2, ST3, J2, C1, C2, C3)
- **CreateModelWithSubcatchments()**: Generates a model with a specified number of subcatchments for index validation testing
- **CreateModelMissingElement()**: Generates a model missing a specific element for validation testing
- **TestFixture class**: RAII-style fixture that automatically creates and cleans up test files

### 2. test_runner_comprehensive.cpp

A comprehensive test runner that demonstrates the test management system:

- Tests version and argument reporting
- Tests lifecycle management with treatment train
- Tests error handling
- Tests missing element validation
- Tests subcatchment index validation
- Tests multiple realizations
- All tests use dynamically generated models

### 3. build_comprehensive_test.bat

Simple build script to compile and run the comprehensive test suite.

## Usage Examples

### Basic Usage with TestFixture

```cpp
#include "swmm_test_models.h"

void MyTest() {
    // Create fixture - automatically cleans up on destruction
    SwmmTestModels::TestFixture fixture("my_test");
    
    // Generate treatment train model
    if (!fixture.CreateTreatmentTrain()) {
        // Handle error
        return;
    }
    
    // Use the model
    const char* model_path = fixture.GetModelPath();
    const char* report_path = fixture.GetReportPath();
    const char* output_path = fixture.GetOutputPath();
    
    // Run your test...
    
    // Cleanup happens automatically when fixture goes out of scope
}
```

### Creating Models with Specific Subcatchments

```cpp
void TestSubcatchmentValidation() {
    SwmmTestModels::TestFixture fixture("subcatch_test");
    
    // Create model with 5 subcatchments
    fixture.CreateWithSubcatchments(5);
    
    // Test with the model...
}
```

### Creating Models Missing Specific Elements

```cpp
void TestMissingElement() {
    SwmmTestModels::TestFixture fixture("missing_st1");
    
    // Create model missing the ST1 bioswale node
    fixture.CreateMissingElement("ST1");
    
    // Verify initialization fails appropriately...
}
```

### Manual Cleanup Control

```cpp
void TestWithManualCleanup() {
    SwmmTestModels::TestFixture fixture("manual_test", false);  // Don't auto-cleanup
    
    fixture.CreateTreatmentTrain();
    
    // Run test...
    
    // Manually cleanup when ready
    fixture.Cleanup();
}
```

### Direct Model Generation (No Fixture)

```cpp
void DirectGeneration() {
    // Generate model directly
    SwmmTestModels::CreateTreatmentTrainModel("my_model.inp");
    
    // Use the model...
    
    // Manual cleanup
    std::remove("my_model.inp");
    std::remove("my_model.rpt");
    std::remove("my_model.out");
}
```

## Updating Existing Tests

To update existing tests to use this system:

1. Include the header:
   ```cpp
   #include "swmm_test_models.h"
   ```

2. Replace manual file creation with fixture:
   ```cpp
   // Old way:
   // Manually create model.inp file or rely on pre-existing file
   
   // New way:
   SwmmTestModels::TestFixture fixture;
   fixture.CreateTreatmentTrain();
   ```

3. Remove manual cleanup code - the fixture handles it automatically

## Model Specifications

### Treatment Train Model

The standard treatment train model includes:

- **Rain Gage**: RG1 with time series
- **Subcatchment**: S1 (5 acres, 50% impervious)
- **Storage Nodes**:
  - ST1 (Bioswale): 1000 cu.ft functional storage
  - ST2 (Detention): 2000 cu.ft functional storage
  - ST3 (Retention): 3000 cu.ft functional storage
- **Junction**: J1
- **Outfall**: J2
- **Conduits**: C1, C2, C3 (2-ft circular pipes)
- **Simulation**: 6-hour duration, 30-second routing step

### Subcatchment Model

Minimal model with configurable number of subcatchments:
- Each subcatchment named SUB0, SUB1, SUB2, etc.
- All drain to single outfall J2
- 1-hour simulation duration

## Running Tests

### Comprehensive Test Suite

```bash
cd tests
build_comprehensive_test.bat
```

This will:
1. Compile the comprehensive test runner
2. Run all tests automatically
3. Display results with pass/fail status
4. Clean up all temporary files

### Individual Tests

Existing test executables can be updated to use the test management system without changing their build process. Simply include `swmm_test_models.h` and use the fixtures.

## Best Practices

1. **Use TestFixture**: Always prefer TestFixture over direct generation for automatic cleanup
2. **Unique Names**: Give each test fixture a unique base name to avoid file conflicts
3. **Check Return Values**: Always check if model generation succeeded before proceeding
4. **Minimal Models**: Use the simplest model that tests your requirement
5. **Document Requirements**: Comment which elements your test requires

## Troubleshooting

### Model Generation Fails

- Check disk space and write permissions
- Verify the tests directory is writable
- Check for file locks on .inp, .rpt, or .out files

### Tests Fail After Model Generation

- Verify the generated model is valid by inspecting the .inp file
- Check SWMM error messages in the .rpt file
- Use the diagnostic tools in DIAGNOSTIC_TOOLS_README.md

### Cleanup Issues

- If files aren't being deleted, check if SWMM has them locked
- Ensure XF_CLEANUP is called before fixture destruction
- Use DisableAutoCleanup() if you need to inspect files after test

## Future Enhancements

Potential additions to the test management system:

1. **Model Templates**: Pre-defined model configurations for common scenarios
2. **Parameter Customization**: Easy ways to customize model parameters
3. **Validation Helpers**: Functions to validate model structure
4. **Performance Models**: Models optimized for performance testing
5. **Error Injection**: Models designed to trigger specific SWMM errors
