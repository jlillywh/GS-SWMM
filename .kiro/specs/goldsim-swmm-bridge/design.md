# Design Document: GoldSim-SWMM Bridge DLL

## Overview

The GoldSim-SWMM Bridge DLL is a C++ dynamic link library that enables bidirectional communication between GoldSim simulation software and the EPA SWMM hydraulic engine for treatment train modeling. The bridge implements the GoldSim External Element API and manages the complete SWMM lifecycle, allowing GoldSim to control simulation timing, provide rainfall inputs, and receive hydraulic calculations from multiple points in a stormwater treatment train.

The design follows a state-machine pattern to manage the SWMM engine lifecycle and ensures proper sequencing of API calls. The bridge operates on a one-to-one time step correspondence model where each GoldSim calculation step triggers exactly one SWMM routing step. This enables Monte Carlo simulation where GoldSim generates stochastic rainfall patterns, SWMM calculates hydraulic response through the treatment train, and GoldSim performs contaminant transport calculations using the returned volumes and flows.

## Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        GoldSim                              │
│              (Stochastic Simulation Engine)                 │
│         - Generates random rainfall patterns                │
│         - Calculates contaminant transport                  │
│         - Performs Monte Carlo analysis                     │
└────────────────────┬────────────────────────────────────────┘
                     │ Method ID, Status, Input/Output Arrays
                     │ Input: Rainfall intensity (in/hr)
                     │ Outputs: 5 hydraulic values
                     ▼
┌─────────────────────────────────────────────────────────────┐
│              Bridge DLL Entry Point                         │
│         SwmmGoldSimBridge(methodID, status,                 │
│                          inargs, outargs)                   │
└────────────────────┬────────────────────────────────────────┘
                     │
        ┌────────────┴────────────┐
        │                         │
        ▼                         ▼
┌──────────────────┐    ┌──────────────────────┐
│  State Manager   │    │  Configuration       │
│  - is_running    │    │  - file paths        │
│  - node indices  │    │  - element names     │
│  - link indices  │    │                      │
└────────┬─────────┘    └──────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│                    SWMM API Wrapper                         │
│  - swmm_open()    - swmm_getValue()                         │
│  - swmm_start()   - swmm_setValue()                         │
│  - swmm_step()    - swmm_getError()                         │
│  - swmm_end()     - swmm_getIndex()                         │
│  - swmm_close()                                             │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    SWMM Engine                              │
│                   (swmm5.dll)                               │
│                                                             │
│  Treatment Train Model:                                     │
│  Rain → [S1 Subcatchment] → [ST1 Bioswale]                 │
│       → [ST2 Detention] → [ST3 Retention] → [J2 Outfall]   │
└─────────────────────────────────────────────────────────────┘
```

### Treatment Train Architecture

The bridge is designed to model a stormwater treatment train with the following components:

```
Rainfall (from GoldSim)
    ↓
┌─────────────────┐
│ Subcatchment S1 │ → Generates runoff from rainfall
└────────┬────────┘
         │ Catchment Discharge (CFS)
         ▼
┌─────────────────┐
│ Bioswale (ST1)  │ → First treatment/detention stage
└────────┬────────┘
         │ Volume (cu.ft)
         ▼
┌─────────────────┐
│ Detention (ST2) │ → Secondary storage
└────────┬────────┘
         │ Volume (cu.ft)
         ▼
┌─────────────────┐
│ Retention (ST3) │ → Tertiary storage
└────────┬────────┘
         │ Volume (cu.ft)
         ▼
┌─────────────────┐
│ Outfall (J2)    │ → Final discharge to environment
└─────────────────┘
         │ Final Discharge (CFS)
         ▼
    Environment
