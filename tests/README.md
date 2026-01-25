# Test Suite

This directory contains automated tests for the GoldSim-SWMM Bridge DLL.

## Test Organization

### C++ Tests (6 test suites)

**Core Functionality Tests:**
1. **test_lifecycle.cpp** - Tests DLL lifecycle (Initialize, Calculate, Cleanup)
2. **test_calculate.cpp** - Tests calculation logic and data exchange
3. **test_error_handling.cpp** - Tests error handling and reporting
4. **test_file_validation.cpp** - Tests file path validation

**Integration Tests:**
5. **test_json_parsing.cpp** - Tests MappingLoader JSON parsing
6. **test_integration_e2e.cpp** - End-to-end integration test with mapping generation

### Python Tests (4 test scripts)

1. **test_parser.py** - Tests SWMM .inp file parser
2. **test_mapping_generation.py** - Tests mapping file generation
3. **test_input_discovery.py** - Tests input discovery (DUMMY references)
4. **test_hash.py** - Tests content hashing
5. **test_roundtrip_property.py** - Property-based testing for parser

### Test Support Files

- **swmm_mock.cpp/h** - Mock SWMM API for unit testing
- **gtest_minimal.h** - Minimal test framework
- **test_model.inp** - Test SWMM model
- **storage_model.inp** - Storage pond test model
- **test_model_pumps.inp** - Pump control test model

## Running Tests

### All Tests
```batch
scripts\test.bat
```

### Rebuild Test Executables
```batch
# From Developer Command Prompt
scripts\rebuild-tests-with-env.bat

# Or from regular command prompt
scripts\rebuild-tests-with-env.bat
```

### Individual Tests
```batch
# C++ tests (from project root)
.\tests\test_lifecycle.exe
.\tests\test_json_parsing.exe

# Python tests (from tests directory)
cd tests
python test_parser.py
python test_mapping_generation.py
```

## Test Coverage

### What's Tested

✅ DLL loading and function resolution
✅ Initialize/Calculate/Cleanup lifecycle
✅ Error handling and formatted error messages
✅ File path validation
✅ JSON mapping file parsing
✅ SWMM .inp file parsing
✅ Input discovery (DUMMY references)
✅ Mapping generation
✅ End-to-end integration

### What's NOT Tested

❌ Actual SWMM simulation (uses mock)
❌ GoldSim integration (requires GoldSim)
❌ Performance benchmarks
❌ Memory leak detection
❌ Thread safety

## Test Cleanup (January 2026)

Removed obsolete/redundant tests:
- test_subcatchment_validation.cpp (legacy feature)
- test_subcatchment_out_of_range.cpp (legacy feature)
- test_validate_mapping.cpp (redundant with json_parsing)
- test_validate_mapping_simple.cpp (redundant with json_parsing)
- test_error_format.cpp (redundant)
- test_error_messages.cpp (redundant)
- test_error_validation.cpp (redundant)
- test_report_arguments.cpp (covered by integration)
- test_report_arguments_invalid_json.cpp (redundant)
- test_input_output_contract.cpp (unimplemented)
- test_mapping_loader.cpp (redundant with json_parsing)
- test_dll_version.cpp (covered by lifecycle)

**Result:** Reduced from 18 test files to 6 essential tests.

## Adding New Tests

### C++ Test Template

```cpp
#include <iostream>
#include <windows.h>
#include "gtest_minimal.h"

int main() {
    std::cout << "=== My New Test ===" << std::endl;
    
    // Load DLL
    HMODULE hDll = LoadLibraryA("GSswmm.dll");
    ASSERT(hDll != NULL, "Failed to load DLL");
    
    // Get function
    typedef void (*BridgeFunctionType)(int, int*, double*, double*);
    BridgeFunctionType bridge = (BridgeFunctionType)GetProcAddress(hDll, "SwmmGoldSimBridge");
    ASSERT(bridge != NULL, "Failed to get function");
    
    // Test logic here
    
    FreeLibrary(hDll);
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
```

### Python Test Template

```python
import unittest
from generate_mapping import parse_inp_file

class TestMyFeature(unittest.TestCase):
    def test_something(self):
        result = parse_inp_file("test_model.inp")
        self.assertIsNotNone(result)

if __name__ == '__main__':
    unittest.main()
```

## Troubleshooting

**"Failed to load GSswmm.dll"**
- Ensure DLL is built: `scripts\build.bat`
- Copy to tests: `copy /Y x64\Release\GSswmm.dll tests\`
- Copy swmm5.dll: `copy /Y swmm5.dll tests\`
- Run from Developer Command Prompt

**"cl.exe not found"**
- Use `scripts\rebuild-tests-with-env.bat` instead
- Or run from Developer Command Prompt for VS 2022

**Tests pass individually but fail in test.bat**
- Rebuild tests: `scripts\rebuild-tests-with-env.bat`
- Ensure latest DLL is in tests directory

## Test Philosophy

- **Fast**: Tests should run in seconds, not minutes
- **Isolated**: Each test is independent
- **Focused**: One test per feature/behavior
- **Clear**: Test names describe what they test
- **Maintainable**: Remove redundant tests, keep essential ones
