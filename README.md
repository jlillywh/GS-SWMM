# GSswmm: GoldSim-SWMM Bridge

A dynamic link library (DLL) that integrates [EPA SWMM](https://www.epa.gov/water-research/storm-water-management-model-swmm) (Storm Water Management Model) with [GoldSim](https://www.goldsim.com/) simulation software. This bridge enables GoldSim users to leverage SWMM's advanced hydraulic and hydrologic modeling capabilities within their GoldSim simulations.

## Quick Start (5 Minutes)

### 1. Prerequisites
- GoldSim (version 14+)
- SWMM model file (`model.inp`)
- Windows system (x64)

### 2. Setup

**A. Copy Required Files to Your Model Directory**
```
Your_Model_Directory/
├── GSswmm.dll          (from x64/Release/)
├── swmm5.dll           (from project root)
└── model.inp           (your SWMM model)
```

**B. Create External Element in GoldSim**
1. Open your GoldSim model
2. Insert → External Element
3. Name it (e.g., "SWMM_Bridge")
4. Configure the DLL:
   - **DLL File**: Browse to `GSswmm.dll`
   - **Function Name**: `SwmmGoldSimBridge` (exact spelling, case-sensitive!)
   - **Unload DLL after each use**: ☐ Unchecked
   - **Run Cleanup after each realization**: ☑ Checked
   - **Run in separate process space**: ☑ Checked

**C. Configure Inputs and Outputs**

Go to the "Interface" tab:

**Inputs (2 required)**:
1. **ETime** 
   - Type: Scalar
   - Units: seconds
   - Value: `ETime` (GoldSim's built-in elapsed time variable)
2. **Rainfall**
   - Type: Scalar
   - Units: inches/hour (or mm/hour)
   - Value: Link to your rainfall input (timeseries, constant, etc.)

**Outputs (1 required)**:
1. **Runoff**
   - Type: Scalar
   - Units: Match your SWMM model's FLOW_UNITS (typically CFS or CMS)

**D. Configure Simulation**
1. Simulation → Settings
2. Set your time step to match SWMM's ROUTING_STEP
   - Example: If SWMM uses `0:05:00`, set GoldSim to `5 minutes`
3. Set total duration (e.g., 2 hours)

**E. Run**
- Press **F5** or Simulation → Run

## How It Works

The bridge acts as a translator between GoldSim and SWMM:

1. **Initialization**: When GoldSim runs, the bridge loads and initializes the SWMM model
2. **Time Stepping**: Each GoldSim time step, the bridge:
   - Receives rainfall from GoldSim
   - Sets the rainfall input in the SWMM model
   - Advances SWMM one time step
   - Calculates and returns runoff flow to GoldSim
3. **Cleanup**: When the simulation completes, the bridge releases SWMM resources

## Detailed Configuration

### SWMM Model File

Your `model.inp` file must include:

```ini
[OPTIONS]
FLOW_UNITS           CFS           ; or CMS, GPM, MGD, LPS
ROUTING_STEP         0:05:00       ; Time step (match with GoldSim)
REPORT_STEP          1:00:00       ; Reporting interval (can be longer)

[SUBCATCHMENTS]
;;Name           Rain Gage        Outlet           Area     %Imperv  Width    %Slope
S1               RG1              J1               10       50       500      0.5

[RAINGAGES]
;;Name           Format    Interval SCF      DATATYPE    TIMESERIES
RG1              TIMESERIES              1.0   RAINFALL    SWMM_Rainfall

[TIMESERIES]
;;Name           Date       Time       Value
SWMM_Rainfall    1/1        0:00       0.0
```

The bridge will automatically update the `SWMM_Rainfall` timeseries with values from GoldSim.

### Time Step Synchronization

**Critical**: Your GoldSim time step must exactly match your SWMM model's ROUTING_STEP.

| SWMM ROUTING_STEP | GoldSim Time Step |
|-------------------|-------------------|
| 0:00:30           | 30 seconds        |
| 0:05:00           | 5 minutes         |
| 0:15:00           | 15 minutes        |
| 1:00:00           | 1 hour            |

If these don't match, SWMM will run ahead or lag behind GoldSim's simulation clock.

### Input Variables

| Input | Type | Units | Description |
|-------|------|-------|-------------|
| ETime | Scalar | seconds | Elapsed simulation time (use GoldSim's built-in `ETime` variable) |
| Rainfall | Scalar | in/hr or mm/hr | Rainfall intensity (connect to your input source) |

### Output Variables

| Output | Type | Units | Description |
|--------|------|-------|-------------|
| Runoff | Scalar | CFS, CMS, etc. | Runoff flow rate (matches SWMM's FLOW_UNITS setting) |

## Verification and Testing

### Pre-Run Checklist

Before running your simulation, verify:

- [ ] **Files Exist**: `GSswmm.dll`, `swmm5.dll`, `model.inp` all in model directory
- [ ] **DLL Configuration**: Function name is exactly `SwmmGoldSimBridge`
- [ ] **Inputs/Outputs**: 2 inputs (ETime, Rainfall), 1 output (Runoff)
- [ ] **Time Step**: GoldSim time step matches SWMM ROUTING_STEP
- [ ] **Units**: Rainfall and runoff units match your SWMM model
- [ ] **Cleanup Enabled**: "Run Cleanup after each realization" is checked

### Test with a Simple Model

1. **Constant Rainfall**: Set rainfall input to a constant value (e.g., 1.0 in/hr)
2. **Duration**: Run for 2 hours to verify the bridge works
3. **Expected Output**: Runoff should increase and then decrease (typical watershed response)
4. **Check Logs**: Review GoldSim's output and any error messages

If runoff is always zero, check:
- Time step matches SWMM routing step
- Rainfall is being passed correctly
- SWMM model has valid subcatchments

## Troubleshooting

| Problem | Cause | Solution |
|---------|-------|----------|
| "Cannot load DLL" | DLL not found | Copy `GSswmm.dll` to model directory |
| "Cannot find function" | Function name incorrect | Verify it's exactly `SwmmGoldSimBridge` (case-sensitive) |
| "Argument mismatch" error | Wrong number of inputs/outputs | Set 2 inputs (ETime, Rainfall) and 1 output (Runoff) |
| "File not found" error | SWMM model not found | Copy `model.inp` to model directory; ensure correct path in DLL |
| Runoff always zero | SWMM not receiving rainfall or time not advancing | Verify time step matches SWMM ROUTING_STEP; check rainfall value is being set |
| Simulation crashes | Resource not released | Ensure "Run Cleanup after each realization" is checked |
| DLL mismatch | Platform conflict | Use x64 DLL on 64-bit GoldSim (or x86 on 32-bit) |

## API Reference

### Function Signature

```c
void SwmmGoldSimBridge(int methodID, int *status, double *inargs, double *outargs)
```

### Method IDs

| ID | Method | Description |
|----|--------|-------------|
| 0 | XF_INITIALIZE | Initialize SWMM model (called once at simulation start) |
| 1 | XF_CALCULATE | Run one time step with given rainfall |
| 2 | XF_REP_VERSION | Return DLL version (1.00) |
| 3 | XF_REP_ARGUMENTS | Return number of inputs (2) and outputs (1) |
| 99 | XF_CLEANUP | Cleanup and release resources (called at simulation end) |

### Input Arguments (XF_CALCULATE)

```
inargs[0] = Elapsed Time (seconds)
inargs[1] = Rainfall intensity (in/hr or mm/hr, based on SWMM model)
```

### Output Arguments (XF_CALCULATE)

```
outargs[0] = Runoff flow (CFS, CMS, etc., based on SWMM model)
```

### Status Codes

| Code | Meaning |
|------|---------|
| 0 (XF_SUCCESS) | Operation successful |
| 1 (XF_FAILURE) | Operation failed |
| -1 (XF_FAILURE_WITH_MSG) | Operation failed with error message |

## Building from Source

To build the DLL from source code:

**Prerequisites**:
- Visual Studio 2022 Community (C++ development tools)
- Windows SDK

**Steps**:
1. Open `GSswmm.sln` in Visual Studio
2. Select "Release | x64" configuration
3. Build → Build Solution
4. Output: `x64/Release/GSswmm.dll`

**Run Tests**:
```batch
cd tests
run_all_tests.bat
```

## Architecture

The bridge consists of:

- **SwmmGoldSimBridge.cpp**: Main bridge logic, handles GoldSim calls
- **swmm5.h**: SWMM API header (EPA SWMM public interface)
- **Tests**: Comprehensive unit tests for lifecycle, error handling, and calculations

## Advanced Options

### Logging

The bridge can output debug logs. To enable:
1. Set environment variable or config file (implementation-dependent)
2. Check `bridge_debug.log` in your model directory

### Multiple Subcatchments

Currently, the bridge uses the first subcatchment (index 0) by default. For multiple subcatchments, you may need to:
- Create multiple External elements (one per subcatchment), or
- Modify the source code to support subcatchment index parameter

### Custom SWMM Models

You can use any valid SWMM model (`.inp` file). The bridge automatically:
- Detects FLOW_UNITS and adjusts output accordingly
- Validates ROUTING_STEP
- Manages the SWMM simulation lifecycle

## Support and Documentation

- **Full Guides**: See `docs/` directory
- **Pre-Flight Checklist**: [docs/PRE_FLIGHT_CHECKLIST.md](docs/PRE_FLIGHT_CHECKLIST.md)
- **Sample Setup**: [docs/SAMPLE_GOLDSIM_MODEL_SETUP.md](docs/SAMPLE_GOLDSIM_MODEL_SETUP.md)
- **Testing Guide**: [docs/GOLDSIM_TESTING_GUIDE.md](docs/GOLDSIM_TESTING_GUIDE.md)
- **GoldSim External Elements Reference**: [docs/goldsim_external.txt](docs/goldsim_external.txt)

## License

This project integrates EPA SWMM (public domain) with GoldSim integration code. Refer to the license files in the repository for details.

## Version

- **DLL Version**: 4.1
- **Last Updated**: January 2026

## Known Limitations

### Water Quality Tracking Not Supported

**Important**: The current SWMM5 API (version 5.2) does **not** expose pollutant concentrations or water quality data during live simulation. The bridge can only access hydraulic properties (flow, depth, volume) in real-time.

**What This Means**:
- ❌ Cannot retrieve TSS, BOD, or other pollutant concentrations during simulation
- ❌ Cannot access node quality or link quality values in real-time
- ✅ Can retrieve flow rates, water depths, and storage volumes
- ✅ Can set rainfall inputs dynamically

**Why This Limitation Exists**:
The SWMM API's `swmm_getValue()` function only provides access to hydraulic properties. Water quality results are calculated and stored in the binary output file (`.out`) after simulation completion, not during runtime.

**Available Properties**:
- Nodes: depth, head, volume, inflow, overflow
- Links: flow, depth, velocity, top width
- Subcatchments: runoff, rainfall, infiltration

**Workaround**:
If you need water quality data, you must:
1. Run the complete SWMM simulation
2. Use SWMM's post-processing tools to extract pollutant data from the `.out` file
3. Import that data into GoldSim as a separate step

This is a fundamental limitation of the SWMM5 API toolkit and cannot be resolved without changes to the EPA SWMM engine itself.

## Changelog

### Version 4.1 (January 2026)
- **Documentation**: Added comprehensive water quality limitation section
- **Clarification**: Documented that SWMM5 API does not expose pollutant concentrations during live simulation
- **API Investigation**: Confirmed only hydraulic properties (flow, depth, volume) are accessible in real-time
- **No Code Changes**: DLL functionality remains unchanged from v4.0

### Version 4.0 (January 2026)
- **Treatment Train Support**: Added support for multi-stage treatment systems
- **7 Output Variables**: Catchment discharge, bioswale volume, detention volume, retention volume, and flows through C1, C2, C3 links
- **Enhanced Validation**: Comprehensive element validation during initialization
- **Improved Logging**: Detailed debug logging with configurable log levels
- **Robust Error Handling**: Better error messages and cleanup procedures

### Version 3.0 (January 2026)
- **Multiple Storage Nodes**: Support for bioswale, detention, and retention basins
- **Link Flow Tracking**: Monitor flows between treatment stages
- **Element Discovery**: Automatic lookup of SWMM elements by name

### Version 2.0 (January 2026)
- **Storage Volume Output**: Added ability to retrieve storage node volumes
- **Enhanced Testing**: Comprehensive test suite with unit and integration tests

### Version 1.0 (January 2026)
- Initial release
- Support for single subcatchment SWMM models
- Time-synchronized rainfall input
- Runoff output with SWMM unit conversion
- Basic error handling and logging