```

At each time step, the bridge returns 5 hydraulic values to GoldSim:
1. **Catchment Discharge**: Runoff from subcatchment S1 (CFS)
2. **Bioswale Volume**: Water stored in ST1 (cu.ft)
3. **Detention Volume**: Water stored in ST2 (cu.ft)
4. **Retention Volume**: Water stored in ST3 (cu.ft)
5. **Final Discharge**: Flow from final conduit to outfall (CFS)

GoldSim uses these volumes and flows to calculate contaminant mass balance at each stage, applying removal rates and tracking pollutant transport through the treatment train.

### State Machine

The bridge maintains a simple state machine to track the SWMM engine lifecycle:

```
┌─────────────┐
│ UNINITIALIZED│
└──────┬───────┘
       │ XF_INITIALIZE
       │ swmm_open() + swmm_start()
       │ + retrieve element indices
       ▼
┌─────────────┐
│   RUNNING   │◄────┐
└──────┬───────┘     │
       │             │ XF_CALCULATE
       │             │ - Set rainfall
       │             │ - swmm_step()
       │             │ - Get 5 hydraulic values
       │             │
       │             └─────────────┐
       │                           │
       │ XF_CLEANUP or             │
       │ swmm_step() returns end   │
       │ swmm_end() + swmm_close() │
       ▼                           │
