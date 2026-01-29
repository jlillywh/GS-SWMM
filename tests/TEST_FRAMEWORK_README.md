# Test Infrastructure Documentation

## Overview

This directory contains the test infrastructure for the GoldSim-SWMM Bridge DLL, including:
- SWMM API mock implementation for unit testing
- Google Test-compatible framework for unit tests
- RapidCheck-compatible framework for property-based tests
- Sample test files demonstrating both testing approaches

## Components

### 1. SWMM API Mock (`swmm_mock.h` / `swmm_mock.cpp`)

The mock implementation provides:
- **Call tracking**: Records all SWMM API function calls and their parameters
- **Configurable return values**: Allows tests to simulate success/failure scenarios
- **Parameter verification**: Enables tests to verify correct parameters were passed
- **State simulation**: Tracks opened/started state to simulate SWMM lifecycle

#### Key Functions

**Mock Control Functions:**
```cpp
SwmmMock_Reset();                              // Reset all state to defaults
SwmmMock_SetSuccessMode();                     // Configure all operations to succeed
SwmmMock_SetOpenFailure(code, msg);            // Simulate swmm_open failure
SwmmMock_SetStartFailure(code, msg);           // Simulate swmm_start failure
SwmmMock_SetStepFailure(code, msg);            // Simulate swmm_step failure
SwmmMock_SetStepEndAfter(n);                   // Simulation ends after N steps
SwmmMock_SetStepErrorAfter(n);                 // Error occurs after N steps
SwmmMock_SetGetValueReturn(value);             // Set return value for getValue
SwmmMock_SetGetCountReturn(count);             // Set return value for getCount
```

**Call Count Verification:**
```cpp
int count = SwmmMock_GetOpenCallCount();       // Get number of swmm_open calls
int count = SwmmMock_GetStartCallCount();      // Get number of swmm_start calls
int count = SwmmMock_GetStepCallCount();       // Get number of swmm_step calls
// ... similar for end, close, getValue, setValue
```

**Parameter Verification:**
```cpp
const char* file = SwmmMock_GetLastInputFile();     // Get last input file path
int index = SwmmMock_GetLastSetValueIndex();        // Get last setValue index
double value = SwmmMock_GetLastSetValueValue();     // Get last setValue value
// ... similar for other parameters
```

### 2. Google Test Framework (`gtest_minimal.h`)

A minimal Google Test-compatible framework providing:
- `TEST(TestSuite, TestName)` macro for defining tests
- Assertion macros: `EXPECT_EQ`, `EXPECT_NE`, `EXPECT_TRUE`, `EXPECT_FALSE`, etc.
- `ASSERT_*` macros that abort test on failure
- Automatic test registration and execution
- Formatted test output

#### Example Unit Test

```cpp
#include "gtest_minimal.h"
#include "swmm_mock.h"

TEST(LifecycleTests, InitializeCallsSwmmOpen) {
    // Setup
    SwmmMock_Reset();
    SwmmMock_SetSuccessMode();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Execute
    SwmmGoldSimBridge(XF_INITIALIZE, &status, inargs, outargs);
    
    // Verify
    EXPECT_EQ(status, XF_SUCCESS);
    EXPECT_EQ(SwmmMock_GetOpenCallCount(), 1);
    EXPECT_EQ(SwmmMock_GetStartCallCount(), 1);
}

int main() {
    return RUN_ALL_TESTS();
}
```

### 3. RapidCheck Framework (`rapidcheck_minimal.h`)

A minimal RapidCheck-compatible property-based testing framework providing:
- `RC_GTEST_PROP` macro for defining property tests
- Generators for random test data: `inRange`, `element`, `boolean`
- `RC_ASSERT` macro for property assertions
- Configurable iteration count (minimum 100 per property)
- Feature and property tagging for traceability

#### Example Property Test

```cpp
#include "rapidcheck_minimal.h"
#include "swmm_mock.h"

// Feature: goldsim-swmm-bridge
// Property 1: Successful method calls return XF_SUCCESS
// Validates: Requirements 1.4

RC_GTEST_PROP(BridgeProperties, SuccessfulCallsReturnSuccess,
              "goldsim-swmm-bridge",
              "Property 1: Successful method calls return XF_SUCCESS")
{
    // Setup
    SwmmMock_Reset();
    SwmmMock_SetSuccessMode();
    
    // Generate random valid method ID
    auto methodIDGen = rc::gen::element(XF_INITIALIZE, XF_CALCULATE, XF_CLEANUP);
    int methodID = methodIDGen.generate();
    
    int status;
    double inargs[10] = {0};
    double outargs[10] = {0};
    
    // Execute
    SwmmGoldSimBridge(methodID, &status, inargs, outargs);
    
    // Verify property holds
    RC_ASSERT(status == XF_SUCCESS);
    
    return true;
}

int main() {
    return RUN_ALL_PROPERTY_TESTS();
}
```

### 4. Sample Test Files