┌─────────────┐                    │
│ UNINITIALIZED│───────────────────┘
└─────────────┘
```

## Components and Interfaces

### 1. Bridge Entry Point

**Function Signature:**
```cpp
extern "C" void __declspec(dllexport) SwmmGoldSimBridge(
    int methodID,
    int* status,
    double* inargs,
    double* outargs
)
```

**Parameters:**
- `methodID`: Integer indicating the requested operation (0=Initialize, 1=Calculate, 2=Report Version, 3=Report Arguments, 99=Cleanup)
- `status`: Pointer to integer for returning operation status (0=success, >0=failure, -1=failure with message)
- `inargs`: Array of double-precision input arguments from GoldSim
- `outargs`: Array of double-precision output arguments to GoldSim

**Responsibilities:**
- Dispatch method calls based on methodID
- Manage global state variables
- Coordinate SWMM API calls
- Handle error conditions and reporting

### 2. Global State Variables

```cpp
static bool is_swmm_running = false;
static int subcatchment_index = 0;
static int bioswale_index = 0;
static int detention_index = 0;
static int retention_index = 0;
static int outfall_index = 0;
static int conduit_c3_index = 0;
static char input_file_path[260] = "model.inp";
static char report_file_path[260] = "model.rpt";
static char output_file_path[260] = "model.out";
static char error_message_buffer[200];
```

**State Variables:**
- `is_swmm_running`: Boolean flag tracking whether SWMM has been initialized and started
- `subcatchment_index`: Zero-based index of subcatchment S1
- `bioswale_index`: Zero-based index of storage node ST1 (bioswale)
- `detention_index`: Zero-based index of storage node ST2 (detention)
- `retention_index`: Zero-based index of storage node ST3 (retention)
- `outfall_index`: Zero-based index of outfall node J2
- `conduit_c3_index`: Zero-based index of conduit C3 (final discharge)
- `input_file_path`: Path to SWMM input file (.inp)
- `report_file_path`: Path to SWMM report file (.rpt)
- `output_file_path`: Path to SWMM output file (.out)
- `error_message_buffer`: Static buffer for error messages returned to GoldSim

### 3. Method Handlers

#### XF_INITIALIZE (methodID = 0)

**Purpose:** Initialize SWMM engine and identify treatment train elements

**Logic:**
1. Check if SWMM is already running; if so, call cleanup sequence
2. Call `swmm_open(input_file_path, report_file_path, output_file_path)`
3. If swmm_open succeeds, call `swmm_start(1)` to begin simulation with output saving
4. Retrieve element indices using swmm_getIndex():
   - `subcatchment_index = swmm_getIndex(swmm_SUBCATCH, "S1")`
   - `bioswale_index = swmm_getIndex(swmm_NODE, "ST1")`
   - `detention_index = swmm_getIndex(swmm_NODE, "ST2")`
   - `retention_index = swmm_getIndex(swmm_NODE, "ST3")`
   - `outfall_index = swmm_getIndex(swmm_NODE, "J2")`
   - `conduit_c3_index = swmm_getIndex(swmm_LINK, "C3")`
5. Validate all indices are non-negative (element exists)
6. If all succeed, set `is_swmm_running = true`
7. If any step fails, retrieve error message and return XF_FAILURE

**Error Handling:**
- If swmm_open fails, return error immediately
- If swmm_start fails, call swmm_close before returning error
- If any element index is negative, call cleanup and return error

#### XF_CALCULATE (methodID = 1)

**Purpose:** Advance simulation one time step and exchange hydraulic data

**Logic:**
1. Verify `is_swmm_running == true`; if false, return XF_FAILURE
2. Read rainfall intensity from `inargs[0]`
3. Call `swmm_setValue(swmm_GAGE_RAINFALL, 0, inargs[0])` to set rainfall on rain gage 0
4. Call `swmm_step(&elapsed_time)` to advance SWMM one routing step
5. Check swmm_step return code:
   - If 0: simulation continues normally
   - If >0: simulation has ended, call cleanup sequence and return XF_SUCCESS
   - If <0: error occurred, retrieve error message and return XF_FAILURE
6. Retrieve hydraulic values:
   - `catchment_discharge = swmm_getValue(swmm_SUBCATCH_RUNOFF, subcatchment_index)`
   - `bioswale_volume = swmm_getValue(swmm_NODE_VOLUME, bioswale_index)`
   - `detention_volume = swmm_getValue(swmm_NODE_VOLUME, detention_index)`
   - `retention_volume = swmm_getValue(swmm_NODE_VOLUME, retention_index)`
   - `final_discharge = swmm_getValue(swmm_LINK_FLOW, conduit_c3_index)`
7. Write values to output array:
   - `outargs[0] = catchment_discharge`
   - `outargs[1] = bioswale_volume`
   - `outargs[2] = detention_volume`
   - `outargs[3] = retention_volume`
   - `outargs[4] = final_discharge`

**Data Flow:**
- Input: `inargs[0]` = rainfall intensity (inches/hour or mm/hour)
- Outputs:
  - `outargs[0]` = catchment discharge (CFS or CMS)
  - `outargs[1]` = bioswale volume (cu.ft or cu.m)
  - `outargs[2]` = detention volume (cu.ft or cu.m)
  - `outargs[3]` = retention volume (cu.ft or cu.m)
  - `outargs[4]` = final discharge (CFS or CMS)

#### XF_REP_VERSION (methodID = 2)

**Purpose:** Report DLL version number to GoldSim

**Logic:**
1. Write version number to `outargs[0]` (3.0 for treatment train version)
2. Return XF_SUCCESS

#### XF_REP_ARGUMENTS (methodID = 3)

**Purpose:** Report number of input and output arguments

**Logic:**
1. Write number of inputs to `outargs[0]` (1 input: rainfall)
2. Write number of outputs to `outargs[1]` (5 outputs: catchment discharge, 3 volumes, final discharge)
3. Return XF_SUCCESS

#### XF_CLEANUP (methodID = 99)

**Purpose:** Terminate SWMM simulation and release resources

**Logic:**
1. Check if `is_swmm_running == true`
2. If true:
   - Call `swmm_end()` to finalize simulation
   - Call `swmm_close()` to close files and release memory
   - Set `is_swmm_running = false`
   - Reset all element indices to 0
3. Return XF_SUCCESS

### 4. Error Handling Module

**Function:** `HandleSwmmError(int error_code, double* outargs, int* status)`

**Purpose:** Retrieve SWMM error messages and format them for GoldSim

**Logic:**
1. Call `swmm_getError(error_message_buffer, sizeof(error_message_buffer))`
2. Ensure error message is null-terminated
3. Cast `outargs` as `ULONG_PTR*` and store address of `error_message_buffer`
4. Set `*status = XF_FAILURE_WITH_MSG` (-1)

**Memory Safety:**
- Uses static buffer with file scope to ensure memory remains valid when GoldSim reads it
- Buffer size limited to 200 characters to prevent overflow

## Data Models

### Input Arguments Array (inargs)

| Index | Type   | Description                    | Units                  |
|-------|--------|--------------------------------|------------------------|
| 0     | double | Rainfall intensity             | inches/hr or mm/hr     |

### Output Arguments Array (outargs)

| Index | Type   | Description                    | Units                  |
|-------|--------|--------------------------------|------------------------|
| 0     | double | Catchment discharge (S1)       | CFS or CMS             |
| 1     | double | Bioswale volume (ST1)          | cu.ft or cu.m          |
| 2     | double | Detention volume (ST2)         | cu.ft or cu.m          |
| 3     | double | Retention volume (ST3)         | cu.ft or cu.m          |
| 4     | double | Final discharge (C3)           | CFS or CMS             |

**Special Cases:**
- When methodID = XF_REP_VERSION: outargs[0] = version number (3.0)
- When methodID = XF_REP_ARGUMENTS: outargs[0] = 1 (num inputs), outargs[1] = 5 (num outputs)
- When status = XF_FAILURE_WITH_MSG: outargs[0] = pointer to error message string

### Treatment Train Element Configuration

```cpp
struct TreatmentTrainElements {
    const char* subcatchment_name = "S1";
    const char* bioswale_name = "ST1";
    const char* detention_name = "ST2";
    const char* retention_name = "ST3";
    const char* outfall_name = "J2";
    const char* final_conduit_name = "C3";
};
```

### Configuration Data

Configuration is currently hardcoded in global variables but designed for future extension:

```cpp
struct BridgeConfiguration {
    char input_file[260];      // SWMM .inp file path
    char report_file[260];     // SWMM .rpt file path
    char output_file[260];     // SWMM .out file path
    TreatmentTrainElements elements;  // Element names
};
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Successful method calls return XF_SUCCESS

*For any* valid method call that completes without errors, the status code should be set to 0 (XF_SUCCESS).

**Validates: Requirements 1.4**

### Property 2: Failed method calls return appropriate error codes

*For any* method call that encounters a fatal error, the status code should be set to a value greater than 0 and less than 99, or -1 for errors with messages.

**Validates: Requirements 1.5**

### Property 3: SWMM API errors propagate to status

*For any* SWMM API function call that returns a non-zero error code, the bridge should set the status to XF_FAILURE or XF_FAILURE_WITH_MSG.

**Validates: Requirements 2.6, 8.1**

### Property 4: One swmm_step per calculate call

*For any* XF_CALCULATE call when SWMM is running, swmm_step() should be called exactly once.

**Validates: Requirements 3.1**

### Property 5: Rainfall values propagate to SWMM

*For any* rainfall intensity value provided in inargs[0], swmm_setValue should be called with swmm_GAGE_RAINFALL property and that value.

**Validates: Requirements 4.2**

### Property 6: All treatment train hydraulic values retrieved

*For any* successful swmm_step() completion, all five hydraulic values (catchment discharge, bioswale volume, detention volume, retention volume, final discharge) should be retrieved using swmm_getValue and written to the output array in the correct order.

**Validates: Requirements 5.1, 5.2, 5.3, 5.4, 5.5, 5.6**

### Property 7: File path validation

*For any* file path that is invalid or points to a non-existent file, calling XF_INITIALIZE should return XF_FAILURE or XF_FAILURE_WITH_MSG.

**Validates: Requirements 7.5**

### Property 8: State invariant maintenance

*For any* sequence of method calls, the is_swmm_running state variable should accurately reflect whether SWMM has been successfully initialized and not yet cleaned up.