- **`test_unit_sample.cpp`**: Demonstrates unit testing with mocked SWMM API
  - Version and argument reporting tests
  - Lifecycle management tests
  - Error handling tests
  - Calculate operation tests

- **`test_property_sample.cpp`**: Demonstrates property-based testing
  - Property 1: Successful calls return success
  - Property 2: Failed calls return error codes
  - Property 3: SWMM errors propagate to status
  - Property 4: Subcatchment index consistency
  - Property 7: State invariant maintenance

## Building and Running Tests

### Build Unit Tests

```batch
cd tests
build_unit_tests.bat
```

This compiles:
1. `swmm_mock.cpp` → `swmm_mock.obj`
2. `test_unit_sample.cpp` → `test_unit_sample.exe`

### Run Unit Tests

```batch
test_unit_sample.exe
```

### Build Property-Based Tests

```batch
cd tests
build_property_tests.bat
```

This compiles:
1. `swmm_mock.cpp` → `swmm_mock.obj` (if not already compiled)
2. `test_property_sample.cpp` → `test_property_sample.exe`

### Run Property-Based Tests

```batch
test_property_sample.exe
```

Each property will be tested with a minimum of 100 random inputs.

## Writing New Tests

### Writing a Unit Test

1. Include the test framework and mock:
   ```cpp
   #include "gtest_minimal.h"
   #include "swmm_mock.h"
   ```

2. Define your test:
   ```cpp
   TEST(TestSuiteName, TestName) {
       // Setup
       SwmmMock_Reset();
       SwmmMock_SetSuccessMode();
       
       // Execute
       // ... call bridge function ...
       
       // Verify
       EXPECT_EQ(expected, actual);
   }
   ```

3. Add main function:
   ```cpp
   int main() {
       return RUN_ALL_TESTS();
   }
   ```

4. Compile and link with `swmm_mock.obj`

### Writing a Property Test

1. Include the property test framework and mock:
   ```cpp
   #include "rapidcheck_minimal.h"
   #include "swmm_mock.h"
   ```

2. Define your property:
   ```cpp
   RC_GTEST_PROP(TestSuiteName, TestName,
                 "feature-name",
                 "Property N: Description")
   {
       // Setup
       SwmmMock_Reset();
       
       // Generate random inputs
       auto gen = rc::gen::inRange(0, 100);
       int value = gen.generate();
       
       // Execute and verify property
       // ... call bridge function ...
       RC_ASSERT(property_holds);
       
       return true;
   }
   ```

3. Add main function:
   ```cpp
   int main() {
       return RUN_ALL_PROPERTY_TESTS();
   }
   ```

4. Compile and link with `swmm_mock.obj`

## Available Generators

### Integer Range
```cpp
auto gen = rc::gen::inRange(0, 100);
int value = gen.generate();
```

### Double Range
```cpp
auto gen = rc::gen::inRange(0.0, 10.0);
double value = gen.generate();
```

### Boolean
```cpp
auto gen = rc::gen::boolean();
bool value = gen.generate();
```

### Element (pick from list)
```cpp
auto gen = rc::gen::element(1, 2, 3, 4, 5);
int value = gen.generate();
```

## Test Organization

Tests should be organized by:
- **Unit tests**: Test specific behaviors and edge cases
- **Property tests**: Test universal properties across many inputs

Each test file should:
- Focus on a specific aspect of functionality
- Include clear comments explaining what is being tested
- Reference requirements being validated
- Use descriptive test names

## Integration with Build System

The test infrastructure is designed to work with the existing Visual Studio build system:
- Mock replaces real SWMM API at link time
- Tests load the bridge DLL dynamically
- Build scripts use standard `cl.exe` compiler
- No external dependencies required

## Configuration

### Minimum Iterations for Property Tests

The minimum number of iterations per property test is configured in `rapidcheck_minimal.h`:

```cpp
#define RC_MIN_ITERATIONS 100
```

Increase this value for more thorough testing (e.g., 1000 or 10000 iterations).

## Troubleshooting

### DLL Not Found
- Ensure `GSswmm.dll` is in the `tests` directory
- Copy from `x64/Release/` after building the main project

### Mock Linking Issues
- Ensure `swmm_mock.obj` is compiled before linking tests
- Include `swmm_mock.obj` in the link command

### Test Failures
- Check mock configuration (reset before each test)
- Verify expected vs actual values in assertion messages
- Use call count and parameter verification functions

## Best Practices

1. **Always reset mock state** before each test
2. **Use descriptive test names** that explain what is being tested
3. **Test one thing per test** - keep tests focused
4. **Verify both success and failure paths**
5. **Use property tests for universal properties**
6. **Use unit tests for specific examples and edge cases**
7. **Tag property tests** with feature name and property description
8. **Reference requirements** in test comments

## Future Enhancements

Potential improvements to the test infrastructure:
- Add test fixtures for common setup/teardown
- Implement test filtering by name or tag
- Add performance benchmarking support
- Integrate with CI/CD pipeline
- Add code coverage reporting
- Support for parameterized tests