**Validates: Requirements 9.1**

### Property 9: Path resolution consistency

*For any* relative file path, the bridge should resolve it relative to the same base directory consistently across all file operations.

**Validates: Requirements 7.4**

### Property 10: Error messages retrieved and formatted

*For any* SWMM API error, the bridge should call swmm_getError() to retrieve the error message and return it via the XF_FAILURE_WITH_MSG mechanism.

**Validates: Requirements 8.2, 8.4**

### Property 11: Multiple realization support

*For any* sequence of multiple realizations (initialize → calculate → cleanup cycles), the bridge should properly initialize and cleanup SWMM for each realization, reset all state variables, and release all resources without memory leaks or performance degradation.

**Validates: Requirements 11.1, 11.2, 11.3, 11.4**

## Error Handling

### Error Detection

The bridge implements comprehensive error detection at multiple levels:

1. **SWMM API Errors**: All SWMM function calls are checked for non-zero return codes
2. **State Validation**: Method calls are validated against current state (e.g., cannot calculate before initialize)
3. **Parameter Validation**: File paths and element indices are validated during initialization
4. **Element Existence**: All treatment train elements are validated to exist in the SWMM model
5. **Resource Cleanup**: Errors during initialization trigger cleanup of partially-initialized resources

### Error Reporting

The bridge uses the GoldSim error reporting mechanism:

```cpp
void HandleSwmmError(int error_code, double* outargs, int* status) {
    // Retrieve error message from SWMM
    swmm_getError(error_message_buffer, sizeof(error_message_buffer));
    
    // Ensure null termination
    error_message_buffer[sizeof(error_message_buffer) - 1] = '\0';
    
    // Cast outargs to pointer type and store buffer address
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    *pAddr = (ULONG_PTR)error_message_buffer;
    
    // Set status to indicate error with message
    *status = XF_FAILURE_WITH_MSG;
}
```

### Error Recovery

- **Initialization Errors**: If swmm_open succeeds but swmm_start fails, the bridge calls swmm_close before returning
- **Element Not Found**: If any treatment train element is not found, the bridge performs cleanup and returns error
- **Simulation End**: When swmm_step indicates simulation end, the bridge performs normal cleanup and returns success
- **Re-initialization**: If XF_INITIALIZE is called while SWMM is running, the bridge first performs cleanup before starting a new simulation

### Critical Error Scenarios

| Scenario | Detection | Response |
|----------|-----------|----------|
| Invalid input file | swmm_open returns error | Return XF_FAILURE_WITH_MSG with SWMM error |
| SWMM initialization failure | swmm_start returns error | Call swmm_close, return XF_FAILURE_WITH_MSG |
| Treatment train element not found | swmm_getIndex returns -1 | Call cleanup, return XF_FAILURE_WITH_MSG |
| Calculate before initialize | is_swmm_running == false | Return XF_FAILURE |
| Simulation end | swmm_step returns >0 | Call cleanup, return XF_SUCCESS |
| SWMM runtime error | swmm_step returns <0 | Return XF_FAILURE_WITH_MSG |
| getValue failure | swmm_getValue returns error | Return XF_FAILURE_WITH_MSG |

## Testing Strategy

### Dual Testing Approach

The testing strategy employs both unit testing and property-based testing to ensure comprehensive coverage:

**Unit Tests** focus on:
- Specific method call sequences (initialize → calculate → cleanup)
- Edge cases (cleanup when not running, calculate before initialize)
- Error conditions (invalid files, missing elements)
- State transitions (uninitialized → running → uninitialized)
- Integration with mocked SWMM API
- Treatment train element identification
- Multi-output data retrieval

**Property-Based Tests** focus on:
- Universal properties across all valid inputs (status codes, rainfall propagation)
- Error propagation across all SWMM API functions
- State invariant maintenance across arbitrary method sequences
- Path resolution consistency
- Multiple realization cycles without resource leaks

### Property-Based Testing Configuration

**Library**: For C++, we will use RapidCheck (https://github.com/emil-e/rapidcheck), a QuickCheck-inspired property-based testing library.

**Configuration**:
- Minimum 100 iterations per property test
- Each test tagged with: **Feature: goldsim-swmm-bridge, Property N: [property text]**
- Tests will use mocked SWMM API to avoid file system dependencies

**Example Property Test Structure**:
```cpp
// Feature: goldsim-swmm-bridge, Property 1: Successful method calls return XF_SUCCESS
RC_GTEST_PROP(BridgeProperties, SuccessfulCallsReturnSuccess, ()) {
    // Generate valid method ID
    auto methodID = *rc::gen::element(0, 1, 2, 3, 99);
    
    // Setup mock SWMM to succeed
    MockSwmmSuccess();
    
    // Call bridge
    int status;
    double inargs[1] = {0.0};
    double outargs[5];
    SwmmGoldSimBridge(methodID, &status, inargs, outargs);
    
    // Verify status is XF_SUCCESS
    RC_ASSERT(status == 0);
}
```

### Unit Testing Strategy

**Test Framework**: Google Test (gtest) for C++ unit testing

**Mock Strategy**: 
- Create mock implementations of SWMM API functions
- Use function pointers or link-time substitution to inject mocks
- Mock functions track call counts and parameters for verification

**Test Categories**:

1. **Lifecycle Tests**
   - Test: Initialize → Calculate → Cleanup sequence
   - Test: Multiple realizations (Initialize → Calculate → Cleanup → Initialize)
   - Test: Re-initialization while running
   - Test: Element index retrieval during initialization

2. **Method Handler Tests**
   - Test: XF_REP_VERSION returns 3.0
   - Test: XF_REP_ARGUMENTS returns 1 input, 5 outputs
   - Test: XF_INITIALIZE opens, starts SWMM, and retrieves element indices
   - Test: XF_CALCULATE sets rainfall, steps, gets 5 hydraulic values
   - Test: XF_CLEANUP ends and closes SWMM

3. **Error Handling Tests**
   - Test: Invalid file path during initialize
   - Test: swmm_start failure triggers cleanup
   - Test: Calculate before initialize returns failure
   - Test: Missing treatment train element returns error
   - Test: SWMM runtime error propagates correctly

4. **State Management Tests**
   - Test: is_swmm_running flag transitions correctly
   - Test: Cleanup when not running succeeds without SWMM calls
   - Test: State persists across multiple calculate calls
   - Test: State resets properly between realizations

5. **Data Exchange Tests**
   - Test: Rainfall value from inargs[0] passed to SWMM
   - Test: All 5 hydraulic values retrieved and written to outargs
   - Test: Output array ordering is correct
   - Test: Correct element indices used for all operations

6. **Treatment Train Tests**
   - Test: All treatment train elements identified correctly
   - Test: Volumes retrieved from correct storage nodes
   - Test: Discharge retrieved from correct conduit
   - Test: Missing element causes initialization failure

### Integration Testing

While unit tests use mocked SWMM API, integration tests should:
- Use actual SWMM engine with test input files
- Verify end-to-end data flow through treatment train
- Test with various SWMM model configurations
- Validate numerical results against known scenarios
- Test Monte Carlo simulation with multiple realizations

**Test SWMM Models**:
- Treatment train model with S1, ST1, ST2, ST3, J2, C3 elements
- Model with various time steps for synchronization testing
- Model with different flow units for unit conversion testing

### Test Execution

**Unit Tests**: Run on every build, fast execution with mocks
**Property Tests**: Run on every build, 100+ iterations per property
**Integration Tests**: Run before releases, slower execution with real SWMM

**Coverage Goals**:
- Line coverage: >90%
- Branch coverage: >85%
- All error paths exercised
- All state transitions tested
- All treatment train elements tested
